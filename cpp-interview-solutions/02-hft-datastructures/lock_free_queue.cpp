/**
 * Lock-Free Queue - Critical for HFT Systems
 *
 * Applications in HFT:
 * - Inter-thread communication without locks
 * - Market data distribution
 * - Order routing pipeline
 * - Event processing queues
 *
 * Interview Focus:
 * - Atomic operations and memory ordering
 * - ABA problem and solutions
 * - SPSC vs MPMC queues
 * - Cache line padding
 * - Performance characteristics
 *
 * Complexity:
 * - Enqueue: O(1) - wait-free for SPSC
 * - Dequeue: O(1) - wait-free for SPSC
 * - Space: O(n) where n is capacity
 */

#include <atomic>
#include <memory>
#include <optional>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

// Cache line size for padding
constexpr size_t CACHE_LINE_SIZE = 64;

// Example 1: Single Producer Single Consumer (SPSC) Lock-Free Queue
// Most common in HFT - extremely fast, wait-free
template<typename T, size_t Size>
class SPSCQueue {
private:
    struct alignas(CACHE_LINE_SIZE) AlignedType {
        T data;
    };

    // Separate cache lines to avoid false sharing
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> head_{0};
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> tail_{0};
    alignas(CACHE_LINE_SIZE) AlignedType buffer_[Size];

    // Cached values to reduce atomic loads
    size_t cached_head_{0};
    size_t cached_tail_{0};

public:
    SPSCQueue() = default;

    // Non-copyable
    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;

    /**
     * Enqueue (Producer side)
     * Time: O(1) - wait-free
     * Memory Order: relaxed for local, release for publishing
     */
    bool try_enqueue(const T& item) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) % Size;

        // Check if queue is full (need acquire to see consumer's updates)
        if (next_tail == cached_head_) {
            cached_head_ = head_.load(std::memory_order_acquire);
            if (next_tail == cached_head_) {
                return false;  // Queue is full
            }
        }

        // Write data
        buffer_[current_tail].data = item;

        // Publish the data (release ensures data write is visible)
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    /**
     * Dequeue (Consumer side)
     * Time: O(1) - wait-free
     * Memory Order: acquire for reading, relaxed for local
     */
    std::optional<T> try_dequeue() {
        size_t current_head = head_.load(std::memory_order_relaxed);

        // Check if queue is empty (need acquire to see producer's updates)
        if (current_head == cached_tail_) {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (current_head == cached_tail_) {
                return std::nullopt;  // Queue is empty
            }
        }

        // Read data
        T item = buffer_[current_head].data;
        size_t next_head = (current_head + 1) % Size;

        // Publish consumption (release ensures read is complete)
        head_.store(next_head, std::memory_order_release);
        return item;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    size_t size() const {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);
        return (t >= h) ? (t - h) : (Size - h + t);
    }
};

// Example 2: Multi-Producer Multi-Consumer (MPMC) Lock-Free Queue
// More complex, uses CAS operations
template<typename T>
class MPMCQueue {
private:
    struct Node {
        std::atomic<Node*> next;
        T data;

        Node() : next(nullptr) {}
        explicit Node(const T& value) : next(nullptr), data(value) {}
    };

    alignas(CACHE_LINE_SIZE) std::atomic<Node*> head_;
    alignas(CACHE_LINE_SIZE) std::atomic<Node*> tail_;

public:
    MPMCQueue() {
        Node* dummy = new Node();
        head_.store(dummy, std::memory_order_relaxed);
        tail_.store(dummy, std::memory_order_relaxed);
    }

    ~MPMCQueue() {
        while (auto item = try_dequeue()) {}
        delete head_.load();
    }

    /**
     * Enqueue (Thread-safe for multiple producers)
     * Time: O(1) - lock-free (may retry on contention)
     * Uses CAS (Compare-And-Swap)
     */
    void enqueue(const T& item) {
        Node* new_node = new Node(item);
        Node* old_tail;

        while (true) {
            old_tail = tail_.load(std::memory_order_acquire);
            Node* next = old_tail->next.load(std::memory_order_acquire);

            // Check if tail is still the last node
            if (old_tail == tail_.load(std::memory_order_acquire)) {
                if (next == nullptr) {
                    // Try to link new node at the end
                    if (old_tail->next.compare_exchange_weak(
                            next, new_node,
                            std::memory_order_release,
                            std::memory_order_relaxed)) {
                        // Successfully linked, now update tail
                        tail_.compare_exchange_weak(
                            old_tail, new_node,
                            std::memory_order_release,
                            std::memory_order_relaxed);
                        return;
                    }
                } else {
                    // Help other thread complete its enqueue
                    tail_.compare_exchange_weak(
                        old_tail, next,
                        std::memory_order_release,
                        std::memory_order_relaxed);
                }
            }
        }
    }

    /**
     * Dequeue (Thread-safe for multiple consumers)
     * Time: O(1) - lock-free (may retry on contention)
     */
    std::optional<T> try_dequeue() {
        while (true) {
            Node* old_head = head_.load(std::memory_order_acquire);
            Node* tail = tail_.load(std::memory_order_acquire);
            Node* next = old_head->next.load(std::memory_order_acquire);

            if (old_head == head_.load(std::memory_order_acquire)) {
                if (old_head == tail) {
                    if (next == nullptr) {
                        return std::nullopt;  // Queue is empty
                    }
                    // Tail is falling behind, help advance it
                    tail_.compare_exchange_weak(
                        tail, next,
                        std::memory_order_release,
                        std::memory_order_relaxed);
                } else {
                    if (next == nullptr) {
                        continue;  // Retry
                    }

                    T value = next->data;

                    // Try to move head forward
                    if (head_.compare_exchange_weak(
                            old_head, next,
                            std::memory_order_release,
                            std::memory_order_relaxed)) {
                        delete old_head;  // Reclaim old dummy node
                        return value;
                    }
                }
            }
        }
    }
};

// Example 3: Bounded MPMC Queue with Sequence Numbers
// Michael-Scott style with bounded buffer
template<typename T, size_t Size>
class BoundedMPMCQueue {
private:
    struct Cell {
        std::atomic<size_t> sequence;
        T data;
    };

    alignas(CACHE_LINE_SIZE) std::atomic<size_t> enqueue_pos_{0};
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> dequeue_pos_{0};
    alignas(CACHE_LINE_SIZE) Cell buffer_[Size];

public:
    BoundedMPMCQueue() {
        for (size_t i = 0; i < Size; ++i) {
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    bool try_enqueue(const T& item) {
        Cell* cell;
        size_t pos = enqueue_pos_.load(std::memory_order_relaxed);

        while (true) {
            cell = &buffer_[pos % Size];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);

            if (diff == 0) {
                // Cell is available for writing
                if (enqueue_pos_.compare_exchange_weak(
                        pos, pos + 1,
                        std::memory_order_relaxed,
                        std::memory_order_relaxed)) {
                    break;
                }
            } else if (diff < 0) {
                return false;  // Queue is full
            } else {
                // Another thread claimed this position
                pos = enqueue_pos_.load(std::memory_order_relaxed);
            }
        }

        cell->data = item;
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    std::optional<T> try_dequeue() {
        Cell* cell;
        size_t pos = dequeue_pos_.load(std::memory_order_relaxed);

        while (true) {
            cell = &buffer_[pos % Size];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);

            if (diff == 0) {
                // Cell is available for reading
                if (dequeue_pos_.compare_exchange_weak(
                        pos, pos + 1,
                        std::memory_order_relaxed,
                        std::memory_order_relaxed)) {
                    break;
                }
            } else if (diff < 0) {
                return std::nullopt;  // Queue is empty
            } else {
                // Another thread claimed this position
                pos = dequeue_pos_.load(std::memory_order_relaxed);
            }
        }

        T value = cell->data;
        cell->sequence.store(pos + Size, std::memory_order_release);
        return value;
    }
};

// Example: Market Order for testing
struct MarketOrder {
    char symbol[8];
    double price;
    int quantity;
    uint64_t timestamp;

    MarketOrder() = default;
    MarketOrder(const char* sym, double p, int q, uint64_t ts)
        : price(p), quantity(q), timestamp(ts) {
        strncpy(symbol, sym, 7);
        symbol[7] = '\0';
    }
};

// Performance test for SPSC Queue
void test_spsc_queue() {
    std::cout << "\n=== SPSC Queue Performance Test ===" << std::endl;

    constexpr size_t QUEUE_SIZE = 1024;
    constexpr size_t NUM_ITEMS = 1000000;

    SPSCQueue<MarketOrder, QUEUE_SIZE> queue;

    auto producer = [&queue]() {
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            MarketOrder order("AAPL", 150.0 + i * 0.01, 100, i);
            while (!queue.try_enqueue(order)) {
                // Spin until space available
                std::this_thread::yield();
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        std::cout << "Producer: " << NUM_ITEMS << " items in "
                  << duration.count() / 1000000.0 << " ms" << std::endl;
        std::cout << "Average: " << duration.count() / NUM_ITEMS << " ns/item" << std::endl;
    };

    auto consumer = [&queue]() {
        size_t count = 0;
        auto start = std::chrono::high_resolution_clock::now();

        while (count < NUM_ITEMS) {
            if (auto order = queue.try_dequeue()) {
                ++count;
            } else {
                std::this_thread::yield();
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        std::cout << "Consumer: " << count << " items in "
                  << duration.count() / 1000000.0 << " ms" << std::endl;
    };

    std::thread prod(producer);
    std::thread cons(consumer);

    prod.join();
    cons.join();
}

// Performance test for MPMC Queue
void test_mpmc_queue() {
    std::cout << "\n=== MPMC Queue Performance Test ===" << std::endl;

    constexpr size_t NUM_PRODUCERS = 2;
    constexpr size_t NUM_CONSUMERS = 2;
    constexpr size_t ITEMS_PER_PRODUCER = 100000;

    BoundedMPMCQueue<MarketOrder, 1024> queue;
    std::atomic<size_t> total_consumed{0};

    auto producer = [&queue](int id) {
        for (size_t i = 0; i < ITEMS_PER_PRODUCER; ++i) {
            MarketOrder order("AAPL", 150.0 + i * 0.01, 100, i);
            while (!queue.try_enqueue(order)) {
                std::this_thread::yield();
            }
        }
        std::cout << "Producer " << id << " finished" << std::endl;
    };

    auto consumer = [&queue, &total_consumed](int id) {
        size_t local_count = 0;
        while (total_consumed.load() < NUM_PRODUCERS * ITEMS_PER_PRODUCER) {
            if (auto order = queue.try_dequeue()) {
                ++local_count;
                ++total_consumed;
            } else {
                std::this_thread::yield();
            }
        }
        std::cout << "Consumer " << id << " processed " << local_count << " items" << std::endl;
    };

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (size_t i = 0; i < NUM_PRODUCERS; ++i) {
        threads.emplace_back(producer, i);
    }
    for (size_t i = 0; i < NUM_CONSUMERS; ++i) {
        threads.emplace_back(consumer, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Total time: " << duration.count() << " ms" << std::endl;
    std::cout << "Total items: " << total_consumed.load() << std::endl;
}

/**
 * KEY INTERVIEW POINTS:
 *
 * 1. Memory Ordering:
 *    - relaxed: No synchronization, only atomicity
 *    - acquire: Prevents reads/writes from moving before
 *    - release: Prevents reads/writes from moving after
 *    - acq_rel: Both acquire and release
 *    - seq_cst: Sequential consistency (strongest, slowest)
 *
 * 2. Cache Line Padding:
 *    - Prevents false sharing between threads
 *    - CPU cache lines are typically 64 bytes
 *    - Producer and consumer on different cache lines
 *    - Critical for performance in multi-core systems
 *
 * 3. ABA Problem:
 *    - Thread A reads value X
 *    - Thread B changes X to Y then back to X
 *    - Thread A's CAS succeeds but state changed
 *    - Solution: Tagged pointers, sequence numbers
 *
 * 4. Wait-Free vs Lock-Free:
 *    - Wait-free: Guaranteed progress in finite steps (SPSC)
 *    - Lock-free: System makes progress (MPMC)
 *    - Lock-based: May block indefinitely
 *
 * 5. Performance Characteristics:
 *    - SPSC: ~10-20ns latency (wait-free)
 *    - MPMC: ~50-100ns latency (lock-free with CAS)
 *    - Mutex-based: ~100-500ns (with contention)
 *
 * 6. When to Use Which:
 *    - SPSC: Market data thread → Strategy thread
 *    - MPMC: Multiple strategies → Order router
 *    - Bounded: Known max throughput, predictable memory
 *    - Unbounded: Variable load, may have memory spikes
 *
 * 7. Common Interview Questions:
 *    Q: Why use lock-free queues in HFT?
 *    A: Eliminate lock contention, predictable latency,
 *       no priority inversion, no deadlocks
 *
 *    Q: What is false sharing?
 *    A: Multiple threads accessing different variables
 *       on same cache line, causing cache invalidation
 *
 *    Q: How to handle memory reclamation?
 *    A: Hazard pointers, RCU, reference counting,
 *       or single-threaded allocation/deallocation
 *
 * 8. Best Practices for HFT:
 *    - Use SPSC when possible (fastest)
 *    - Pad to cache line size (64 bytes)
 *    - Use relaxed ordering when safe
 *    - Batch operations when possible
 *    - Pin threads to cores
 *    - Profile with different memory orders
 *
 * 9. Trade-offs:
 *    - SPSC: Fastest but limited to 1-to-1
 *    - MPMC: Flexible but slower and complex
 *    - Bounded: Fixed memory but may block when full
 *    - Unbounded: Never blocks but unbounded memory
 *
 * Time Complexity:
 * - SPSC enqueue/dequeue: O(1) wait-free
 * - MPMC enqueue/dequeue: O(1) lock-free (may retry)
 *
 * Space Complexity:
 * - Bounded: O(capacity)
 * - Unbounded: O(max_size_reached)
 */

int main() {
    test_spsc_queue();
    test_mpmc_queue();
    return 0;
}

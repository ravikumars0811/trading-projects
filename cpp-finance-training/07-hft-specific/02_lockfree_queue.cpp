/*
 * Lock-Free SPSC (Single Producer Single Consumer) Queue
 * HFT-Critical: Zero-latency message passing between threads
 */

#include <atomic>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>

// Cache line size (typical modern CPUs)
constexpr size_t CACHE_LINE_SIZE = 64;

// Simple message structure for market data
struct MarketTick {
    char symbol[8];
    double price;
    int volume;
    uint64_t timestamp;

    MarketTick() : price(0.0), volume(0), timestamp(0) {
        symbol[0] = '\0';
    }

    MarketTick(const char* sym, double p, int v, uint64_t ts)
        : price(p), volume(v), timestamp(ts) {
        strncpy(symbol, sym, sizeof(symbol) - 1);
        symbol[sizeof(symbol) - 1] = '\0';
    }

    void display() const {
        std::cout << symbol << ": $" << price
                  << " vol=" << volume
                  << " ts=" << timestamp << std::endl;
    }
};

/*
 * Lock-Free SPSC Ring Buffer Queue
 *
 * Key Features:
 * - Wait-free for producer and consumer
 * - Cache-line aligned to avoid false sharing
 * - Power-of-2 size for fast modulo via bitwise AND
 * - Memory ordering guarantees correct visibility
 */
template<typename T, size_t Size>
class SPSCQueue {
private:
    static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");

    // Separate cache lines for head and tail to avoid false sharing
    struct alignas(CACHE_LINE_SIZE) AlignedIndex {
        std::atomic<size_t> value;
        char padding[CACHE_LINE_SIZE - sizeof(std::atomic<size_t>)];

        AlignedIndex() : value(0) {}
    };

    AlignedIndex head_;  // Consumer index (reads from here)
    AlignedIndex tail_;  // Producer index (writes to here)
    T buffer_[Size];

    static constexpr size_t mask_ = Size - 1;

public:
    SPSCQueue() {
        head_.value.store(0, std::memory_order_relaxed);
        tail_.value.store(0, std::memory_order_relaxed);
    }

    // Producer: Push element (returns false if queue full)
    bool push(const T& item) {
        const size_t current_tail = tail_.value.load(std::memory_order_relaxed);
        const size_t next_tail = (current_tail + 1) & mask_;

        // Check if queue is full
        // We need acquire semantics to see consumer's updates
        if (next_tail == head_.value.load(std::memory_order_acquire)) {
            return false;  // Queue full
        }

        // Write data
        buffer_[current_tail] = item;

        // Publish the write with release semantics
        // Ensures buffer write happens before tail update
        tail_.value.store(next_tail, std::memory_order_release);

        return true;
    }

    // Consumer: Pop element (returns false if queue empty)
    bool pop(T& item) {
        const size_t current_head = head_.value.load(std::memory_order_relaxed);

        // Check if queue is empty
        // We need acquire semantics to see producer's updates
        if (current_head == tail_.value.load(std::memory_order_acquire)) {
            return false;  // Queue empty
        }

        // Read data
        item = buffer_[current_head];

        // Publish the read with release semantics
        const size_t next_head = (current_head + 1) & mask_;
        head_.value.store(next_head, std::memory_order_release);

        return true;
    }

    // Check if queue is empty (not thread-safe for size, but safe for empty check)
    bool empty() const {
        return head_.value.load(std::memory_order_acquire) ==
               tail_.value.load(std::memory_order_acquire);
    }

    // Approximate size (may not be accurate due to concurrent access)
    size_t size() const {
        size_t head = head_.value.load(std::memory_order_acquire);
        size_t tail = tail_.value.load(std::memory_order_acquire);
        return (tail - head) & mask_;
    }

    // Maximum capacity
    static constexpr size_t capacity() {
        return Size - 1;  // One slot reserved to distinguish full from empty
    }
};

// Performance test
void performanceTest() {
    std::cout << "\n=== Performance Test ===" << std::endl;

    constexpr size_t QUEUE_SIZE = 1024;
    constexpr size_t NUM_MESSAGES = 10'000'000;

    SPSCQueue<MarketTick, QUEUE_SIZE> queue;

    // Producer thread
    std::thread producer([&queue]() {
        for (size_t i = 0; i < NUM_MESSAGES; ++i) {
            MarketTick tick("AAPL", 150.25 + (i * 0.01), 100, i);

            // Busy wait if queue full
            while (!queue.push(tick)) {
                // In production, might yield or use backoff strategy
                std::this_thread::yield();
            }
        }
    });

    // Consumer thread
    std::atomic<size_t> consumed{0};
    auto start = std::chrono::high_resolution_clock::now();

    std::thread consumer([&queue, &consumed]() {
        MarketTick tick;
        size_t count = 0;

        while (count < NUM_MESSAGES) {
            if (queue.pop(tick)) {
                count++;
            } else {
                // Queue empty, busy wait
                std::this_thread::yield();
            }
        }

        consumed.store(count);
    });

    producer.join();
    consumer.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Messages processed: " << consumed.load() << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    std::cout << "Throughput: " << (NUM_MESSAGES / (duration.count() / 1000.0)) / 1'000'000
              << " million msg/sec" << std::endl;

    auto avg_latency_ns = (duration.count() * 1'000'000.0) / NUM_MESSAGES;
    std::cout << "Average latency: " << avg_latency_ns << " ns/msg" << std::endl;
}

// Functional test
void functionalTest() {
    std::cout << "\n=== Functional Test ===" << std::endl;

    SPSCQueue<MarketTick, 8> queue;

    std::cout << "Queue capacity: " << queue.capacity() << std::endl;
    std::cout << "Queue empty: " << (queue.empty() ? "Yes" : "No") << std::endl;

    // Push some data
    std::cout << "\nPushing 5 ticks..." << std::endl;
    queue.push(MarketTick("AAPL", 150.25, 100, 1000));
    queue.push(MarketTick("MSFT", 375.50, 200, 1001));
    queue.push(MarketTick("GOOGL", 140.75, 150, 1002));
    queue.push(MarketTick("AMZN", 185.30, 75, 1003));
    queue.push(MarketTick("TSLA", 242.15, 300, 1004));

    std::cout << "Queue size: " << queue.size() << std::endl;
    std::cout << "Queue empty: " << (queue.empty() ? "Yes" : "No") << std::endl;

    // Pop some data
    std::cout << "\nPopping 3 ticks..." << std::endl;
    MarketTick tick;
    for (int i = 0; i < 3; ++i) {
        if (queue.pop(tick)) {
            std::cout << "  Popped: ";
            tick.display();
        }
    }

    std::cout << "Queue size: " << queue.size() << std::endl;

    // Try to fill queue
    std::cout << "\nFilling queue..." << std::endl;
    int pushed = 0;
    while (queue.push(MarketTick("TEST", 100.0, 10, pushed))) {
        pushed++;
    }
    std::cout << "Pushed " << pushed << " more items" << std::endl;
    std::cout << "Queue size: " << queue.size() << " (full)" << std::endl;

    // Try to push one more (should fail)
    if (!queue.push(MarketTick("FAIL", 0.0, 0, 0))) {
        std::cout << "Cannot push to full queue (correct!)" << std::endl;
    }

    // Drain queue
    std::cout << "\nDraining queue..." << std::endl;
    int popped = 0;
    while (queue.pop(tick)) {
        popped++;
    }
    std::cout << "Popped " << popped << " items" << std::endl;
    std::cout << "Queue empty: " << (queue.empty() ? "Yes" : "No") << std::endl;
}

int main() {
    std::cout << "=== Lock-Free SPSC Queue ===" << std::endl;
    std::cout << "Cache line size: " << CACHE_LINE_SIZE << " bytes" << std::endl;
    std::cout << "MarketTick size: " << sizeof(MarketTick) << " bytes" << std::endl;

    functionalTest();
    performanceTest();

    std::cout << "\n=== Design Principles ===" << std::endl;
    std::cout << "1. Cache-line alignment prevents false sharing" << std::endl;
    std::cout << "2. Power-of-2 size enables fast modulo via bitwise AND" << std::endl;
    std::cout << "3. Memory ordering ensures correct visibility" << std::endl;
    std::cout << "4. Single producer/consumer = no contention" << std::endl;
    std::cout << "5. Wait-free operations = bounded latency" << std::endl;

    std::cout << "\n=== Memory Ordering Explanation ===" << std::endl;
    std::cout << "relaxed: No synchronization, just atomicity" << std::endl;
    std::cout << "acquire: See all writes before release" << std::endl;
    std::cout << "release: Publish all writes before this" << std::endl;
    std::cout << "seq_cst: Total ordering (slowest)" << std::endl;

    std::cout << "\n=== Production Enhancements ===" << std::endl;
    std::cout << "1. Add batching for better throughput" << std::endl;
    std::cout << "2. Implement backoff strategy when full/empty" << std::endl;
    std::cout << "3. Add statistics (drops, latency)" << std::endl;
    std::cout << "4. Use aligned_alloc for buffer" << std::endl;
    std::cout << "5. Template on memory order for flexibility" << std::endl;
    std::cout << "6. Add TryPush/TryPop with timeout" << std::endl;

    return 0;
}

/*
 * Key Takeaways:
 * 1. Lock-free != wait-free (this one is wait-free)
 * 2. SPSC is simpler than MPMC (multi-producer multi-consumer)
 * 3. Cache-line alignment critical for performance
 * 4. Memory ordering must be correct for correctness
 * 5. Power-of-2 size enables fast wraparound
 *
 * Why Lock-Free for HFT:
 * - No context switches from blocking
 * - Predictable latency (no worst-case lock contention)
 * - Better throughput on modern CPUs
 * - Scales to many cores
 * - No priority inversion
 *
 * Trade-offs:
 * - More complex to implement correctly
 * - Harder to debug
 * - May use more CPU (busy waiting)
 * - SPSC limitation (one producer, one consumer)
 *
 * When to Use:
 * - Market data thread → strategy thread
 * - Strategy thread → order thread
 * - Tick data → aggregation thread
 * - Any high-frequency producer-consumer pattern
 */

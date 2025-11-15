/**
 * Thread Pool Implementation for HFT
 *
 * Applications in HFT:
 * - Parallel order processing
 * - Concurrent strategy execution
 * - Batch market data processing
 * - Risk calculations
 *
 * Interview Focus:
 * - Work queue management
 * - Thread synchronization
 * - Task scheduling
 * - Graceful shutdown
 *
 * Complexity:
 * - Submit task: O(1) amortized
 * - Space: O(queue_size + num_threads)
 */

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <chrono>

// Basic Thread Pool
class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    std::atomic<size_t> active_tasks_;

public:
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false), active_tasks_(0) {

        workers_.reserve(num_threads);

        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i] {
                std::cout << "Worker " << i << " started" << std::endl;

                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(mutex_);

                        // Wait for task or stop signal
                        condition_.wait(lock, [this] {
                            return stop_.load() || !tasks_.empty();
                        });

                        if (stop_.load() && tasks_.empty()) {
                            break;
                        }

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }

                    ++active_tasks_;
                    task();
                    --active_tasks_;
                }

                std::cout << "Worker " << i << " stopped" << std::endl;
            });
        }
    }

    ~ThreadPool() {
        shutdown();
    }

    // Submit a task and get a future
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type> {

        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (stop_.load()) {
                throw std::runtime_error("Cannot submit to stopped ThreadPool");
            }

            tasks_.emplace([task]() { (*task)(); });
        }

        condition_.notify_one();
        return result;
    }

    // Wait for all tasks to complete
    void wait() {
        while (active_tasks_.load() > 0 || !tasks_.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // Graceful shutdown
    void shutdown() {
        if (!stop_.load()) {
            stop_.store(true);
            condition_.notify_all();

            for (auto& worker : workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }
    }

    size_t num_threads() const {
        return workers_.size();
    }

    size_t pending_tasks() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return tasks_.size();
    }
};

// Advanced: Work-Stealing Thread Pool
class WorkStealingThreadPool {
private:
    struct WorkerThread {
        std::thread thread;
        std::deque<std::function<void()>> tasks;
        std::mutex mutex;
        std::condition_variable cv;
        std::atomic<bool> stop{false};
    };

    std::vector<std::unique_ptr<WorkerThread>> workers_;
    std::atomic<size_t> next_worker_{0};

public:
    explicit WorkStealingThreadPool(size_t num_threads) {
        workers_.reserve(num_threads);

        for (size_t i = 0; i < num_threads; ++i) {
            auto worker = std::make_unique<WorkerThread>();

            worker->thread = std::thread([this, i, w = worker.get()] {
                while (!w->stop.load()) {
                    std::function<void()> task;

                    // Try to get task from own queue
                    {
                        std::unique_lock<std::mutex> lock(w->mutex);
                        if (!w->tasks.empty()) {
                            task = std::move(w->tasks.front());
                            w->tasks.pop_front();
                        }
                    }

                    // If no task, try to steal from other workers
                    if (!task) {
                        for (size_t j = 0; j < workers_.size(); ++j) {
                            if (j == i) continue;

                            auto& other = workers_[j];
                            std::unique_lock<std::mutex> lock(other->mutex, std::try_to_lock);

                            if (lock.owns_lock() && !other->tasks.empty()) {
                                // Steal from back (LIFO for cache locality)
                                task = std::move(other->tasks.back());
                                other->tasks.pop_back();
                                break;
                            }
                        }
                    }

                    if (task) {
                        task();
                    } else {
                        // No work available, wait briefly
                        std::unique_lock<std::mutex> lock(w->mutex);
                        w->cv.wait_for(lock, std::chrono::milliseconds(10));
                    }
                }
            });

            workers_.push_back(std::move(worker));
        }
    }

    ~WorkStealingThreadPool() {
        for (auto& worker : workers_) {
            worker->stop.store(true);
            worker->cv.notify_one();
            if (worker->thread.joinable()) {
                worker->thread.join();
            }
        }
    }

    template<typename F>
    void submit(F&& f) {
        // Round-robin task distribution
        size_t idx = next_worker_.fetch_add(1) % workers_.size();
        auto& worker = workers_[idx];

        {
            std::unique_lock<std::mutex> lock(worker->mutex);
            worker->tasks.emplace_back(std::forward<F>(f));
        }

        worker->cv.notify_one();
    }
};

// Example: Priority Thread Pool
template<typename Priority>
class PriorityThreadPool {
private:
    struct Task {
        Priority priority;
        std::function<void()> func;

        bool operator<(const Task& other) const {
            // Lower priority value = higher priority in queue
            return priority > other.priority;
        }
    };

    std::vector<std::thread> workers_;
    std::priority_queue<Task> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;

public:
    explicit PriorityThreadPool(size_t num_threads) : stop_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        condition_.wait(lock, [this] {
                            return stop_.load() || !tasks_.empty();
                        });

                        if (stop_.load() && tasks_.empty()) {
                            break;
                        }

                        task = std::move(tasks_.top().func);
                        tasks_.pop();
                    }

                    task();
                }
            });
        }
    }

    ~PriorityThreadPool() {
        stop_.store(true);
        condition_.notify_all();

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    template<typename F>
    void submit(Priority priority, F&& f) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.push({priority, std::forward<F>(f)});
        }
        condition_.notify_one();
    }
};

// HFT Application Examples

// Example 1: Parallel Order Validation
struct Order {
    int id;
    std::string symbol;
    double price;
    int quantity;
};

bool validateOrder(const Order& order) {
    // Simulate validation logic
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Check price range, quantity, etc.
    return order.price > 0 && order.quantity > 0;
}

// Example 2: Concurrent Risk Calculation
double calculatePortfolioRisk(const std::vector<Order>& orders) {
    // Simulate complex risk calculation
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    double total_value = 0.0;
    for (const auto& order : orders) {
        total_value += order.price * order.quantity;
    }

    return total_value * 0.02; // 2% risk factor
}

// Example 3: Market Data Processing
void processMarketData(const std::string& symbol, double price) {
    // Simulate market data processing
    std::cout << "Processing: " << symbol << " @ " << price << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

// Demonstration
void demonstrate_thread_pools() {
    std::cout << "\n=== Basic Thread Pool ===" << std::endl;
    {
        ThreadPool pool(4);

        // Submit tasks and collect futures
        std::vector<std::future<int>> results;

        for (int i = 0; i < 10; ++i) {
            results.push_back(pool.submit([i] {
                std::cout << "Task " << i << " running on thread "
                          << std::this_thread::get_id() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return i * i;
            }));
        }

        // Wait for all tasks and print results
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << "Task " << i << " result: "
                      << results[i].get() << std::endl;
        }
    }

    std::cout << "\n=== Order Validation Example ===" << std::endl;
    {
        ThreadPool pool(4);
        std::vector<Order> orders = {
            {1, "AAPL", 150.0, 100},
            {2, "GOOGL", 2800.0, 50},
            {3, "MSFT", 300.0, 75},
            {4, "TSLA", 700.0, 25},
            {5, "AMZN", 3300.0, 10}
        };

        std::vector<std::future<bool>> validation_results;

        for (const auto& order : orders) {
            validation_results.push_back(
                pool.submit(validateOrder, order)
            );
        }

        for (size_t i = 0; i < validation_results.size(); ++i) {
            bool valid = validation_results[i].get();
            std::cout << "Order " << orders[i].id
                      << (valid ? " VALID" : " INVALID") << std::endl;
        }
    }

    std::cout << "\n=== Priority Thread Pool ===" << std::endl;
    {
        PriorityThreadPool<int> pool(4);

        // Submit tasks with different priorities
        // Lower number = higher priority
        pool.submit(3, [] { std::cout << "Low priority task" << std::endl; });
        pool.submit(1, [] { std::cout << "High priority task" << std::endl; });
        pool.submit(2, [] { std::cout << "Medium priority task" << std::endl; });
        pool.submit(1, [] { std::cout << "Another high priority task" << std::endl; });

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "\n=== Work-Stealing Thread Pool ===" << std::endl;
    {
        WorkStealingThreadPool pool(4);

        // Submit many small tasks
        for (int i = 0; i < 20; ++i) {
            pool.submit([i] {
                std::cout << "Work-stealing task " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            });
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

/**
 * KEY INTERVIEW POINTS:
 *
 * 1. Thread Pool Benefits:
 *    - Avoid thread creation overhead
 *    - Limit concurrent threads (resource control)
 *    - Better CPU cache utilization
 *    - Centralized task scheduling
 *
 * 2. Design Decisions:
 *    - Queue type: FIFO, priority, work-stealing
 *    - Thread count: CPU count, workload dependent
 *    - Task granularity: Too small = overhead, too large = underutilization
 *    - Shutdown strategy: Graceful vs immediate
 *
 * 3. Synchronization:
 *    - Mutex: Protect shared queue
 *    - Condition variable: Wake sleeping threads
 *    - Atomic: Stop flag, counters
 *    - Future/Promise: Return values
 *
 * 4. Work Stealing:
 *    - Each thread has own queue
 *    - Idle threads steal from busy threads
 *    - Better load balancing
 *    - Cache-friendly (LIFO stealing)
 *
 * 5. Priority Queues:
 *    - Critical tasks first
 *    - Risk checks before order submission
 *    - Market orders before limit orders
 *    - Watch for starvation
 *
 * 6. Performance Considerations:
 *    - Thread creation: ~10-100 microseconds
 *    - Context switch: ~1-10 microseconds
 *    - Mutex lock/unlock: ~25-50 nanoseconds (uncontended)
 *    - Cache line contention: Major bottleneck
 *
 * 7. HFT Applications:
 *    - Order validation (CPU-bound)
 *    - Risk calculations (CPU-bound)
 *    - Market data processing (mixed)
 *    - Strategy backtesting (CPU-bound)
 *    - Log writing (I/O-bound)
 *
 * 8. Common Interview Questions:
 *    Q: How many threads to create?
 *    A: CPU-bound: num_cores
 *       I/O-bound: More than cores (2-4x)
 *       Measure and tune for workload
 *
 *    Q: Thread pool vs creating threads on-demand?
 *    A: Pool: Lower latency, bounded resources
 *       On-demand: Simpler, unbounded resources
 *       HFT: Always use pool for predictability
 *
 *    Q: How to handle shutdown?
 *    A: Set stop flag, notify all threads
 *       Optionally wait for pending tasks
 *       Join all threads, cleanup resources
 *
 *    Q: What if task throws exception?
 *    A: Caught by future, propagated on get()
 *       Worker thread continues running
 *       Log exception for debugging
 *
 * 9. Advanced Techniques:
 *    - Thread affinity (pin to CPU cores)
 *    - NUMA-aware scheduling
 *    - Lock-free queues
 *    - Fiber/coroutine pools
 *    - GPU compute pools
 *
 * 10. Best Practices:
 *     - Size based on workload profile
 *     - Monitor queue depth
 *     - Set task size limits
 *     - Implement backpressure
 *     - Profile contention points
 *     - Use move semantics for tasks
 *     - Consider task priorities
 *     - Implement graceful degradation
 *
 * Time Complexity:
 * - Submit: O(1) amortized (O(log n) for priority queue)
 * - Shutdown: O(n) where n is pending tasks
 *
 * Space Complexity:
 * - O(threads + pending_tasks)
 */

int main() {
    demonstrate_thread_pools();
    return 0;
}

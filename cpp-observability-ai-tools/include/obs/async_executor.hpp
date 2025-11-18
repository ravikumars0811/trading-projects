#pragma once

#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace obs::utils {

// Thread pool for async execution
class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    ~ThreadPool();

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (stop_) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return result;
    }

    size_t queue_size() const;
    size_t num_threads() const { return workers_.size(); }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool stop_{false};
};

// Async executor with scheduling capabilities
class AsyncExecutor {
public:
    static AsyncExecutor& instance();

    // Execute task asynchronously
    template<typename F>
    std::future<typename std::invoke_result<F>::type> execute(F&& f) {
        return thread_pool_->submit(std::forward<F>(f));
    }

    // Schedule task for later execution
    template<typename F>
    void schedule(F&& f, std::chrono::milliseconds delay) {
        execute([f = std::forward<F>(f), delay]() {
            std::this_thread::sleep_for(delay);
            f();
        });
    }

    // Execute task periodically
    template<typename F>
    void schedule_periodic(F&& f, std::chrono::milliseconds interval) {
        auto keep_running = std::make_shared<std::atomic<bool>>(true);
        execute([f = std::forward<F>(f), interval, keep_running]() {
            while (keep_running->load()) {
                f();
                std::this_thread::sleep_for(interval);
            }
        });
    }

    // Configuration
    void resize_pool(size_t num_threads);
    size_t get_queue_size() const;

private:
    AsyncExecutor();
    ~AsyncExecutor() = default;
    AsyncExecutor(const AsyncExecutor&) = delete;
    AsyncExecutor& operator=(const AsyncExecutor&) = delete;

    std::unique_ptr<ThreadPool> thread_pool_;
};

} // namespace obs::utils

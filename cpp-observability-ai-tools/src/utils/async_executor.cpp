#include "obs/async_executor.hpp"

namespace obs::utils {

ThreadPool::ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait(lock, [this]() {
                        return stop_ || !tasks_.empty();
                    });

                    if (stop_ && tasks_.empty()) {
                        return;
                    }

                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }
    condition_.notify_all();

    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

size_t ThreadPool::queue_size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tasks_.size();
}

AsyncExecutor& AsyncExecutor::instance() {
    static AsyncExecutor instance;
    return instance;
}

AsyncExecutor::AsyncExecutor()
    : thread_pool_(std::make_unique<ThreadPool>()) {}

void AsyncExecutor::resize_pool(size_t num_threads) {
    thread_pool_ = std::make_unique<ThreadPool>(num_threads);
}

size_t AsyncExecutor::get_queue_size() const {
    return thread_pool_->queue_size();
}

} // namespace obs::utils

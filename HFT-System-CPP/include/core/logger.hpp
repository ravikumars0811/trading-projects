#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <thread>
#include "lock_free_queue.hpp"

namespace hft {
namespace core {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/**
 * Asynchronous logger with minimal overhead
 * Uses lock-free queue for message passing
 */
class Logger {
public:
    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    void init(const std::string& log_file, LogLevel level = LogLevel::INFO);
    void shutdown();

    void log(LogLevel level, const std::string& message);

    template<typename... Args>
    void debug(Args&&... args) {
        if (log_level_ <= LogLevel::DEBUG) {
            log(LogLevel::DEBUG, format(std::forward<Args>(args)...));
        }
    }

    template<typename... Args>
    void info(Args&&... args) {
        if (log_level_ <= LogLevel::INFO) {
            log(LogLevel::INFO, format(std::forward<Args>(args)...));
        }
    }

    template<typename... Args>
    void warning(Args&&... args) {
        if (log_level_ <= LogLevel::WARNING) {
            log(LogLevel::WARNING, format(std::forward<Args>(args)...));
        }
    }

    template<typename... Args>
    void error(Args&&... args) {
        if (log_level_ <= LogLevel::ERROR) {
            log(LogLevel::ERROR, format(std::forward<Args>(args)...));
        }
    }

private:
    Logger() = default;
    ~Logger();

    template<typename... Args>
    std::string format(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << args);
        return oss.str();
    }

    void writerThread();
    std::string levelToString(LogLevel level) const;

    struct LogMessage {
        LogLevel level;
        std::string message;
        uint64_t timestamp;
    };

    std::ofstream log_file_;
    LogLevel log_level_ = LogLevel::INFO;
    LockFreeQueue<LogMessage, 32768> message_queue_;
    std::atomic<bool> running_{false};
    std::thread writer_thread_;
};

// Convenience macros
#define LOG_DEBUG(...) hft::core::Logger::instance().debug(__VA_ARGS__)
#define LOG_INFO(...) hft::core::Logger::instance().info(__VA_ARGS__)
#define LOG_WARNING(...) hft::core::Logger::instance().warning(__VA_ARGS__)
#define LOG_ERROR(...) hft::core::Logger::instance().error(__VA_ARGS__)

} // namespace core
} // namespace hft

#include "core/logger.hpp"
#include "core/timer.hpp"
#include <iostream>

namespace hft {
namespace core {

Logger::~Logger() {
    shutdown();
}

void Logger::init(const std::string& log_file, LogLevel level) {
    log_level_ = level;
    log_file_.open(log_file, std::ios::out | std::ios::app);

    if (!log_file_.is_open()) {
        std::cerr << "Failed to open log file: " << log_file << std::endl;
        return;
    }

    running_ = true;
    writer_thread_ = std::thread(&Logger::writerThread, this);
}

void Logger::shutdown() {
    if (running_.exchange(false)) {
        if (writer_thread_.joinable()) {
            writer_thread_.join();
        }
        log_file_.close();
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    LogMessage msg{level, message, Timer::timestamp_ns()};

    // If queue is full, drop the message (in production, you might want to handle this differently)
    message_queue_.push(std::move(msg));
}

void Logger::writerThread() {
    while (running_ || !message_queue_.empty()) {
        auto msg = message_queue_.pop();
        if (msg.has_value()) {
            auto timestamp = msg->timestamp;
            auto seconds = timestamp / 1000000000;
            auto nanos = timestamp % 1000000000;

            log_file_ << "[" << seconds << "." << std::setfill('0') << std::setw(9) << nanos << "] "
                     << "[" << levelToString(msg->level) << "] "
                     << msg->message << std::endl;
            log_file_.flush();
        } else {
            // Queue is empty, sleep briefly
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRIT";
        default: return "UNKNOWN";
    }
}

} // namespace core
} // namespace hft

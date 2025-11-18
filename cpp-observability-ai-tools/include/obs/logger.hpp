#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <sstream>
#include <chrono>
#include <nlohmann/json.hpp>

namespace obs {

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

struct LogEntry {
    LogLevel level;
    std::string message;
    std::string logger_name;
    std::string file;
    int line;
    std::string function;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> context;
};

class Logger {
public:
    static Logger& instance();

    void log(LogLevel level,
            const std::string& message,
            const std::string& logger_name = "default",
            const std::string& file = "",
            int line = 0,
            const std::string& function = "");

    void log_with_context(LogLevel level,
                         const std::string& message,
                         const std::unordered_map<std::string, std::string>& context,
                         const std::string& logger_name = "default");

    // Configuration
    void set_level(LogLevel level);
    void set_output_file(const std::string& filename);
    void enable_console(bool enable);
    void enable_json_format(bool enable);

    // Query
    std::vector<LogEntry> get_recent_logs(size_t limit = 100) const;
    std::vector<LogEntry> get_logs_by_level(LogLevel level) const;
    nlohmann::json export_json() const;

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void write_log(const LogEntry& entry);
    std::string level_to_string(LogLevel level) const;

    mutable std::mutex mutex_;
    LogLevel min_level_{LogLevel::INFO};
    bool console_enabled_{true};
    bool json_format_{false};
    std::ofstream file_stream_;
    std::vector<LogEntry> recent_logs_;
    size_t max_recent_logs_{1000};
};

// Convenience macros
#define LOG_TRACE(msg) obs::Logger::instance().log(obs::LogLevel::TRACE, msg, "default", __FILE__, __LINE__, __FUNCTION__)
#define LOG_DEBUG(msg) obs::Logger::instance().log(obs::LogLevel::DEBUG, msg, "default", __FILE__, __LINE__, __FUNCTION__)
#define LOG_INFO(msg) obs::Logger::instance().log(obs::LogLevel::INFO, msg, "default", __FILE__, __LINE__, __FUNCTION__)
#define LOG_WARN(msg) obs::Logger::instance().log(obs::LogLevel::WARN, msg, "default", __FILE__, __LINE__, __FUNCTION__)
#define LOG_ERROR(msg) obs::Logger::instance().log(obs::LogLevel::ERROR, msg, "default", __FILE__, __LINE__, __FUNCTION__)
#define LOG_FATAL(msg) obs::Logger::instance().log(obs::LogLevel::FATAL, msg, "default", __FILE__, __LINE__, __FUNCTION__)

} // namespace obs

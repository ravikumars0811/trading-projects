#include "obs/logger.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>

namespace obs {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() = default;

Logger::~Logger() {
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
}

std::string Logger::level_to_string(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

void Logger::log(LogLevel level,
                const std::string& message,
                const std::string& logger_name,
                const std::string& file,
                int line,
                const std::string& function) {
    if (level < min_level_) {
        return;
    }

    LogEntry entry;
    entry.level = level;
    entry.message = message;
    entry.logger_name = logger_name;
    entry.file = file;
    entry.line = line;
    entry.function = function;
    entry.timestamp = std::chrono::system_clock::now();

    write_log(entry);
}

void Logger::log_with_context(LogLevel level,
                             const std::string& message,
                             const std::unordered_map<std::string, std::string>& context,
                             const std::string& logger_name) {
    if (level < min_level_) {
        return;
    }

    LogEntry entry;
    entry.level = level;
    entry.message = message;
    entry.logger_name = logger_name;
    entry.context = context;
    entry.timestamp = std::chrono::system_clock::now();

    write_log(entry);
}

void Logger::write_log(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Store in recent logs
    recent_logs_.push_back(entry);
    if (recent_logs_.size() > max_recent_logs_) {
        recent_logs_.erase(recent_logs_.begin());
    }

    // Format timestamp
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    std::ostringstream timestamp_str;
    timestamp_str << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");

    if (json_format_) {
        nlohmann::json log_json;
        log_json["timestamp"] = timestamp_str.str();
        log_json["level"] = level_to_string(entry.level);
        log_json["logger"] = entry.logger_name;
        log_json["message"] = entry.message;
        if (!entry.file.empty()) {
            log_json["file"] = entry.file;
            log_json["line"] = entry.line;
            log_json["function"] = entry.function;
        }
        if (!entry.context.empty()) {
            log_json["context"] = entry.context;
        }

        std::string output = log_json.dump();

        if (console_enabled_) {
            std::cout << output << std::endl;
        }
        if (file_stream_.is_open()) {
            file_stream_ << output << std::endl;
        }
    } else {
        std::ostringstream oss;
        oss << "[" << timestamp_str.str() << "] "
            << "[" << level_to_string(entry.level) << "] "
            << "[" << entry.logger_name << "] "
            << entry.message;

        if (!entry.file.empty()) {
            oss << " (" << entry.file << ":" << entry.line << " " << entry.function << ")";
        }

        if (!entry.context.empty()) {
            oss << " {";
            bool first = true;
            for (const auto& [key, value] : entry.context) {
                if (!first) oss << ", ";
                oss << key << "=" << value;
                first = false;
            }
            oss << "}";
        }

        std::string output = oss.str();

        if (console_enabled_) {
            if (entry.level >= LogLevel::ERROR) {
                std::cerr << output << std::endl;
            } else {
                std::cout << output << std::endl;
            }
        }
        if (file_stream_.is_open()) {
            file_stream_ << output << std::endl;
        }
    }
}

void Logger::set_level(LogLevel level) {
    min_level_ = level;
}

void Logger::set_output_file(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
    file_stream_.open(filename, std::ios::app);
}

void Logger::enable_console(bool enable) {
    console_enabled_ = enable;
}

void Logger::enable_json_format(bool enable) {
    json_format_ = enable;
}

std::vector<LogEntry> Logger::get_recent_logs(size_t limit) const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t start = recent_logs_.size() > limit ? recent_logs_.size() - limit : 0;
    return std::vector<LogEntry>(recent_logs_.begin() + start, recent_logs_.end());
}

std::vector<LogEntry> Logger::get_logs_by_level(LogLevel level) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<LogEntry> result;

    std::copy_if(recent_logs_.begin(), recent_logs_.end(), std::back_inserter(result),
                [level](const LogEntry& e) { return e.level == level; });

    return result;
}

nlohmann::json Logger::export_json() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result = nlohmann::json::array();

    for (const auto& entry : recent_logs_) {
        nlohmann::json log_json;
        auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");

        log_json["timestamp"] = oss.str();
        log_json["level"] = level_to_string(entry.level);
        log_json["logger"] = entry.logger_name;
        log_json["message"] = entry.message;
        if (!entry.file.empty()) {
            log_json["file"] = entry.file;
            log_json["line"] = entry.line;
            log_json["function"] = entry.function;
        }
        if (!entry.context.empty()) {
            log_json["context"] = entry.context;
        }

        result.push_back(log_json);
    }

    return result;
}

} // namespace obs

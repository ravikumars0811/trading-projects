#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace obs {

enum class MetricType {
    COUNTER,
    GAUGE,
    HISTOGRAM,
    TIMER
};

struct Metric {
    std::string name;
    MetricType type;
    double value;
    std::unordered_map<std::string, std::string> labels;
    std::chrono::system_clock::time_point timestamp;
};

class MetricsCollector {
public:
    static MetricsCollector& instance();

    // Metric operations
    void increment(const std::string& name, double value = 1.0,
                  const std::unordered_map<std::string, std::string>& labels = {});

    void set_gauge(const std::string& name, double value,
                  const std::unordered_map<std::string, std::string>& labels = {});

    void record_histogram(const std::string& name, double value,
                         const std::unordered_map<std::string, std::string>& labels = {});

    void start_timer(const std::string& name);
    void stop_timer(const std::string& name,
                   const std::unordered_map<std::string, std::string>& labels = {});

    // Query operations
    std::vector<Metric> get_all_metrics() const;
    std::vector<Metric> get_metrics_by_name(const std::string& name) const;
    nlohmann::json export_prometheus_format() const;
    nlohmann::json export_json() const;

    // Statistics
    struct Statistics {
        size_t total_metrics;
        size_t unique_metric_names;
        double collection_rate_per_sec;
    };

    Statistics get_statistics() const;
    void reset();

private:
    MetricsCollector() = default;
    ~MetricsCollector() = default;
    MetricsCollector(const MetricsCollector&) = delete;
    MetricsCollector& operator=(const MetricsCollector&) = delete;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<Metric>> metrics_;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> timers_;
    std::atomic<size_t> total_collected_{0};
    std::chrono::steady_clock::time_point start_time_{std::chrono::steady_clock::now()};
};

// RAII timer helper
class ScopedTimer {
public:
    explicit ScopedTimer(const std::string& name,
                        const std::unordered_map<std::string, std::string>& labels = {})
        : name_(name), labels_(labels) {
        MetricsCollector::instance().start_timer(name_);
    }

    ~ScopedTimer() {
        MetricsCollector::instance().stop_timer(name_, labels_);
    }

private:
    std::string name_;
    std::unordered_map<std::string, std::string> labels_;
};

} // namespace obs

#include "obs/metrics_collector.hpp"
#include <random>
#include <sstream>
#include <iomanip>

namespace obs {

MetricsCollector& MetricsCollector::instance() {
    static MetricsCollector instance;
    return instance;
}

void MetricsCollector::increment(const std::string& name, double value,
                                const std::unordered_map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(mutex_);

    Metric metric;
    metric.name = name;
    metric.type = MetricType::COUNTER;
    metric.value = value;
    metric.labels = labels;
    metric.timestamp = std::chrono::system_clock::now();

    metrics_[name].push_back(metric);
    total_collected_++;
}

void MetricsCollector::set_gauge(const std::string& name, double value,
                                const std::unordered_map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(mutex_);

    Metric metric;
    metric.name = name;
    metric.type = MetricType::GAUGE;
    metric.value = value;
    metric.labels = labels;
    metric.timestamp = std::chrono::system_clock::now();

    metrics_[name].push_back(metric);
    total_collected_++;
}

void MetricsCollector::record_histogram(const std::string& name, double value,
                                       const std::unordered_map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(mutex_);

    Metric metric;
    metric.name = name;
    metric.type = MetricType::HISTOGRAM;
    metric.value = value;
    metric.labels = labels;
    metric.timestamp = std::chrono::system_clock::now();

    metrics_[name].push_back(metric);
    total_collected_++;
}

void MetricsCollector::start_timer(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    timers_[name] = std::chrono::steady_clock::now();
}

void MetricsCollector::stop_timer(const std::string& name,
                                 const std::unordered_map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = timers_.find(name);
    if (it != timers_.end()) {
        auto duration = std::chrono::steady_clock::now() - it->second;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        Metric metric;
        metric.name = name;
        metric.type = MetricType::TIMER;
        metric.value = static_cast<double>(ms);
        metric.labels = labels;
        metric.timestamp = std::chrono::system_clock::now();

        metrics_[name].push_back(metric);
        total_collected_++;
        timers_.erase(it);
    }
}

std::vector<Metric> MetricsCollector::get_all_metrics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Metric> all_metrics;

    for (const auto& [name, metrics] : metrics_) {
        all_metrics.insert(all_metrics.end(), metrics.begin(), metrics.end());
    }

    return all_metrics;
}

std::vector<Metric> MetricsCollector::get_metrics_by_name(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = metrics_.find(name);
    if (it != metrics_.end()) {
        return it->second;
    }
    return {};
}

nlohmann::json MetricsCollector::export_json() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result = nlohmann::json::array();

    for (const auto& [name, metrics] : metrics_) {
        for (const auto& metric : metrics) {
            nlohmann::json m;
            m["name"] = metric.name;
            m["type"] = static_cast<int>(metric.type);
            m["value"] = metric.value;
            m["labels"] = metric.labels;

            auto time_t = std::chrono::system_clock::to_time_t(metric.timestamp);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            m["timestamp"] = oss.str();

            result.push_back(m);
        }
    }

    return result;
}

nlohmann::json MetricsCollector::export_prometheus_format() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result;

    for (const auto& [name, metrics] : metrics_) {
        if (!metrics.empty()) {
            const auto& latest = metrics.back();
            std::string prom_name = name;
            std::replace(prom_name.begin(), prom_name.end(), '.', '_');

            std::ostringstream labels_str;
            for (const auto& [key, value] : latest.labels) {
                if (labels_str.tellp() > 0) labels_str << ",";
                labels_str << key << "=\"" << value << "\"";
            }

            result[prom_name] = {
                {"value", latest.value},
                {"labels", labels_str.str()}
            };
        }
    }

    return result;
}

MetricsCollector::Statistics MetricsCollector::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);

    Statistics stats;
    stats.total_metrics = total_collected_.load();
    stats.unique_metric_names = metrics_.size();

    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count();
    stats.collection_rate_per_sec = duration > 0 ? static_cast<double>(stats.total_metrics) / duration : 0.0;

    return stats;
}

void MetricsCollector::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    metrics_.clear();
    timers_.clear();
    total_collected_ = 0;
    start_time_ = std::chrono::steady_clock::now();
}

} // namespace obs

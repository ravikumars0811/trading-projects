#include "obs/metrics_collector.hpp"
#include "obs/logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    auto& metrics = obs::MetricsCollector::instance();
    auto& logger = obs::Logger::instance();

    logger.set_level(obs::LogLevel::INFO);
    LOG_INFO("Metrics Collection Example");
    LOG_INFO("===========================\n");

    // Example 1: Counter metrics
    LOG_INFO("1. Counter Metrics:");
    for (int i = 0; i < 10; ++i) {
        metrics.increment("http_requests_total", 1.0, {
            {"endpoint", "/api/users"},
            {"method", "GET"},
            {"status", "200"}
        });
    }
    LOG_INFO("   Incremented http_requests_total 10 times");

    // Example 2: Gauge metrics
    LOG_INFO("\n2. Gauge Metrics:");
    metrics.set_gauge("memory_usage_bytes", 1024 * 1024 * 512); // 512 MB
    metrics.set_gauge("active_connections", 42);
    metrics.set_gauge("queue_size", 156);
    LOG_INFO("   Set various gauge metrics");

    // Example 3: Histogram metrics
    LOG_INFO("\n3. Histogram Metrics:");
    std::vector<double> response_times = {12.5, 23.1, 45.6, 78.9, 15.3, 67.2, 34.8, 91.2};
    for (double time : response_times) {
        metrics.record_histogram("http_request_duration_ms", time);
    }
    LOG_INFO("   Recorded " + std::to_string(response_times.size()) + " response time samples");

    // Example 4: Timer metrics (manual)
    LOG_INFO("\n4. Timer Metrics (Manual):");
    metrics.start_timer("database_query");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    metrics.stop_timer("database_query", {{"query_type", "SELECT"}});
    LOG_INFO("   Timed a database query");

    // Example 5: RAII timer
    LOG_INFO("\n5. Timer Metrics (RAII):");
    {
        obs::ScopedTimer timer("api_call", {{"service", "auth"}});
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        LOG_INFO("   API call completed (timer auto-stopped)");
    }

    // Export and display metrics
    LOG_INFO("\n6. Export Metrics:");
    auto json_export = metrics.export_json();
    LOG_INFO("   Total metrics: " + std::to_string(json_export.size()));

    // Display statistics
    auto stats = metrics.get_statistics();
    LOG_INFO("\nStatistics:");
    LOG_INFO("  Total metrics collected: " + std::to_string(stats.total_metrics));
    LOG_INFO("  Unique metric names: " + std::to_string(stats.unique_metric_names));
    LOG_INFO("  Collection rate: " + std::to_string(stats.collection_rate_per_sec) + " metrics/sec");

    // Display sample metrics
    LOG_INFO("\nSample Metrics (JSON format):");
    std::cout << json_export.dump(2) << std::endl;

    return 0;
}

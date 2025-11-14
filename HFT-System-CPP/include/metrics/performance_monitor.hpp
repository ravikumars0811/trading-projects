#pragma once

#include <string>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <cstdint>

namespace hft {
namespace metrics {

/**
 * Performance metrics for monitoring system health
 */
struct SystemMetrics {
    uint64_t total_orders = 0;
    uint64_t total_fills = 0;
    uint64_t total_cancels = 0;
    uint64_t total_rejects = 0;

    uint64_t market_data_messages = 0;
    uint64_t trades_executed = 0;

    double avg_order_latency_us = 0.0;
    double avg_market_data_latency_us = 0.0;

    uint64_t timestamp = 0;
};

/**
 * Performance Monitor
 * Collects and tracks system performance metrics
 */
class PerformanceMonitor {
public:
    static PerformanceMonitor& instance() {
        static PerformanceMonitor instance;
        return instance;
    }

    // Metric updates
    void recordOrder();
    void recordFill();
    void recordCancel();
    void recordReject();

    void recordMarketDataMessage();
    void recordTrade();

    void recordOrderLatency(int64_t latency_ns);
    void recordMarketDataLatency(int64_t latency_ns);

    // Metric queries
    SystemMetrics getMetrics() const;
    void printMetrics() const;

    // Reset metrics
    void reset();

private:
    PerformanceMonitor() = default;

    std::atomic<uint64_t> total_orders_{0};
    std::atomic<uint64_t> total_fills_{0};
    std::atomic<uint64_t> total_cancels_{0};
    std::atomic<uint64_t> total_rejects_{0};

    std::atomic<uint64_t> market_data_messages_{0};
    std::atomic<uint64_t> trades_executed_{0};

    std::atomic<uint64_t> avg_order_latency_ns_{0};
    std::atomic<uint64_t> avg_md_latency_ns_{0};
};

} // namespace metrics
} // namespace hft

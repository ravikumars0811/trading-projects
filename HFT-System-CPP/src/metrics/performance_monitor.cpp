#include "metrics/performance_monitor.hpp"
#include "core/timer.hpp"
#include <iostream>
#include <iomanip>

namespace hft {
namespace metrics {

void PerformanceMonitor::recordOrder() {
    total_orders_++;
}

void PerformanceMonitor::recordFill() {
    total_fills_++;
}

void PerformanceMonitor::recordCancel() {
    total_cancels_++;
}

void PerformanceMonitor::recordReject() {
    total_rejects_++;
}

void PerformanceMonitor::recordMarketDataMessage() {
    market_data_messages_++;
}

void PerformanceMonitor::recordTrade() {
    trades_executed_++;
}

void PerformanceMonitor::recordOrderLatency(int64_t latency_ns) {
    // Exponential moving average
    uint64_t current_avg = avg_order_latency_ns_.load();
    uint64_t new_avg = (current_avg * 95 + latency_ns * 5) / 100;
    avg_order_latency_ns_.store(new_avg);
}

void PerformanceMonitor::recordMarketDataLatency(int64_t latency_ns) {
    // Exponential moving average
    uint64_t current_avg = avg_md_latency_ns_.load();
    uint64_t new_avg = (current_avg * 95 + latency_ns * 5) / 100;
    avg_md_latency_ns_.store(new_avg);
}

SystemMetrics PerformanceMonitor::getMetrics() const {
    SystemMetrics metrics;

    metrics.total_orders = total_orders_;
    metrics.total_fills = total_fills_;
    metrics.total_cancels = total_cancels_;
    metrics.total_rejects = total_rejects_;

    metrics.market_data_messages = market_data_messages_;
    metrics.trades_executed = trades_executed_;

    metrics.avg_order_latency_us = avg_order_latency_ns_.load() / 1000.0;
    metrics.avg_market_data_latency_us = avg_md_latency_ns_.load() / 1000.0;

    metrics.timestamp = core::Timer::timestamp_ns();

    return metrics;
}

void PerformanceMonitor::printMetrics() const {
    auto metrics = getMetrics();

    std::cout << "\n=== HFT System Performance Metrics ===\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Orders:              " << metrics.total_orders << "\n";
    std::cout << "Fills:               " << metrics.total_fills << "\n";
    std::cout << "Cancels:             " << metrics.total_cancels << "\n";
    std::cout << "Rejects:             " << metrics.total_rejects << "\n";
    std::cout << "Market Data Msgs:    " << metrics.market_data_messages << "\n";
    std::cout << "Trades:              " << metrics.trades_executed << "\n";
    std::cout << "Avg Order Latency:   " << metrics.avg_order_latency_us << " us\n";
    std::cout << "Avg MD Latency:      " << metrics.avg_market_data_latency_us << " us\n";
    std::cout << "======================================\n\n";
}

void PerformanceMonitor::reset() {
    total_orders_ = 0;
    total_fills_ = 0;
    total_cancels_ = 0;
    total_rejects_ = 0;
    market_data_messages_ = 0;
    trades_executed_ = 0;
    avg_order_latency_ns_ = 0;
    avg_md_latency_ns_ = 0;
}

} // namespace metrics
} // namespace hft

#pragma once

#include "oms/order_manager.hpp"
#include "core/lock_free_queue.hpp"
#include <string>
#include <thread>
#include <atomic>
#include <unordered_map>

namespace hft {
namespace gateway {

/**
 * Exchange Gateway
 * Handles communication with exchanges
 * In production, this would implement FIX protocol
 */
class ExchangeGateway {
public:
    explicit ExchangeGateway(oms::OrderManager& order_manager);
    ~ExchangeGateway();

    // Lifecycle
    void connect(const std::string& host, int port);
    void disconnect();
    bool isConnected() const { return connected_; }

    // Order routing
    bool sendOrder(const oms::OrderRequest& request);
    bool cancelOrder(uint64_t order_id);
    bool modifyOrder(uint64_t order_id, uint32_t new_quantity, uint64_t new_price);

    // Statistics
    uint64_t getOrdersSent() const { return orders_sent_; }
    uint64_t getCancelsSent() const { return cancels_sent_; }
    uint64_t getAvgLatency() const { return avg_latency_ns_; }

private:
    void processingThread();
    void simulateExecution(const oms::OrderRequest& request);

    oms::OrderManager& order_manager_;

    std::atomic<bool> connected_{false};
    std::atomic<bool> running_{false};
    std::thread processing_thread_;

    core::LockFreeQueue<oms::OrderRequest, 32768> order_queue_;

    std::atomic<uint64_t> orders_sent_{0};
    std::atomic<uint64_t> cancels_sent_{0};
    std::atomic<uint64_t> avg_latency_ns_{0};

    std::string host_;
    int port_;
};

} // namespace gateway
} // namespace hft

#include "gateway/exchange_gateway.hpp"
#include "core/timer.hpp"
#include "core/logger.hpp"
#include <random>

namespace hft {
namespace gateway {

ExchangeGateway::ExchangeGateway(oms::OrderManager& order_manager)
    : order_manager_(order_manager) {}

ExchangeGateway::~ExchangeGateway() {
    disconnect();
}

void ExchangeGateway::connect(const std::string& host, int port) {
    if (connected_.exchange(true)) {
        return;  // Already connected
    }

    host_ = host;
    port_ = port;

    running_ = true;
    processing_thread_ = std::thread(&ExchangeGateway::processingThread, this);

    LOG_INFO("Exchange gateway connected to ", host, ":", port);
}

void ExchangeGateway::disconnect() {
    if (!connected_.exchange(false)) {
        return;  // Already disconnected
    }

    running_ = false;
    if (processing_thread_.joinable()) {
        processing_thread_.join();
    }

    LOG_INFO("Exchange gateway disconnected");
}

bool ExchangeGateway::sendOrder(const oms::OrderRequest& request) {
    if (!connected_) {
        LOG_ERROR("Cannot send order: gateway not connected");
        return false;
    }

    bool success = order_queue_.push(request);
    if (success) {
        orders_sent_++;
    }

    return success;
}

bool ExchangeGateway::cancelOrder(uint64_t order_id) {
    if (!connected_) {
        return false;
    }

    cancels_sent_++;
    order_manager_.updateOrderStatus(order_id, oms::OrderStatus::CANCELLED);

    return true;
}

bool ExchangeGateway::modifyOrder(uint64_t order_id, uint32_t new_quantity, uint64_t new_price) {
    if (!connected_) {
        return false;
    }

    return order_manager_.modifyOrder(order_id, new_quantity, new_price);
}

void ExchangeGateway::processingThread() {
    while (running_ || !order_queue_.empty()) {
        auto order_opt = order_queue_.pop();

        if (order_opt.has_value()) {
            auto& request = order_opt.value();

            // Simulate network latency
            core::Timer timer;

            // In production, this would send via FIX protocol
            simulateExecution(request);

            int64_t latency = timer.elapsed_ns();

            // Update average latency
            uint64_t current_avg = avg_latency_ns_.load();
            uint64_t new_avg = (current_avg * 95 + latency * 5) / 100;
            avg_latency_ns_.store(new_avg);
        } else {
            // Queue is empty, sleep briefly
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
}

void ExchangeGateway::simulateExecution(const oms::OrderRequest& request) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> execution_dist(0, 100);

    // Simulate network latency (50-200 microseconds)
    std::uniform_int_distribution<> latency_dist(50, 200);
    std::this_thread::sleep_for(std::chrono::microseconds(latency_dist(gen)));

    // Update order status to acknowledged
    order_manager_.updateOrderStatus(request.client_order_id,
                                     oms::OrderStatus::ACKNOWLEDGED);

    // Simulate execution (70% fill rate)
    if (execution_dist(gen) < 70) {
        // Simulate partial or full fill
        std::uniform_int_distribution<> fill_ratio_dist(50, 100);
        uint32_t fill_qty = (request.quantity * fill_ratio_dist(gen)) / 100;

        oms::Fill fill{
            request.client_order_id,
            request.client_order_id,  // exec_id
            request.price,
            fill_qty,
            core::Timer::timestamp_ns()
        };

        order_manager_.addFill(request.client_order_id, fill);
    }
}

} // namespace gateway
} // namespace hft

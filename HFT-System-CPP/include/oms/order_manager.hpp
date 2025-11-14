#pragma once

#include "market_data/order_book.hpp"
#include <unordered_map>
#include <functional>
#include <atomic>

namespace hft {
namespace oms {

enum class OrderStatus : uint8_t {
    PENDING,
    SENT,
    ACKNOWLEDGED,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED,
    REJECTED
};

enum class OrderType : uint8_t {
    LIMIT,
    MARKET,
    IOC,  // Immediate or Cancel
    FOK   // Fill or Kill
};

struct OrderRequest {
    uint64_t client_order_id;
    std::string symbol;
    market_data::Side side;
    OrderType type;
    uint64_t price;
    uint32_t quantity;
    uint64_t timestamp;
};

struct OrderState {
    OrderRequest request;
    OrderStatus status;
    uint32_t filled_quantity;
    uint64_t average_fill_price;
    uint64_t last_update_time;
};

struct Fill {
    uint64_t order_id;
    uint64_t exec_id;
    uint64_t price;
    uint32_t quantity;
    uint64_t timestamp;
};

/**
 * Order Management System
 * Manages order lifecycle, routing, and execution tracking
 */
class OrderManager {
public:
    using OrderUpdateCallback = std::function<void(const OrderState&)>;
    using FillCallback = std::function<void(const Fill&)>;

    OrderManager();
    ~OrderManager() = default;

    // Order submission
    uint64_t submitOrder(const OrderRequest& request);
    bool cancelOrder(uint64_t client_order_id);
    bool modifyOrder(uint64_t client_order_id, uint32_t new_quantity, uint64_t new_price);

    // Order state queries
    const OrderState* getOrderState(uint64_t client_order_id) const;
    std::vector<OrderState> getActiveOrders() const;
    std::vector<OrderState> getOrdersBySymbol(const std::string& symbol) const;

    // Callbacks
    void setOrderUpdateCallback(OrderUpdateCallback cb) { order_update_callback_ = std::move(cb); }
    void setFillCallback(FillCallback cb) { fill_callback_ = std::move(cb); }

    // Update order state (called by exchange gateway)
    void updateOrderStatus(uint64_t client_order_id, OrderStatus status);
    void addFill(uint64_t client_order_id, const Fill& fill);

    // Statistics
    uint64_t getTotalOrdersSubmitted() const { return total_orders_submitted_; }
    uint64_t getTotalFills() const { return total_fills_; }

private:
    std::unordered_map<uint64_t, OrderState> orders_;
    std::atomic<uint64_t> next_order_id_{1};

    OrderUpdateCallback order_update_callback_;
    FillCallback fill_callback_;

    std::atomic<uint64_t> total_orders_submitted_{0};
    std::atomic<uint64_t> total_fills_{0};
};

} // namespace oms
} // namespace hft

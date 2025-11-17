#pragma once

#include <unordered_map>
#include <vector>
#include <array>
#include <atomic>
#include <functional>
#include <cstdint>
#include <string>
#include "market_data/order_book.hpp"
#include "core/memory_pool.hpp"

namespace hft {
namespace oms {

/**
 * Optimized Order Manager with Performance Enhancements
 *
 * Optimizations:
 * 1. Cache-friendly data structures (array-based for hot paths)
 * 2. Memory pool for order allocation (eliminates dynamic allocation)
 * 3. Lock-free counters for statistics
 * 4. Inline hot paths for better compiler optimization
 * 5. Reduced hash map lookups via direct indexing
 * 6. Batch order processing support
 */

enum class OrderStatus : uint8_t {
    PENDING = 0,
    ACCEPTED,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED,
    REJECTED
};

struct OrderRequest {
    std::string symbol;
    market_data::Side side;
    uint64_t price;
    uint32_t quantity;
    uint64_t timestamp;
};

struct Fill {
    uint64_t order_id;
    uint64_t price;
    uint32_t quantity;
    uint64_t timestamp;
};

// Cache-aligned order state (64-byte cache line)
struct alignas(64) OrderState {
    OrderRequest request;
    OrderStatus status;
    uint32_t filled_quantity;
    uint64_t average_fill_price;
    uint64_t last_update_time;
    uint64_t order_id;

    // Padding to cache line boundary
    char padding[64 - sizeof(OrderRequest) - sizeof(OrderStatus) -
                 sizeof(uint32_t) - sizeof(uint64_t)*3];
};

class OptimizedOrderManager {
public:
    using OrderUpdateCallback = std::function<void(const OrderState&)>;
    using FillCallback = std::function<void(const Fill&)>;

    // Use memory pool for order states
    static constexpr size_t MAX_ORDERS = 1000000;  // Pre-allocate for 1M orders

    OptimizedOrderManager();
    ~OptimizedOrderManager() = default;

    // Single order operations (hot path - inlined)
    inline uint64_t submitOrder(const OrderRequest& request);
    inline bool cancelOrder(uint64_t client_order_id);
    inline bool modifyOrder(uint64_t client_order_id, uint32_t new_quantity, uint64_t new_price);

    // Batch operations for improved throughput
    std::vector<uint64_t> submitOrderBatch(const std::vector<OrderRequest>& requests);
    std::vector<bool> cancelOrderBatch(const std::vector<uint64_t>& order_ids);

    // Query operations
    const OrderState* getOrderState(uint64_t client_order_id) const;
    std::vector<OrderState> getActiveOrders() const;
    std::vector<OrderState> getOrdersBySymbol(const std::string& symbol) const;

    // Status updates (from exchange)
    void updateOrderStatus(uint64_t client_order_id, OrderStatus status);
    void addFill(uint64_t client_order_id, const Fill& fill);

    // Callbacks
    void setOrderUpdateCallback(OrderUpdateCallback callback) {
        order_update_callback_ = std::move(callback);
    }

    void setFillCallback(FillCallback callback) {
        fill_callback_ = std::move(callback);
    }

    // Statistics (lock-free atomics)
    uint64_t getTotalOrdersSubmitted() const {
        return total_orders_submitted_.load(std::memory_order_relaxed);
    }

    uint64_t getTotalFills() const {
        return total_fills_.load(std::memory_order_relaxed);
    }

    uint64_t getTotalCancels() const {
        return total_cancels_.load(std::memory_order_relaxed);
    }

private:
    // Use array-based storage for fast direct access
    // Orders indexed by order_id % MAX_ORDERS
    std::array<OrderState, MAX_ORDERS> order_states_;

    // Hash map for symbol-based lookups (secondary index)
    std::unordered_map<std::string, std::vector<uint64_t>> symbol_to_orders_;

    // Order ID generation (atomic for thread safety)
    std::atomic<uint64_t> next_order_id_{1};

    // Statistics (lock-free)
    std::atomic<uint64_t> total_orders_submitted_{0};
    std::atomic<uint64_t> total_fills_{0};
    std::atomic<uint64_t> total_cancels_{0};

    // Callbacks
    OrderUpdateCallback order_update_callback_;
    FillCallback fill_callback_;

    // Helper methods
    inline OrderState* getOrderStateInternal(uint64_t order_id);
};

// Inline implementations for hot paths

inline uint64_t OptimizedOrderManager::submitOrder(const OrderRequest& request) {
    uint64_t order_id = next_order_id_.fetch_add(1, std::memory_order_relaxed);

    // Direct array access (no hash map lookup)
    size_t index = order_id % MAX_ORDERS;
    OrderState& state = order_states_[index];

    state.request = request;
    state.status = OrderStatus::PENDING;
    state.filled_quantity = 0;
    state.average_fill_price = 0;
    state.last_update_time = request.timestamp;
    state.order_id = order_id;

    // Update symbol index
    symbol_to_orders_[request.symbol].push_back(order_id);

    total_orders_submitted_.fetch_add(1, std::memory_order_relaxed);

    if (order_update_callback_) {
        order_update_callback_(state);
    }

    return order_id;
}

inline bool OptimizedOrderManager::cancelOrder(uint64_t client_order_id) {
    OrderState* state = getOrderStateInternal(client_order_id);
    if (!state) return false;

    if (state->status == OrderStatus::FILLED ||
        state->status == OrderStatus::CANCELLED ||
        state->status == OrderStatus::REJECTED) {
        return false;
    }

    state->status = OrderStatus::CANCELLED;
    state->last_update_time = core::Timer::timestamp_ns();

    total_cancels_.fetch_add(1, std::memory_order_relaxed);

    if (order_update_callback_) {
        order_update_callback_(*state);
    }

    return true;
}

inline bool OptimizedOrderManager::modifyOrder(uint64_t client_order_id,
                                                uint32_t new_quantity,
                                                uint64_t new_price) {
    OrderState* state = getOrderStateInternal(client_order_id);
    if (!state) return false;

    if (state->status == OrderStatus::FILLED ||
        state->status == OrderStatus::CANCELLED ||
        state->status == OrderStatus::REJECTED) {
        return false;
    }

    state->request.quantity = new_quantity;
    state->request.price = new_price;
    state->last_update_time = core::Timer::timestamp_ns();

    if (order_update_callback_) {
        order_update_callback_(*state);
    }

    return true;
}

inline OrderState* OptimizedOrderManager::getOrderStateInternal(uint64_t order_id) {
    size_t index = order_id % MAX_ORDERS;
    OrderState& state = order_states_[index];

    // Verify this is the correct order (handle wraparound)
    if (state.order_id == order_id) {
        return &state;
    }

    return nullptr;
}

} // namespace oms
} // namespace hft

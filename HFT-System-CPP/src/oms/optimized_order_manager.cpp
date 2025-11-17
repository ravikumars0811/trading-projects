#include "oms/optimized_order_manager.hpp"
#include "core/timer.hpp"
#include "core/logger.hpp"
#include <algorithm>

namespace hft {
namespace oms {

OptimizedOrderManager::OptimizedOrderManager() {
    // Initialize order states
    for (auto& state : order_states_) {
        state.status = OrderStatus::REJECTED;  // Mark as invalid initially
        state.order_id = 0;
    }

    // Reserve space for symbol index
    symbol_to_orders_.reserve(1000);  // Assume max 1000 symbols
}

std::vector<uint64_t> OptimizedOrderManager::submitOrderBatch(
    const std::vector<OrderRequest>& requests) {

    std::vector<uint64_t> order_ids;
    order_ids.reserve(requests.size());

    // Batch process all orders
    for (const auto& request : requests) {
        uint64_t order_id = submitOrder(request);
        order_ids.push_back(order_id);
    }

    LOG_INFO("Batch submitted: ", requests.size(), " orders");

    return order_ids;
}

std::vector<bool> OptimizedOrderManager::cancelOrderBatch(
    const std::vector<uint64_t>& order_ids) {

    std::vector<bool> results;
    results.reserve(order_ids.size());

    for (uint64_t order_id : order_ids) {
        bool success = cancelOrder(order_id);
        results.push_back(success);
    }

    LOG_INFO("Batch cancelled: ", order_ids.size(), " orders");

    return results;
}

const OrderState* OptimizedOrderManager::getOrderState(uint64_t client_order_id) const {
    size_t index = client_order_id % MAX_ORDERS;
    const OrderState& state = order_states_[index];

    if (state.order_id == client_order_id) {
        return &state;
    }

    return nullptr;
}

std::vector<OrderState> OptimizedOrderManager::getActiveOrders() const {
    std::vector<OrderState> active_orders;
    active_orders.reserve(10000);  // Pre-allocate

    for (const auto& state : order_states_) {
        if (state.order_id > 0 &&
            state.status != OrderStatus::FILLED &&
            state.status != OrderStatus::CANCELLED &&
            state.status != OrderStatus::REJECTED) {
            active_orders.push_back(state);
        }
    }

    return active_orders;
}

std::vector<OrderState> OptimizedOrderManager::getOrdersBySymbol(
    const std::string& symbol) const {

    std::vector<OrderState> symbol_orders;

    auto it = symbol_to_orders_.find(symbol);
    if (it != symbol_to_orders_.end()) {
        symbol_orders.reserve(it->second.size());

        for (uint64_t order_id : it->second) {
            const OrderState* state = getOrderState(order_id);
            if (state && state->request.symbol == symbol) {
                symbol_orders.push_back(*state);
            }
        }
    }

    return symbol_orders;
}

void OptimizedOrderManager::updateOrderStatus(uint64_t client_order_id,
                                               OrderStatus status) {
    OrderState* state = getOrderStateInternal(client_order_id);
    if (!state) return;

    state->status = status;
    state->last_update_time = core::Timer::timestamp_ns();

    LOG_INFO("Order status updated: ID=", client_order_id,
             " Status=", static_cast<int>(status));

    if (order_update_callback_) {
        order_update_callback_(*state);
    }
}

void OptimizedOrderManager::addFill(uint64_t client_order_id, const Fill& fill) {
    OrderState* state = getOrderStateInternal(client_order_id);
    if (!state) return;

    // Update average fill price using weighted average
    uint64_t total_filled_value = state->average_fill_price * state->filled_quantity +
                                  fill.price * fill.quantity;
    state->filled_quantity += fill.quantity;

    if (state->filled_quantity > 0) {
        state->average_fill_price = total_filled_value / state->filled_quantity;
    }

    // Update status
    if (state->filled_quantity >= state->request.quantity) {
        state->status = OrderStatus::FILLED;
    } else {
        state->status = OrderStatus::PARTIALLY_FILLED;
    }

    state->last_update_time = core::Timer::timestamp_ns();

    total_fills_.fetch_add(1, std::memory_order_relaxed);

    LOG_INFO("Order filled: ID=", client_order_id,
             " Price=", fill.price, " Qty=", fill.quantity,
             " TotalFilled=", state->filled_quantity);

    if (fill_callback_) {
        fill_callback_(fill);
    }

    if (order_update_callback_) {
        order_update_callback_(*state);
    }
}

} // namespace oms
} // namespace hft

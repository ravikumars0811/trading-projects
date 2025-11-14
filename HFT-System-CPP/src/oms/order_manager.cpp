#include "oms/order_manager.hpp"
#include "core/timer.hpp"
#include "core/logger.hpp"

namespace hft {
namespace oms {

OrderManager::OrderManager() = default;

uint64_t OrderManager::submitOrder(const OrderRequest& request) {
    uint64_t order_id = next_order_id_++;

    OrderState state{
        request,
        OrderStatus::PENDING,
        0,  // filled_quantity
        0,  // average_fill_price
        core::Timer::timestamp_ns()
    };

    orders_[order_id] = state;
    total_orders_submitted_++;

    LOG_INFO("Order submitted: ID=", order_id, " Symbol=", request.symbol,
             " Side=", (request.side == market_data::Side::BUY ? "BUY" : "SELL"),
             " Price=", request.price, " Qty=", request.quantity);

    if (order_update_callback_) {
        order_update_callback_(state);
    }

    return order_id;
}

bool OrderManager::cancelOrder(uint64_t client_order_id) {
    auto it = orders_.find(client_order_id);
    if (it == orders_.end()) {
        LOG_WARNING("Cancel failed: Order not found, ID=", client_order_id);
        return false;
    }

    OrderState& state = it->second;

    if (state.status == OrderStatus::FILLED ||
        state.status == OrderStatus::CANCELLED ||
        state.status == OrderStatus::REJECTED) {
        LOG_WARNING("Cancel failed: Order in terminal state, ID=", client_order_id);
        return false;
    }

    state.status = OrderStatus::CANCELLED;
    state.last_update_time = core::Timer::timestamp_ns();

    LOG_INFO("Order cancelled: ID=", client_order_id);

    if (order_update_callback_) {
        order_update_callback_(state);
    }

    return true;
}

bool OrderManager::modifyOrder(uint64_t client_order_id, uint32_t new_quantity, uint64_t new_price) {
    auto it = orders_.find(client_order_id);
    if (it == orders_.end()) {
        return false;
    }

    OrderState& state = it->second;

    if (state.status == OrderStatus::FILLED ||
        state.status == OrderStatus::CANCELLED ||
        state.status == OrderStatus::REJECTED) {
        return false;
    }

    state.request.quantity = new_quantity;
    state.request.price = new_price;
    state.last_update_time = core::Timer::timestamp_ns();

    LOG_INFO("Order modified: ID=", client_order_id,
             " NewQty=", new_quantity, " NewPrice=", new_price);

    if (order_update_callback_) {
        order_update_callback_(state);
    }

    return true;
}

const OrderState* OrderManager::getOrderState(uint64_t client_order_id) const {
    auto it = orders_.find(client_order_id);
    return (it != orders_.end()) ? &it->second : nullptr;
}

std::vector<OrderState> OrderManager::getActiveOrders() const {
    std::vector<OrderState> active_orders;

    for (const auto& [id, state] : orders_) {
        if (state.status != OrderStatus::FILLED &&
            state.status != OrderStatus::CANCELLED &&
            state.status != OrderStatus::REJECTED) {
            active_orders.push_back(state);
        }
    }

    return active_orders;
}

std::vector<OrderState> OrderManager::getOrdersBySymbol(const std::string& symbol) const {
    std::vector<OrderState> symbol_orders;

    for (const auto& [id, state] : orders_) {
        if (state.request.symbol == symbol) {
            symbol_orders.push_back(state);
        }
    }

    return symbol_orders;
}

void OrderManager::updateOrderStatus(uint64_t client_order_id, OrderStatus status) {
    auto it = orders_.find(client_order_id);
    if (it == orders_.end()) {
        return;
    }

    OrderState& state = it->second;
    state.status = status;
    state.last_update_time = core::Timer::timestamp_ns();

    LOG_INFO("Order status updated: ID=", client_order_id, " Status=", static_cast<int>(status));

    if (order_update_callback_) {
        order_update_callback_(state);
    }
}

void OrderManager::addFill(uint64_t client_order_id, const Fill& fill) {
    auto it = orders_.find(client_order_id);
    if (it == orders_.end()) {
        return;
    }

    OrderState& state = it->second;

    // Update average fill price
    uint64_t total_filled_value = state.average_fill_price * state.filled_quantity +
                                  fill.price * fill.quantity;
    state.filled_quantity += fill.quantity;
    state.average_fill_price = total_filled_value / state.filled_quantity;

    // Update status
    if (state.filled_quantity >= state.request.quantity) {
        state.status = OrderStatus::FILLED;
    } else {
        state.status = OrderStatus::PARTIALLY_FILLED;
    }

    state.last_update_time = core::Timer::timestamp_ns();

    total_fills_++;

    LOG_INFO("Order filled: ID=", client_order_id,
             " Price=", fill.price, " Qty=", fill.quantity,
             " TotalFilled=", state.filled_quantity);

    if (fill_callback_) {
        fill_callback_(fill);
    }

    if (order_update_callback_) {
        order_update_callback_(state);
    }
}

} // namespace oms
} // namespace hft

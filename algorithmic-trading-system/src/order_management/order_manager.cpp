#include "../../include/order_manager.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>

namespace trading {

// Portfolio implementation
Portfolio::Portfolio(double initial_capital)
    : initial_capital_(initial_capital), cash_(initial_capital) {}

void Portfolio::update_position(const Trade& trade) {
    std::unique_lock lock(mutex_);

    auto& position = positions_[trade.symbol];
    if (position.symbol.empty()) {
        position.symbol = trade.symbol;
    }

    position.update_with_trade(trade);

    // Update cash
    if (trade.side == OrderSide::BUY) {
        cash_ -= trade.price * trade.quantity;
    } else {
        cash_ += trade.price * trade.quantity;
    }

    cash_ -= trade.commission;
    total_commission_ += trade.commission;
}

Position Portfolio::get_position(const Symbol& symbol) const {
    std::shared_lock lock(mutex_);
    auto it = positions_.find(symbol);
    return it != positions_.end() ? it->second : Position(symbol, 0, 0);
}

std::vector<Position> Portfolio::get_all_positions() const {
    std::shared_lock lock(mutex_);
    std::vector<Position> result;
    result.reserve(positions_.size());
    for (const auto& [symbol, pos] : positions_) {
        if (std::abs(pos.quantity) > 1e-6) {
            result.push_back(pos);
        }
    }
    return result;
}

double Portfolio::get_equity(const std::unordered_map<Symbol, Price>& current_prices) const {
    std::shared_lock lock(mutex_);
    double equity = cash_;

    for (const auto& [symbol, pos] : positions_) {
        auto it = current_prices.find(symbol);
        if (it != current_prices.end()) {
            equity += std::abs(pos.quantity) * it->second;
        }
    }

    return equity;
}

double Portfolio::get_total_pnl(const std::unordered_map<Symbol, Price>& current_prices) const {
    return get_equity(current_prices) - initial_capital_;
}

double Portfolio::get_margin_used(const std::unordered_map<Symbol, Price>& current_prices) const {
    std::shared_lock lock(mutex_);
    double margin = 0.0;

    for (const auto& [symbol, pos] : positions_) {
        auto it = current_prices.find(symbol);
        if (it != current_prices.end()) {
            margin += std::abs(pos.quantity) * it->second;
        }
    }

    return margin;
}

void Portfolio::deduct_commission(double commission) {
    std::unique_lock lock(mutex_);
    cash_ -= commission;
    total_commission_ += commission;
}

// OrderManager implementation
OrderManager::OrderManager(std::shared_ptr<Portfolio> portfolio)
    : portfolio_(portfolio) {}

OrderManager::~OrderManager() = default;

OrderId OrderManager::submit_order(const Order& order) {
    auto new_order = std::make_shared<Order>(order);
    new_order->order_id = next_order_id_.fetch_add(1);
    new_order->created_at = now();
    new_order->updated_at = now();
    new_order->status = OrderStatus::SUBMITTED;

    {
        std::unique_lock lock(orders_mutex_);
        orders_[new_order->order_id] = new_order;
    }

    notify_order_update(*new_order);

    std::cout << "Order submitted: " << new_order->order_id << " "
              << new_order->symbol << " " << to_string(new_order->side) << " "
              << new_order->quantity << " @ " << new_order->price << "\n";

    return new_order->order_id;
}

bool OrderManager::cancel_order(OrderId order_id) {
    std::shared_ptr<Order> order;
    {
        std::shared_lock lock(orders_mutex_);
        auto it = orders_.find(order_id);
        if (it == orders_.end() || !it->second->is_active()) {
            return false;
        }
        order = it->second;
    }

    order->status = OrderStatus::CANCELLED;
    order->updated_at = now();
    notify_order_update(*order);

    std::cout << "Order cancelled: " << order_id << "\n";
    return true;
}

bool OrderManager::modify_order(OrderId order_id, Price new_price, Quantity new_quantity) {
    std::shared_ptr<Order> order;
    {
        std::unique_lock lock(orders_mutex_);
        auto it = orders_.find(order_id);
        if (it == orders_.end() || !it->second->is_active()) {
            return false;
        }
        order = it->second;

        order->price = new_price;
        order->quantity = new_quantity;
        order->updated_at = now();
    }

    notify_order_update(*order);
    return true;
}

std::shared_ptr<Order> OrderManager::get_order(OrderId order_id) const {
    std::shared_lock lock(orders_mutex_);
    auto it = orders_.find(order_id);
    return it != orders_.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<Order>> OrderManager::get_active_orders() const {
    std::shared_lock lock(orders_mutex_);
    std::vector<std::shared_ptr<Order>> result;
    for (const auto& [id, order] : orders_) {
        if (order->is_active()) {
            result.push_back(order);
        }
    }
    return result;
}

std::vector<std::shared_ptr<Order>> OrderManager::get_orders_by_symbol(const Symbol& symbol) const {
    std::shared_lock lock(orders_mutex_);
    std::vector<std::shared_ptr<Order>> result;
    for (const auto& [id, order] : orders_) {
        if (order->symbol == symbol) {
            result.push_back(order);
        }
    }
    return result;
}

std::vector<std::shared_ptr<Order>> OrderManager::get_orders_by_strategy(const std::string& strategy_id) const {
    std::shared_lock lock(orders_mutex_);
    std::vector<std::shared_ptr<Order>> result;
    for (const auto& [id, order] : orders_) {
        if (order->strategy_id == strategy_id) {
            result.push_back(order);
        }
    }
    return result;
}

void OrderManager::on_order_filled(OrderId order_id, const Trade& trade) {
    std::shared_ptr<Order> order;
    {
        std::unique_lock lock(orders_mutex_);
        auto it = orders_.find(order_id);
        if (it == orders_.end()) return;

        order = it->second;
        order->filled_quantity = order->quantity;
        order->status = OrderStatus::FILLED;
        order->updated_at = now();
    }

    portfolio_->update_position(trade);
    notify_order_update(*order);
    notify_trade(trade);

    std::cout << "Order filled: " << order_id << " " << trade.quantity
              << " @ " << trade.price << "\n";
}

void OrderManager::on_order_partially_filled(OrderId order_id, const Trade& trade) {
    std::shared_ptr<Order> order;
    {
        std::unique_lock lock(orders_mutex_);
        auto it = orders_.find(order_id);
        if (it == orders_.end()) return;

        order = it->second;
        order->filled_quantity += trade.quantity;
        order->status = OrderStatus::PARTIALLY_FILLED;
        order->updated_at = now();
    }

    portfolio_->update_position(trade);
    notify_order_update(*order);
    notify_trade(trade);
}

void OrderManager::on_order_cancelled(OrderId order_id) {
    std::shared_ptr<Order> order;
    {
        std::unique_lock lock(orders_mutex_);
        auto it = orders_.find(order_id);
        if (it == orders_.end()) return;

        order = it->second;
        order->status = OrderStatus::CANCELLED;
        order->updated_at = now();
    }

    notify_order_update(*order);
}

void OrderManager::on_order_rejected(OrderId order_id, const std::string& reason) {
    std::shared_ptr<Order> order;
    {
        std::unique_lock lock(orders_mutex_);
        auto it = orders_.find(order_id);
        if (it == orders_.end()) return;

        order = it->second;
        order->status = OrderStatus::REJECTED;
        order->updated_at = now();
    }

    notify_order_update(*order);
    notify_rejection(order_id, reason);

    std::cout << "Order rejected: " << order_id << " - " << reason << "\n";
}

void OrderManager::register_order_callback(const OrderCallback& callback) {
    std::lock_guard lock(callback_mutex_);
    order_callbacks_.push_back(callback);
}

void OrderManager::register_trade_callback(const TradeCallback& callback) {
    std::lock_guard lock(callback_mutex_);
    trade_callbacks_.push_back(callback);
}

void OrderManager::register_reject_callback(const RejectCallback& callback) {
    std::lock_guard lock(callback_mutex_);
    reject_callbacks_.push_back(callback);
}

size_t OrderManager::get_active_order_count() const {
    std::shared_lock lock(orders_mutex_);
    return std::count_if(orders_.begin(), orders_.end(),
                        [](const auto& pair) { return pair.second->is_active(); });
}

void OrderManager::notify_order_update(const Order& order) {
    std::lock_guard lock(callback_mutex_);
    for (const auto& callback : order_callbacks_) {
        try {
            callback(order);
        } catch (const std::exception& e) {
            std::cerr << "Exception in order callback: " << e.what() << "\n";
        }
    }
}

void OrderManager::notify_trade(const Trade& trade) {
    std::lock_guard lock(callback_mutex_);
    for (const auto& callback : trade_callbacks_) {
        try {
            callback(trade);
        } catch (const std::exception& e) {
            std::cerr << "Exception in trade callback: " << e.what() << "\n";
        }
    }
}

void OrderManager::notify_rejection(OrderId order_id, const std::string& reason) {
    std::lock_guard lock(callback_mutex_);
    for (const auto& callback : reject_callbacks_) {
        try {
            callback(order_id, reason);
        } catch (const std::exception& e) {
            std::cerr << "Exception in reject callback: " << e.what() << "\n";
        }
    }
}

// SimulatedExecutionEngine implementation
SimulatedExecutionEngine::SimulatedExecutionEngine(OrderManager& order_manager)
    : order_manager_(order_manager) {}

void SimulatedExecutionEngine::submit_order(const Order& order) {
    std::lock_guard lock(mutex_);
    pending_orders_[order.order_id] = std::make_shared<Order>(order);
}

void SimulatedExecutionEngine::cancel_order(OrderId order_id) {
    std::lock_guard lock(mutex_);
    pending_orders_.erase(order_id);
    order_manager_.on_order_cancelled(order_id);
}

void SimulatedExecutionEngine::process_market_data(const MarketData& data) {
    std::vector<std::shared_ptr<Order>> orders_to_fill;

    {
        std::lock_guard lock(mutex_);
        for (auto& [id, order] : pending_orders_) {
            if (order->symbol == data.symbol) {
                orders_to_fill.push_back(order);
            }
        }
    }

    for (auto& order : orders_to_fill) {
        try_fill_order(order, data);
    }
}

void SimulatedExecutionEngine::try_fill_order(std::shared_ptr<Order> order, const MarketData& data) {
    // Simulate latency
    std::this_thread::sleep_for(std::chrono::microseconds(latency_us_));

    // Check if order should fill
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    if (dis(gen) > fill_probability_) {
        return; // Order doesn't fill this time
    }

    bool should_fill = false;
    Price fill_price = 0.0;

    switch (order->type) {
        case OrderType::MARKET:
            should_fill = true;
            fill_price = calculate_fill_price(*order, data);
            break;

        case OrderType::LIMIT:
            if (order->side == OrderSide::BUY && data.ask <= order->price) {
                should_fill = true;
                fill_price = std::min(order->price, data.ask);
            } else if (order->side == OrderSide::SELL && data.bid >= order->price) {
                should_fill = true;
                fill_price = std::max(order->price, data.bid);
            }
            break;

        case OrderType::STOP:
            if (order->side == OrderSide::BUY && data.last_price >= order->stop_price) {
                should_fill = true;
                fill_price = calculate_fill_price(*order, data);
            } else if (order->side == OrderSide::SELL && data.last_price <= order->stop_price) {
                should_fill = true;
                fill_price = calculate_fill_price(*order, data);
            }
            break;

        default:
            break;
    }

    if (should_fill) {
        Trade trade(order->order_id, order->symbol, order->side,
                   fill_price, order->quantity);
        trade.trade_id = next_trade_id_++;
        trade.timestamp = now();

        // Calculate commission (e.g., 0.1% of trade value)
        trade.commission = fill_price * order->quantity * 0.001;

        order_manager_.on_order_filled(order->order_id, trade);

        std::lock_guard lock(mutex_);
        pending_orders_.erase(order->order_id);
    }
}

Price SimulatedExecutionEngine::calculate_fill_price(const Order& order, const MarketData& data) {
    Price base_price = (order.side == OrderSide::BUY) ? data.ask : data.bid;

    // Add slippage
    double slippage = base_price * (slippage_bps_ / 10000.0);
    if (order.side == OrderSide::BUY) {
        return base_price + slippage;
    } else {
        return base_price - slippage;
    }
}

// TWAPExecutor implementation
TWAPExecutor::TWAPExecutor(OrderManager& order_manager, uint64_t duration_seconds, size_t num_slices)
    : order_manager_(order_manager), duration_seconds_(duration_seconds), num_slices_(num_slices) {}

void TWAPExecutor::execute(const Symbol& symbol, OrderSide side, Quantity total_quantity) {
    running_.store(true);

    execution_thread_ = std::thread([this, symbol, side, total_quantity]() {
        Quantity slice_quantity = total_quantity / num_slices_;
        uint64_t interval_ms = (duration_seconds_ * 1000) / num_slices_;

        for (size_t i = 0; i < num_slices_ && running_.load(); ++i) {
            Order order(symbol, side, OrderType::MARKET, 0.0, slice_quantity);
            order.strategy_id = "TWAP";
            order_manager_.submit_order(order);

            if (i < num_slices_ - 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
            }
        }

        running_.store(false);
    });
}

void TWAPExecutor::cancel() {
    running_.store(false);
    if (execution_thread_.joinable()) {
        execution_thread_.join();
    }
}

} // namespace trading

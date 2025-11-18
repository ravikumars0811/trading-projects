#pragma once

#include "order.hpp"
#include "types.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>
#include <queue>

namespace trading {

// Order callbacks
using OrderCallback = std::function<void(const Order&)>;
using TradeCallback = std::function<void(const Trade&)>;
using RejectCallback = std::function<void(OrderId, const std::string& reason)>;

// Portfolio manager for tracking positions and PnL
class Portfolio {
public:
    Portfolio(double initial_capital);

    // Position management
    void update_position(const Trade& trade);
    Position get_position(const Symbol& symbol) const;
    std::vector<Position> get_all_positions() const;

    // Capital and PnL
    double get_cash() const { return cash_; }
    double get_equity(const std::unordered_map<Symbol, Price>& current_prices) const;
    double get_total_pnl(const std::unordered_map<Symbol, Price>& current_prices) const;

    // Risk metrics
    double get_buying_power() const { return cash_ * margin_multiplier_; }
    double get_margin_used(const std::unordered_map<Symbol, Price>& current_prices) const;
    void set_margin_multiplier(double multiplier) { margin_multiplier_ = multiplier; }

    // Transaction costs
    void deduct_commission(double commission);
    double get_total_commission() const { return total_commission_; }

private:
    mutable std::shared_mutex mutex_;
    double initial_capital_;
    double cash_;
    double margin_multiplier_{1.0};
    double total_commission_{0.0};
    std::unordered_map<Symbol, Position> positions_;
};

// Order Management System
class OrderManager {
public:
    OrderManager(std::shared_ptr<Portfolio> portfolio);
    ~OrderManager();

    // Order submission
    OrderId submit_order(const Order& order);
    bool cancel_order(OrderId order_id);
    bool modify_order(OrderId order_id, Price new_price, Quantity new_quantity);

    // Order queries
    std::shared_ptr<Order> get_order(OrderId order_id) const;
    std::vector<std::shared_ptr<Order>> get_active_orders() const;
    std::vector<std::shared_ptr<Order>> get_orders_by_symbol(const Symbol& symbol) const;
    std::vector<std::shared_ptr<Order>> get_orders_by_strategy(const std::string& strategy_id) const;

    // Order status updates (called by execution engine)
    void on_order_filled(OrderId order_id, const Trade& trade);
    void on_order_partially_filled(OrderId order_id, const Trade& trade);
    void on_order_cancelled(OrderId order_id);
    void on_order_rejected(OrderId order_id, const std::string& reason);

    // Callbacks
    void register_order_callback(const OrderCallback& callback);
    void register_trade_callback(const TradeCallback& callback);
    void register_reject_callback(const RejectCallback& callback);

    // Statistics
    size_t get_total_orders() const { return next_order_id_.load() - 1; }
    size_t get_active_order_count() const;

    // Portfolio access
    std::shared_ptr<Portfolio> get_portfolio() const { return portfolio_; }

private:
    void notify_order_update(const Order& order);
    void notify_trade(const Trade& trade);
    void notify_rejection(OrderId order_id, const std::string& reason);

    std::atomic<OrderId> next_order_id_{1};
    mutable std::shared_mutex orders_mutex_;
    std::unordered_map<OrderId, std::shared_ptr<Order>> orders_;

    std::shared_ptr<Portfolio> portfolio_;

    // Callbacks
    std::mutex callback_mutex_;
    std::vector<OrderCallback> order_callbacks_;
    std::vector<TradeCallback> trade_callbacks_;
    std::vector<RejectCallback> reject_callbacks_;
};

// Execution engine interface
class IExecutionEngine {
public:
    virtual ~IExecutionEngine() = default;
    virtual void submit_order(const Order& order) = 0;
    virtual void cancel_order(OrderId order_id) = 0;
    virtual bool is_connected() const = 0;
};

// Simulated execution engine for backtesting
class SimulatedExecutionEngine : public IExecutionEngine {
public:
    SimulatedExecutionEngine(OrderManager& order_manager);

    void submit_order(const Order& order) override;
    void cancel_order(OrderId order_id) override;
    bool is_connected() const override { return true; }

    // Backtesting functions
    void process_market_data(const MarketData& data);
    void set_slippage(double slippage_bps) { slippage_bps_ = slippage_bps; }
    void set_latency(uint64_t latency_us) { latency_us_ = latency_us; }
    void set_fill_probability(double prob) { fill_probability_ = prob; }

private:
    void try_fill_order(std::shared_ptr<Order> order, const MarketData& data);
    Price calculate_fill_price(const Order& order, const MarketData& data);

    OrderManager& order_manager_;
    std::unordered_map<OrderId, std::shared_ptr<Order>> pending_orders_;
    mutable std::mutex mutex_;

    double slippage_bps_{1.0};
    uint64_t latency_us_{1000};
    double fill_probability_{0.99};
    uint64_t next_trade_id_{1};
};

// Live execution engine (Alpaca, Interactive Brokers, etc.)
class AlpacaExecutionEngine : public IExecutionEngine {
public:
    AlpacaExecutionEngine(const std::string& api_key, const std::string& api_secret,
                          OrderManager& order_manager, bool paper_trading = true);
    ~AlpacaExecutionEngine() override;

    void connect();
    void disconnect();

    void submit_order(const Order& order) override;
    void cancel_order(OrderId order_id) override;
    bool is_connected() const override { return connected_.load(); }

private:
    void handle_order_update(const std::string& message);
    std::string order_to_json(const Order& order);

    std::string api_key_;
    std::string api_secret_;
    std::string base_url_;
    OrderManager& order_manager_;
    std::atomic<bool> connected_{false};

    // Map internal order IDs to broker order IDs
    std::unordered_map<OrderId, std::string> order_id_map_;
    mutable std::mutex map_mutex_;
};

// Smart order router for optimal execution
class SmartOrderRouter {
public:
    SmartOrderRouter(std::vector<std::shared_ptr<IExecutionEngine>> engines);

    void route_order(const Order& order);
    void set_routing_strategy(const std::string& strategy) { routing_strategy_ = strategy; }

private:
    std::shared_ptr<IExecutionEngine> select_venue(const Order& order);

    std::vector<std::shared_ptr<IExecutionEngine>> engines_;
    std::string routing_strategy_{"round_robin"};
    std::atomic<size_t> round_robin_index_{0};
};

// TWAP (Time-Weighted Average Price) execution algorithm
class TWAPExecutor {
public:
    TWAPExecutor(OrderManager& order_manager, uint64_t duration_seconds, size_t num_slices);

    void execute(const Symbol& symbol, OrderSide side, Quantity total_quantity);
    void cancel();

private:
    void execute_slice();

    OrderManager& order_manager_;
    uint64_t duration_seconds_;
    size_t num_slices_;
    std::atomic<bool> running_{false};
    std::thread execution_thread_;
};

// VWAP (Volume-Weighted Average Price) execution algorithm
class VWAPExecutor {
public:
    VWAPExecutor(OrderManager& order_manager);

    void execute(const Symbol& symbol, OrderSide side, Quantity total_quantity,
                 const std::vector<double>& volume_profile);
    void cancel();

private:
    OrderManager& order_manager_;
    std::atomic<bool> running_{false};
};

} // namespace trading

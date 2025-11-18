#pragma once

#include "order.hpp"
#include "order_manager.hpp"
#include "market_data_handler.hpp"
#include <memory>
#include <functional>

namespace trading {

// Risk limits
struct RiskLimits {
    double max_position_size{1000000.0};      // Maximum position size per symbol
    double max_portfolio_value{10000000.0};   // Maximum total portfolio value
    double max_daily_loss{50000.0};           // Maximum daily loss
    double max_drawdown{0.20};                // Maximum drawdown (20%)
    double max_leverage{2.0};                 // Maximum leverage
    double max_concentration{0.30};           // Maximum concentration per symbol (30%)
    double var_limit{100000.0};               // Value at Risk limit
    size_t max_orders_per_second{100};        // Rate limiting
    size_t max_active_orders{1000};           // Maximum active orders
};

// Risk metrics
struct RiskMetrics {
    double current_drawdown{0.0};
    double daily_pnl{0.0};
    double peak_equity{0.0};
    double current_leverage{0.0};
    double portfolio_var{0.0};
    double sharpe_ratio{0.0};
    double max_concentration{0.0};
    size_t orders_last_second{0};
    Timestamp last_update;

    RiskMetrics() : last_update(now()) {}
};

// Pre-trade risk check result
struct RiskCheckResult {
    bool approved{false};
    std::vector<std::string> violations;
    std::string reason;

    void add_violation(const std::string& violation) {
        approved = false;
        violations.push_back(violation);
    }
};

// Risk manager
class RiskManager {
public:
    RiskManager(std::shared_ptr<OrderManager> order_manager,
               std::shared_ptr<MarketDataHandler> market_data_handler,
               const RiskLimits& limits);

    // Pre-trade risk checks
    RiskCheckResult check_order(const Order& order);

    // Post-trade monitoring
    void on_trade(const Trade& trade);
    void on_market_data(const MarketData& data);

    // Risk metrics
    RiskMetrics calculate_risk_metrics();
    RiskMetrics get_current_metrics() const { return current_metrics_; }

    // Circuit breakers
    void trigger_circuit_breaker(const std::string& reason);
    void reset_circuit_breaker();
    bool is_circuit_breaker_active() const { return circuit_breaker_active_.load(); }

    // Dynamic risk limits
    void update_limits(const RiskLimits& new_limits);
    RiskLimits get_limits() const { return limits_; }

    // Position limits
    bool check_position_limit(const Symbol& symbol, Quantity additional_quantity);
    bool check_concentration_limit(const Symbol& symbol, double position_value);

    // Callbacks
    using RiskViolationCallback = std::function<void(const std::string&)>;
    void register_violation_callback(const RiskViolationCallback& callback);

    // Statistics
    size_t get_total_violations() const { return total_violations_.load(); }
    std::vector<std::string> get_recent_violations(size_t count) const;

private:
    double calculate_position_value(const Position& pos, Price current_price);
    double calculate_portfolio_value();
    double calculate_var(double confidence_level = 0.95);
    bool check_rate_limit();
    void update_daily_pnl();
    void notify_violation(const std::string& violation);

    std::shared_ptr<OrderManager> order_manager_;
    std::shared_ptr<MarketDataHandler> market_data_handler_;

    RiskLimits limits_;
    RiskMetrics current_metrics_;

    mutable std::mutex metrics_mutex_;
    std::unordered_map<Symbol, Price> latest_prices_;

    std::atomic<bool> circuit_breaker_active_{false};
    std::string circuit_breaker_reason_;

    // Rate limiting
    std::deque<Timestamp> order_timestamps_;
    mutable std::mutex rate_limit_mutex_;

    // Daily tracking
    double session_start_equity_{0.0};
    Timestamp session_start_time_;

    // Violation tracking
    std::deque<std::string> violation_history_;
    mutable std::mutex violation_mutex_;
    std::atomic<size_t> total_violations_{0};

    // Callbacks
    std::vector<RiskViolationCallback> violation_callbacks_;
    mutable std::mutex callback_mutex_;

    static constexpr size_t MAX_VIOLATION_HISTORY = 1000;
};

// Position sizer with risk management
class RiskAwarePositionSizer {
public:
    RiskAwarePositionSizer(std::shared_ptr<Portfolio> portfolio,
                          std::shared_ptr<RiskManager> risk_manager);

    // Position sizing methods
    Quantity kelly_criterion(const Symbol& symbol, double win_prob,
                           double win_loss_ratio, Price current_price);

    Quantity fixed_fractional(const Symbol& symbol, double risk_per_trade,
                             Price entry_price, Price stop_loss);

    Quantity volatility_based(const Symbol& symbol, double target_volatility,
                             double asset_volatility, Price current_price);

    Quantity equal_weight(size_t num_positions, Price current_price);

private:
    std::shared_ptr<Portfolio> portfolio_;
    std::shared_ptr<RiskManager> risk_manager_;
};

// Value at Risk calculator
class VaRCalculator {
public:
    enum class Method {
        HISTORICAL,
        PARAMETRIC,
        MONTE_CARLO
    };

    VaRCalculator(Method method = Method::HISTORICAL);

    // Calculate VaR for a portfolio
    double calculate(const std::vector<Position>& positions,
                    const std::unordered_map<Symbol, std::vector<double>>& returns,
                    double confidence_level = 0.95);

    // Calculate Conditional VaR (CVaR / Expected Shortfall)
    double calculate_cvar(const std::vector<Position>& positions,
                         const std::unordered_map<Symbol, std::vector<double>>& returns,
                         double confidence_level = 0.95);

private:
    double calculate_historical_var(const std::vector<double>& portfolio_returns,
                                   double confidence_level);

    double calculate_parametric_var(const std::vector<double>& portfolio_returns,
                                   double confidence_level);

    Method method_;
};

// Stop loss manager
class StopLossManager {
public:
    StopLossManager(std::shared_ptr<OrderManager> order_manager);

    // Stop loss types
    void set_fixed_stop(const Symbol& symbol, Price stop_price);
    void set_trailing_stop(const Symbol& symbol, double trail_amount);
    void set_percentage_stop(const Symbol& symbol, double percentage);

    // Update stops based on market data
    void update(const MarketData& data);

    // Remove stop
    void remove_stop(const Symbol& symbol);

private:
    struct StopLoss {
        enum class Type { FIXED, TRAILING, PERCENTAGE };
        Type type;
        Price stop_price;
        double trail_amount;
        double percentage;
        Price highest_price;  // For trailing stops
    };

    std::shared_ptr<OrderManager> order_manager_;
    std::unordered_map<Symbol, StopLoss> stops_;
    mutable std::mutex mutex_;
};

// Correlation monitor
class CorrelationMonitor {
public:
    CorrelationMonitor(size_t lookback_period = 100);

    void update_returns(const Symbol& symbol, double return_value);
    double get_correlation(const Symbol& symbol1, const Symbol& symbol2);
    std::unordered_map<std::string, double> get_all_correlations();

    // Detect concentration risk
    std::vector<std::pair<Symbol, Symbol>> find_highly_correlated_pairs(double threshold = 0.7);

private:
    double calculate_correlation(const std::deque<double>& returns1,
                                const std::deque<double>& returns2);

    size_t lookback_period_;
    std::unordered_map<Symbol, std::deque<double>> returns_history_;
    mutable std::shared_mutex mutex_;
};

} // namespace trading

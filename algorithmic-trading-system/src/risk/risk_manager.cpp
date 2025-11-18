#include "../../include/risk_manager.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>

namespace trading {

// RiskManager implementation
RiskManager::RiskManager(std::shared_ptr<OrderManager> order_manager,
                        std::shared_ptr<MarketDataHandler> market_data_handler,
                        const RiskLimits& limits)
    : order_manager_(order_manager),
      market_data_handler_(market_data_handler),
      limits_(limits),
      session_start_time_(now()) {

    auto portfolio = order_manager_->get_portfolio();
    session_start_equity_ = portfolio->get_cash();
    current_metrics_.peak_equity = session_start_equity_;
}

RiskCheckResult RiskManager::check_order(const Order& order) {
    RiskCheckResult result;
    result.approved = true;

    // Check circuit breaker
    if (circuit_breaker_active_.load()) {
        result.add_violation("Circuit breaker is active: " + circuit_breaker_reason_);
        return result;
    }

    // Check rate limit
    if (!check_rate_limit()) {
        result.add_violation("Order rate limit exceeded");
        notify_violation("Rate limit exceeded");
        return result;
    }

    // Get current market data
    MarketData market_data;
    if (!market_data_handler_->get_market_data(order.symbol, market_data)) {
        result.add_violation("No market data available for " + order.symbol);
        return result;
    }

    // Check position limits
    auto portfolio = order_manager_->get_portfolio();
    auto position = portfolio->get_position(order.symbol);

    Quantity new_quantity = position.quantity;
    if (order.side == OrderSide::BUY) {
        new_quantity += order.quantity;
    } else {
        new_quantity -= order.quantity;
    }

    double position_value = std::abs(new_quantity) * market_data.mid_price();
    if (position_value > limits_.max_position_size) {
        result.add_violation("Position size would exceed limit for " + order.symbol);
        notify_violation("Position size limit exceeded for " + order.symbol);
    }

    // Check concentration limit
    double portfolio_value = calculate_portfolio_value();
    double concentration = position_value / portfolio_value;
    if (concentration > limits_.max_concentration) {
        result.add_violation("Concentration limit would be exceeded for " + order.symbol);
        notify_violation("Concentration limit exceeded for " + order.symbol);
    }

    // Check leverage
    double total_position_value = 0.0;
    for (const auto& pos : portfolio->get_all_positions()) {
        MarketData md;
        if (market_data_handler_->get_market_data(pos.symbol, md)) {
            total_position_value += std::abs(pos.quantity) * md.mid_price();
        }
    }
    total_position_value += position_value;

    double leverage = total_position_value / portfolio->get_cash();
    if (leverage > limits_.max_leverage) {
        result.add_violation("Leverage limit would be exceeded");
        notify_violation("Leverage limit exceeded");
    }

    // Check daily loss limit
    update_daily_pnl();
    if (std::abs(current_metrics_.daily_pnl) > limits_.max_daily_loss) {
        result.add_violation("Daily loss limit exceeded");
        notify_violation("Daily loss limit exceeded");
        trigger_circuit_breaker("Daily loss limit exceeded");
    }

    // Check drawdown
    if (current_metrics_.current_drawdown > limits_.max_drawdown) {
        result.add_violation("Maximum drawdown exceeded");
        notify_violation("Maximum drawdown exceeded");
        trigger_circuit_breaker("Maximum drawdown exceeded");
    }

    // Check active order limit
    if (order_manager_->get_active_order_count() >= limits_.max_active_orders) {
        result.add_violation("Too many active orders");
        notify_violation("Active order limit exceeded");
    }

    return result;
}

void RiskManager::on_trade(const Trade& trade) {
    update_daily_pnl();
    calculate_risk_metrics();
}

void RiskManager::on_market_data(const MarketData& data) {
    std::lock_guard lock(metrics_mutex_);
    latest_prices_[data.symbol] = data.mid_price();
}

RiskMetrics RiskManager::calculate_risk_metrics() {
    std::lock_guard lock(metrics_mutex_);

    auto portfolio = order_manager_->get_portfolio();
    double portfolio_value = calculate_portfolio_value();

    // Update peak equity and drawdown
    if (portfolio_value > current_metrics_.peak_equity) {
        current_metrics_.peak_equity = portfolio_value;
    }

    if (current_metrics_.peak_equity > 0) {
        current_metrics_.current_drawdown =
            (current_metrics_.peak_equity - portfolio_value) / current_metrics_.peak_equity;
    }

    // Calculate leverage
    double total_position_value = 0.0;
    for (const auto& pos : portfolio->get_all_positions()) {
        auto it = latest_prices_.find(pos.symbol);
        if (it != latest_prices_.end()) {
            total_position_value += std::abs(pos.quantity) * it->second;
        }
    }
    current_metrics_.current_leverage = total_position_value / portfolio->get_cash();

    // Calculate concentration
    current_metrics_.max_concentration = 0.0;
    for (const auto& pos : portfolio->get_all_positions()) {
        auto it = latest_prices_.find(pos.symbol);
        if (it != latest_prices_.end()) {
            double pos_value = std::abs(pos.quantity) * it->second;
            double concentration = pos_value / portfolio_value;
            current_metrics_.max_concentration = std::max(
                current_metrics_.max_concentration, concentration);
        }
    }

    // Update daily PnL
    update_daily_pnl();

    current_metrics_.last_update = now();
    return current_metrics_;
}

void RiskManager::trigger_circuit_breaker(const std::string& reason) {
    circuit_breaker_active_.store(true);
    circuit_breaker_reason_ = reason;

    std::cout << "CIRCUIT BREAKER TRIGGERED: " << reason << "\n";
    notify_violation("Circuit breaker triggered: " + reason);

    // Cancel all active orders
    for (const auto& order : order_manager_->get_active_orders()) {
        order_manager_->cancel_order(order->order_id);
    }
}

void RiskManager::reset_circuit_breaker() {
    circuit_breaker_active_.store(false);
    circuit_breaker_reason_.clear();
    std::cout << "Circuit breaker reset\n";
}

void RiskManager::update_limits(const RiskLimits& new_limits) {
    std::lock_guard lock(metrics_mutex_);
    limits_ = new_limits;
}

bool RiskManager::check_position_limit(const Symbol& symbol, Quantity additional_quantity) {
    auto portfolio = order_manager_->get_portfolio();
    auto position = portfolio->get_position(symbol);

    MarketData market_data;
    if (!market_data_handler_->get_market_data(symbol, market_data)) {
        return false;
    }

    double new_position_value = std::abs(position.quantity + additional_quantity) *
                               market_data.mid_price();
    return new_position_value <= limits_.max_position_size;
}

bool RiskManager::check_concentration_limit(const Symbol& symbol, double position_value) {
    double portfolio_value = calculate_portfolio_value();
    double concentration = position_value / portfolio_value;
    return concentration <= limits_.max_concentration;
}

void RiskManager::register_violation_callback(const RiskViolationCallback& callback) {
    std::lock_guard lock(callback_mutex_);
    violation_callbacks_.push_back(callback);
}

std::vector<std::string> RiskManager::get_recent_violations(size_t count) const {
    std::lock_guard lock(violation_mutex_);
    size_t start = violation_history_.size() > count ? violation_history_.size() - count : 0;
    return std::vector<std::string>(violation_history_.begin() + start,
                                   violation_history_.end());
}

double RiskManager::calculate_position_value(const Position& pos, Price current_price) {
    return std::abs(pos.quantity) * current_price;
}

double RiskManager::calculate_portfolio_value() {
    auto portfolio = order_manager_->get_portfolio();
    std::unordered_map<Symbol, Price> prices;

    {
        std::lock_guard lock(metrics_mutex_);
        prices = latest_prices_;
    }

    return portfolio->get_equity(prices);
}

double RiskManager::calculate_var(double confidence_level) {
    // Simplified VaR calculation
    // In production, use historical simulation or Monte Carlo
    double portfolio_value = calculate_portfolio_value();
    double volatility = 0.02; // Assume 2% daily volatility
    double z_score = 1.645; // 95% confidence

    if (confidence_level >= 0.99) {
        z_score = 2.326;
    }

    return portfolio_value * volatility * z_score;
}

bool RiskManager::check_rate_limit() {
    std::lock_guard lock(rate_limit_mutex_);

    auto now_time = now();
    auto one_second_ago = now_time - std::chrono::seconds(1);

    // Remove old timestamps
    while (!order_timestamps_.empty() &&
           order_timestamps_.front() < one_second_ago) {
        order_timestamps_.pop_front();
    }

    if (order_timestamps_.size() >= limits_.max_orders_per_second) {
        return false;
    }

    order_timestamps_.push_back(now_time);
    return true;
}

void RiskManager::update_daily_pnl() {
    auto portfolio = order_manager_->get_portfolio();
    std::unordered_map<Symbol, Price> prices;

    {
        std::lock_guard lock(metrics_mutex_);
        prices = latest_prices_;
    }

    double current_equity = portfolio->get_equity(prices);
    current_metrics_.daily_pnl = current_equity - session_start_equity_;
}

void RiskManager::notify_violation(const std::string& violation) {
    {
        std::lock_guard lock(violation_mutex_);
        violation_history_.push_back(violation);
        if (violation_history_.size() > MAX_VIOLATION_HISTORY) {
            violation_history_.pop_front();
        }
    }

    total_violations_.fetch_add(1);

    std::lock_guard lock(callback_mutex_);
    for (const auto& callback : violation_callbacks_) {
        try {
            callback(violation);
        } catch (const std::exception& e) {
            std::cerr << "Exception in violation callback: " << e.what() << "\n";
        }
    }
}

// RiskAwarePositionSizer implementation
RiskAwarePositionSizer::RiskAwarePositionSizer(
    std::shared_ptr<Portfolio> portfolio,
    std::shared_ptr<RiskManager> risk_manager)
    : portfolio_(portfolio), risk_manager_(risk_manager) {}

Quantity RiskAwarePositionSizer::kelly_criterion(
    const Symbol& symbol, double win_prob, double win_loss_ratio, Price current_price) {

    double kelly_fraction = (win_prob * win_loss_ratio - (1 - win_prob)) / win_loss_ratio;
    kelly_fraction = std::max(0.0, std::min(kelly_fraction, 0.25)); // Cap at 25%

    double capital = portfolio_->get_cash();
    double position_value = capital * kelly_fraction;

    return position_value / current_price;
}

Quantity RiskAwarePositionSizer::fixed_fractional(
    const Symbol& symbol, double risk_per_trade, Price entry_price, Price stop_loss) {

    double capital = portfolio_->get_cash();
    double risk_amount = capital * risk_per_trade;

    double price_risk = std::abs(entry_price - stop_loss);
    if (price_risk < 1e-6) return 0.0;

    return risk_amount / price_risk;
}

Quantity RiskAwarePositionSizer::volatility_based(
    const Symbol& symbol, double target_volatility, double asset_volatility, Price current_price) {

    if (asset_volatility < 1e-6) return 0.0;

    double capital = portfolio_->get_cash();
    double position_value = capital * (target_volatility / asset_volatility);

    return position_value / current_price;
}

Quantity RiskAwarePositionSizer::equal_weight(size_t num_positions, Price current_price) {
    double capital = portfolio_->get_cash();
    double position_value = capital / num_positions;

    return position_value / current_price;
}

// VaRCalculator implementation
VaRCalculator::VaRCalculator(Method method) : method_(method) {}

double VaRCalculator::calculate(
    const std::vector<Position>& positions,
    const std::unordered_map<Symbol, std::vector<double>>& returns,
    double confidence_level) {

    // Calculate portfolio returns
    std::vector<double> portfolio_returns;

    // Determine max length
    size_t max_length = 0;
    for (const auto& [symbol, rets] : returns) {
        max_length = std::max(max_length, rets.size());
    }

    for (size_t i = 0; i < max_length; ++i) {
        double portfolio_return = 0.0;
        double total_weight = 0.0;

        for (const auto& pos : positions) {
            auto it = returns.find(pos.symbol);
            if (it != returns.end() && i < it->second.size()) {
                double weight = std::abs(pos.quantity);
                portfolio_return += it->second[i] * weight;
                total_weight += weight;
            }
        }

        if (total_weight > 0) {
            portfolio_returns.push_back(portfolio_return / total_weight);
        }
    }

    return calculate_historical_var(portfolio_returns, confidence_level);
}

double VaRCalculator::calculate_cvar(
    const std::vector<Position>& positions,
    const std::unordered_map<Symbol, std::vector<double>>& returns,
    double confidence_level) {

    // First calculate VaR
    double var = calculate(positions, returns, confidence_level);

    // Calculate mean of returns below VaR
    std::vector<double> portfolio_returns;
    // ... (similar to calculate())

    std::vector<double> tail_returns;
    for (double ret : portfolio_returns) {
        if (ret <= -var) {
            tail_returns.push_back(ret);
        }
    }

    if (tail_returns.empty()) return var;

    double mean_tail = std::accumulate(tail_returns.begin(), tail_returns.end(), 0.0) /
                      tail_returns.size();
    return -mean_tail;
}

double VaRCalculator::calculate_historical_var(
    const std::vector<double>& portfolio_returns, double confidence_level) {

    if (portfolio_returns.empty()) return 0.0;

    std::vector<double> sorted_returns = portfolio_returns;
    std::sort(sorted_returns.begin(), sorted_returns.end());

    size_t index = static_cast<size_t>((1.0 - confidence_level) * sorted_returns.size());
    return -sorted_returns[index];
}

double VaRCalculator::calculate_parametric_var(
    const std::vector<double>& portfolio_returns, double confidence_level) {

    if (portfolio_returns.empty()) return 0.0;

    double mean = std::accumulate(portfolio_returns.begin(), portfolio_returns.end(), 0.0) /
                 portfolio_returns.size();

    double variance = 0.0;
    for (double ret : portfolio_returns) {
        variance += (ret - mean) * (ret - mean);
    }
    double std_dev = std::sqrt(variance / portfolio_returns.size());

    double z_score = 1.645; // 95% confidence
    if (confidence_level >= 0.99) {
        z_score = 2.326;
    }

    return z_score * std_dev;
}

// StopLossManager implementation
StopLossManager::StopLossManager(std::shared_ptr<OrderManager> order_manager)
    : order_manager_(order_manager) {}

void StopLossManager::set_fixed_stop(const Symbol& symbol, Price stop_price) {
    std::lock_guard lock(mutex_);
    StopLoss stop;
    stop.type = StopLoss::Type::FIXED;
    stop.stop_price = stop_price;
    stops_[symbol] = stop;
}

void StopLossManager::set_trailing_stop(const Symbol& symbol, double trail_amount) {
    std::lock_guard lock(mutex_);
    StopLoss stop;
    stop.type = StopLoss::Type::TRAILING;
    stop.trail_amount = trail_amount;
    stop.highest_price = 0.0;
    stops_[symbol] = stop;
}

void StopLossManager::set_percentage_stop(const Symbol& symbol, double percentage) {
    std::lock_guard lock(mutex_);
    StopLoss stop;
    stop.type = StopLoss::Type::PERCENTAGE;
    stop.percentage = percentage;
    stops_[symbol] = stop;
}

void StopLossManager::update(const MarketData& data) {
    std::lock_guard lock(mutex_);

    auto it = stops_.find(data.symbol);
    if (it == stops_.end()) return;

    auto& stop = it->second;
    bool should_exit = false;

    switch (stop.type) {
        case StopLoss::Type::FIXED:
            if (data.last_price <= stop.stop_price) {
                should_exit = true;
            }
            break;

        case StopLoss::Type::TRAILING:
            if (data.last_price > stop.highest_price) {
                stop.highest_price = data.last_price;
                stop.stop_price = data.last_price - stop.trail_amount;
            }
            if (data.last_price <= stop.stop_price) {
                should_exit = true;
            }
            break;

        case StopLoss::Type::PERCENTAGE:
            {
                auto portfolio = order_manager_->get_portfolio();
                auto position = portfolio->get_position(data.symbol);
                double loss = (position.avg_price - data.last_price) / position.avg_price;
                if (loss >= stop.percentage) {
                    should_exit = true;
                }
            }
            break;
    }

    if (should_exit) {
        // Get position and create exit order
        auto portfolio = order_manager_->get_portfolio();
        auto position = portfolio->get_position(data.symbol);

        if (std::abs(position.quantity) > 1e-6) {
            OrderSide side = position.quantity > 0 ? OrderSide::SELL : OrderSide::BUY;
            Order exit_order(data.symbol, side, OrderType::MARKET, 0.0,
                           std::abs(position.quantity));
            exit_order.strategy_id = "StopLoss";
            order_manager_->submit_order(exit_order);

            std::cout << "Stop loss triggered for " << data.symbol << "\n";
        }

        stops_.erase(it);
    }
}

void StopLossManager::remove_stop(const Symbol& symbol) {
    std::lock_guard lock(mutex_);
    stops_.erase(symbol);
}

// CorrelationMonitor implementation
CorrelationMonitor::CorrelationMonitor(size_t lookback_period)
    : lookback_period_(lookback_period) {}

void CorrelationMonitor::update_returns(const Symbol& symbol, double return_value) {
    std::unique_lock lock(mutex_);
    auto& history = returns_history_[symbol];
    history.push_back(return_value);

    if (history.size() > lookback_period_) {
        history.pop_front();
    }
}

double CorrelationMonitor::get_correlation(const Symbol& symbol1, const Symbol& symbol2) {
    std::shared_lock lock(mutex_);

    auto it1 = returns_history_.find(symbol1);
    auto it2 = returns_history_.find(symbol2);

    if (it1 == returns_history_.end() || it2 == returns_history_.end()) {
        return 0.0;
    }

    return calculate_correlation(it1->second, it2->second);
}

std::unordered_map<std::string, double> CorrelationMonitor::get_all_correlations() {
    std::shared_lock lock(mutex_);
    std::unordered_map<std::string, double> correlations;

    std::vector<Symbol> symbols;
    for (const auto& [symbol, _] : returns_history_) {
        symbols.push_back(symbol);
    }

    for (size_t i = 0; i < symbols.size(); ++i) {
        for (size_t j = i + 1; j < symbols.size(); ++j) {
            std::string key = symbols[i] + "_" + symbols[j];
            correlations[key] = calculate_correlation(
                returns_history_[symbols[i]], returns_history_[symbols[j]]);
        }
    }

    return correlations;
}

std::vector<std::pair<Symbol, Symbol>> CorrelationMonitor::find_highly_correlated_pairs(
    double threshold) {

    std::shared_lock lock(mutex_);
    std::vector<std::pair<Symbol, Symbol>> pairs;

    std::vector<Symbol> symbols;
    for (const auto& [symbol, _] : returns_history_) {
        symbols.push_back(symbol);
    }

    for (size_t i = 0; i < symbols.size(); ++i) {
        for (size_t j = i + 1; j < symbols.size(); ++j) {
            double corr = calculate_correlation(
                returns_history_[symbols[i]], returns_history_[symbols[j]]);

            if (std::abs(corr) >= threshold) {
                pairs.emplace_back(symbols[i], symbols[j]);
            }
        }
    }

    return pairs;
}

double CorrelationMonitor::calculate_correlation(
    const std::deque<double>& returns1, const std::deque<double>& returns2) {

    size_t n = std::min(returns1.size(), returns2.size());
    if (n < 2) return 0.0;

    double mean1 = std::accumulate(returns1.begin(), returns1.end(), 0.0) / n;
    double mean2 = std::accumulate(returns2.begin(), returns2.end(), 0.0) / n;

    double cov = 0.0;
    double var1 = 0.0;
    double var2 = 0.0;

    for (size_t i = 0; i < n; ++i) {
        double diff1 = returns1[i] - mean1;
        double diff2 = returns2[i] - mean2;
        cov += diff1 * diff2;
        var1 += diff1 * diff1;
        var2 += diff2 * diff2;
    }

    double denom = std::sqrt(var1 * var2);
    return denom > 1e-10 ? cov / denom : 0.0;
}

} // namespace trading

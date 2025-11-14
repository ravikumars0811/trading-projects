#include "risk/risk_manager.hpp"
#include "core/timer.hpp"
#include "core/logger.hpp"
#include <cmath>

namespace hft {
namespace risk {

RiskManager::RiskManager(oms::PositionManager& position_manager)
    : position_manager_(position_manager) {}

void RiskManager::setGlobalRiskLimits(const RiskLimits& limits) {
    global_limits_ = limits;
    LOG_INFO("Global risk limits updated");
}

void RiskManager::setSymbolRiskLimits(const std::string& symbol, const RiskLimits& limits) {
    symbol_limits_[symbol] = limits;
    LOG_INFO("Risk limits updated for symbol: ", symbol);
}

RiskCheckResult RiskManager::checkOrder(const oms::OrderRequest& request,
                                       uint64_t market_price) const {
    const RiskLimits& limits = getRiskLimits(request.symbol);

    // Check order size
    if (request.quantity > limits.max_order_size) {
        LOG_WARNING("Risk check failed: Order size too large - ",
                   request.quantity, " > ", limits.max_order_size);
        return RiskCheckResult::FAIL_ORDER_SIZE;
    }

    // Check position limit
    int64_t current_position = position_manager_.getNetPosition(request.symbol);
    int64_t position_delta = (request.side == market_data::Side::BUY) ?
                            request.quantity : -static_cast<int64_t>(request.quantity);
    int64_t new_position = current_position + position_delta;

    if (std::abs(new_position) > limits.max_position) {
        LOG_WARNING("Risk check failed: Position limit - Current=", current_position,
                   " Delta=", position_delta, " Limit=", limits.max_position);
        return RiskCheckResult::FAIL_POSITION_LIMIT;
    }

    // Check price collar (if market price is available)
    if (market_price > 0 && request.price > 0) {
        double price_deviation = std::abs(static_cast<double>(request.price) -
                                         static_cast<double>(market_price)) /
                                market_price * 100.0;

        if (price_deviation > limits.price_collar_percent) {
            LOG_WARNING("Risk check failed: Price collar - Deviation=",
                       price_deviation, "% > ", limits.price_collar_percent, "%");
            return RiskCheckResult::FAIL_PRICE_COLLAR;
        }
    }

    // Check order rate
    uint64_t current_time = core::Timer::timestamp_ns();
    auto& last_time = last_order_time_[request.symbol];
    auto& order_cnt = order_count_[request.symbol];

    if (current_time - last_time > 1000000000) {  // 1 second
        order_cnt = 0;
        last_time = current_time;
    }

    if (order_cnt >= limits.max_orders_per_second) {
        LOG_WARNING("Risk check failed: Order rate limit - ",
                   order_cnt, " >= ", limits.max_orders_per_second);
        return RiskCheckResult::FAIL_ORDER_RATE;
    }

    order_cnt++;

    return RiskCheckResult::PASS;
}

void RiskManager::updateOrderMetrics(const std::string& symbol) {
    // Update metrics (called after order submission)
}

bool RiskManager::checkPnLLimit(const std::string& symbol, double current_price) const {
    const RiskLimits& limits = getRiskLimits(symbol);

    double realized_pnl = position_manager_.getRealizedPnL(symbol);
    double unrealized_pnl = position_manager_.getUnrealizedPnL(symbol, current_price);
    double total_pnl = realized_pnl + unrealized_pnl;

    if (total_pnl < -limits.max_loss_per_symbol) {
        LOG_WARNING("PnL limit breached for ", symbol,
                   " - Loss: ", -total_pnl, " > ", limits.max_loss_per_symbol);
        return false;
    }

    return true;
}

bool RiskManager::isPositionLimitBreached(const std::string& symbol) const {
    const RiskLimits& limits = getRiskLimits(symbol);
    int64_t position = position_manager_.getNetPosition(symbol);
    return std::abs(position) > limits.max_position;
}

bool RiskManager::isPnLLimitBreached(const std::string& symbol, double current_price) const {
    return !checkPnLLimit(symbol, current_price);
}

const RiskLimits& RiskManager::getRiskLimits(const std::string& symbol) const {
    auto it = symbol_limits_.find(symbol);
    return (it != symbol_limits_.end()) ? it->second : global_limits_;
}

} // namespace risk
} // namespace hft

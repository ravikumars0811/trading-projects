#include "strategy/stat_arb_strategy.hpp"
#include "core/timer.hpp"
#include "core/logger.hpp"
#include <cmath>
#include <numeric>

namespace hft {
namespace strategy {

StatArbStrategy::StatArbStrategy(oms::OrderManager& order_manager,
                                oms::PositionManager& position_manager,
                                const Parameters& params)
    : StrategyBase("StatArb", order_manager, position_manager),
      params_(params) {}

void StatArbStrategy::initialize() {
    LOG_INFO("Initializing Statistical Arbitrage Strategy for ", symbol_);
    price_history_.clear();
}

void StatArbStrategy::shutdown() {
    LOG_INFO("Shutting down Statistical Arbitrage Strategy");
}

void StatArbStrategy::onMarketData(const std::string& symbol,
                                  const market_data::OrderBook& order_book) {
    if (!enabled_ || symbol != symbol_) {
        return;
    }

    uint64_t mid_price = order_book.getMidPrice();
    if (mid_price == 0) {
        return;
    }

    double mid_price_d = static_cast<double>(mid_price);

    // Update statistics
    updateStatistics(mid_price_d);

    if (price_history_.size() < params_.lookback_period) {
        return;  // Not enough data yet
    }

    // Calculate z-score
    double z_score = calculateZScore(mid_price_d, current_mean_, current_std_dev_);

    LOG_DEBUG("StatArb: MidPrice=", mid_price, " Mean=", current_mean_,
             " StdDev=", current_std_dev_, " ZScore=", z_score);

    // Check for signals
    checkEntrySignal(order_book, z_score);
    checkExitSignal(order_book, z_score);
}

void StatArbStrategy::onOrderUpdate(const oms::OrderState& state) {
    // Handle order updates
}

void StatArbStrategy::onFill(const oms::Fill& fill) {
    LOG_INFO("StatArb filled: Price=", fill.price, " Qty=", fill.quantity);
}

void StatArbStrategy::updateStatistics(double mid_price) {
    price_history_.push_back(mid_price);

    if (price_history_.size() > params_.lookback_period) {
        price_history_.pop_front();
    }

    if (price_history_.size() >= params_.lookback_period) {
        current_mean_ = calculateMean();
        current_std_dev_ = calculateStdDev(current_mean_);
    }
}

double StatArbStrategy::calculateMean() const {
    return std::accumulate(price_history_.begin(), price_history_.end(), 0.0) /
           price_history_.size();
}

double StatArbStrategy::calculateStdDev(double mean) const {
    double sum_squared_diff = 0.0;

    for (double price : price_history_) {
        double diff = price - mean;
        sum_squared_diff += diff * diff;
    }

    return std::sqrt(sum_squared_diff / price_history_.size());
}

double StatArbStrategy::calculateZScore(double price, double mean, double std_dev) const {
    if (std_dev == 0.0) {
        return 0.0;
    }
    return (price - mean) / std_dev;
}

void StatArbStrategy::checkEntrySignal(const market_data::OrderBook& order_book,
                                      double z_score) {
    if (current_state_ != State::FLAT) {
        return;  // Already in a position
    }

    int64_t current_position = position_manager_.getNetPosition(symbol_);

    // Price is too high (mean reversion: sell)
    if (z_score > params_.entry_threshold &&
        current_position > -params_.max_position) {

        oms::OrderRequest request{
            0,
            symbol_,
            market_data::Side::SELL,
            oms::OrderType::LIMIT,
            order_book.getBestBid(),  // Aggressive order
            params_.position_size,
            core::Timer::timestamp_ns()
        };

        order_manager_.submitOrder(request);
        current_state_ = State::SHORT;

        LOG_INFO("StatArb Entry: SHORT signal, ZScore=", z_score);
    }
    // Price is too low (mean reversion: buy)
    else if (z_score < -params_.entry_threshold &&
             current_position < params_.max_position) {

        oms::OrderRequest request{
            0,
            symbol_,
            market_data::Side::BUY,
            oms::OrderType::LIMIT,
            order_book.getBestAsk(),  // Aggressive order
            params_.position_size,
            core::Timer::timestamp_ns()
        };

        order_manager_.submitOrder(request);
        current_state_ = State::LONG;

        LOG_INFO("StatArb Entry: LONG signal, ZScore=", z_score);
    }
}

void StatArbStrategy::checkExitSignal(const market_data::OrderBook& order_book,
                                     double z_score) {
    int64_t current_position = position_manager_.getNetPosition(symbol_);

    // Exit short position
    if (current_state_ == State::SHORT && z_score < params_.exit_threshold) {
        oms::OrderRequest request{
            0,
            symbol_,
            market_data::Side::BUY,
            oms::OrderType::LIMIT,
            order_book.getBestAsk(),
            static_cast<uint32_t>(std::abs(current_position)),
            core::Timer::timestamp_ns()
        };

        order_manager_.submitOrder(request);
        current_state_ = State::FLAT;

        LOG_INFO("StatArb Exit: Closing SHORT, ZScore=", z_score);
    }
    // Exit long position
    else if (current_state_ == State::LONG && z_score > -params_.exit_threshold) {
        oms::OrderRequest request{
            0,
            symbol_,
            market_data::Side::SELL,
            oms::OrderType::LIMIT,
            order_book.getBestBid(),
            static_cast<uint32_t>(std::abs(current_position)),
            core::Timer::timestamp_ns()
        };

        order_manager_.submitOrder(request);
        current_state_ = State::FLAT;

        LOG_INFO("StatArb Exit: Closing LONG, ZScore=", z_score);
    }
}

} // namespace strategy
} // namespace hft

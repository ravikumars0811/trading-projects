#pragma once

#include "strategy_base.hpp"
#include <deque>
#include <array>

namespace hft {
namespace strategy {

/**
 * Statistical Arbitrage Strategy
 * Mean reversion strategy based on price deviations
 */
class StatArbStrategy : public StrategyBase {
public:
    struct Parameters {
        size_t lookback_period = 100;      // Number of ticks for mean calculation
        double entry_threshold = 2.0;      // Standard deviations for entry
        double exit_threshold = 0.5;       // Standard deviations for exit
        uint32_t position_size = 100;      // Size per trade
        int32_t max_position = 500;        // Maximum position limit
        double tick_size = 1.0;            // Minimum price increment
    };

    StatArbStrategy(oms::OrderManager& order_manager,
                   oms::PositionManager& position_manager,
                   const Parameters& params);

    void initialize() override;
    void shutdown() override;

    void onMarketData(const std::string& symbol,
                     const market_data::OrderBook& order_book) override;

    void onOrderUpdate(const oms::OrderState& state) override;
    void onFill(const oms::Fill& fill) override;

    void setParameters(const Parameters& params) { params_ = params; }
    const Parameters& getParameters() const { return params_; }

private:
    void updateStatistics(double mid_price);
    double calculateMean() const;
    double calculateStdDev(double mean) const;
    double calculateZScore(double price, double mean, double std_dev) const;

    void checkEntrySignal(const market_data::OrderBook& order_book,
                         double z_score);
    void checkExitSignal(const market_data::OrderBook& order_book,
                        double z_score);

    Parameters params_;
    std::deque<double> price_history_;
    double current_mean_ = 0.0;
    double current_std_dev_ = 0.0;

    enum class State {
        FLAT,
        LONG,
        SHORT
    };
    State current_state_ = State::FLAT;
};

} // namespace strategy
} // namespace hft

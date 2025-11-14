#pragma once

#include "strategy_base.hpp"
#include <unordered_map>

namespace hft {
namespace strategy {

/**
 * Market Making Strategy
 * Provides liquidity by placing bids and offers around the mid-price
 */
class MarketMakingStrategy : public StrategyBase {
public:
    struct Parameters {
        double spread_bps = 10.0;          // Spread in basis points
        uint32_t quote_size = 100;         // Size per quote
        int32_t max_position = 1000;       // Maximum position limit
        double tick_size = 1.0;            // Minimum price increment
        uint32_t max_orders_per_side = 5;  // Max orders on each side
        uint64_t quote_refresh_ms = 100;   // Quote refresh interval
    };

    MarketMakingStrategy(oms::OrderManager& order_manager,
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
    void updateQuotes(const market_data::OrderBook& order_book);
    void cancelAllOrders();
    bool shouldQuote(const market_data::OrderBook& order_book) const;

    uint64_t calculateBidPrice(uint64_t mid_price) const;
    uint64_t calculateAskPrice(uint64_t mid_price) const;
    uint32_t calculateQuoteSize(market_data::Side side) const;

    Parameters params_;
    std::unordered_map<uint64_t, oms::OrderRequest> active_orders_;
    uint64_t last_quote_time_ = 0;
};

} // namespace strategy
} // namespace hft

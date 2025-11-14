#include "strategy/market_making_strategy.hpp"
#include "core/timer.hpp"
#include "core/logger.hpp"
#include <cmath>

namespace hft {
namespace strategy {

MarketMakingStrategy::MarketMakingStrategy(oms::OrderManager& order_manager,
                                          oms::PositionManager& position_manager,
                                          const Parameters& params)
    : StrategyBase("MarketMaking", order_manager, position_manager),
      params_(params) {}

void MarketMakingStrategy::initialize() {
    LOG_INFO("Initializing Market Making Strategy for ", symbol_);
}

void MarketMakingStrategy::shutdown() {
    LOG_INFO("Shutting down Market Making Strategy");
    cancelAllOrders();
}

void MarketMakingStrategy::onMarketData(const std::string& symbol,
                                       const market_data::OrderBook& order_book) {
    if (!enabled_ || symbol != symbol_) {
        return;
    }

    uint64_t current_time = core::Timer::timestamp_ns();
    uint64_t time_since_last_quote = (current_time - last_quote_time_) / 1000000; // Convert to ms

    // Check if we should update quotes
    if (time_since_last_quote < params_.quote_refresh_ms) {
        return;
    }

    if (shouldQuote(order_book)) {
        updateQuotes(order_book);
        last_quote_time_ = current_time;
    }
}

void MarketMakingStrategy::onOrderUpdate(const oms::OrderState& state) {
    // Handle order updates
    if (state.status == oms::OrderStatus::FILLED ||
        state.status == oms::OrderStatus::CANCELLED ||
        state.status == oms::OrderStatus::REJECTED) {
        active_orders_.erase(state.request.client_order_id);
    }
}

void MarketMakingStrategy::onFill(const oms::Fill& fill) {
    LOG_INFO("Market maker filled: Price=", fill.price, " Qty=", fill.quantity);
}

void MarketMakingStrategy::updateQuotes(const market_data::OrderBook& order_book) {
    uint64_t mid_price = order_book.getMidPrice();
    if (mid_price == 0) {
        return;
    }

    int64_t current_position = position_manager_.getNetPosition(symbol_);

    // Calculate quote prices
    uint64_t bid_price = calculateBidPrice(mid_price);
    uint64_t ask_price = calculateAskPrice(mid_price);

    // Adjust quote size based on position
    uint32_t bid_size = calculateQuoteSize(market_data::Side::BUY);
    uint32_t ask_size = calculateQuoteSize(market_data::Side::SELL);

    // Cancel existing orders
    cancelAllOrders();

    // Place bid order if we're not at position limit
    if (current_position < params_.max_position) {
        oms::OrderRequest bid_request{
            0,  // Will be assigned by order manager
            symbol_,
            market_data::Side::BUY,
            oms::OrderType::LIMIT,
            bid_price,
            bid_size,
            core::Timer::timestamp_ns()
        };

        uint64_t order_id = order_manager_.submitOrder(bid_request);
        active_orders_[order_id] = bid_request;
    }

    // Place ask order if we're not at position limit
    if (current_position > -params_.max_position) {
        oms::OrderRequest ask_request{
            0,  // Will be assigned by order manager
            symbol_,
            market_data::Side::SELL,
            oms::OrderType::LIMIT,
            ask_price,
            ask_size,
            core::Timer::timestamp_ns()
        };

        uint64_t order_id = order_manager_.submitOrder(ask_request);
        active_orders_[order_id] = ask_request;
    }
}

void MarketMakingStrategy::cancelAllOrders() {
    for (const auto& [order_id, request] : active_orders_) {
        order_manager_.cancelOrder(order_id);
    }
    active_orders_.clear();
}

bool MarketMakingStrategy::shouldQuote(const market_data::OrderBook& order_book) const {
    // Check if market data is valid
    if (order_book.getBestBid() == 0 || order_book.getBestAsk() == UINT64_MAX) {
        return false;
    }

    // Check if spread is reasonable
    uint64_t spread = order_book.getSpread();
    if (spread > 100) {  // Arbitrary threshold
        return false;
    }

    return true;
}

uint64_t MarketMakingStrategy::calculateBidPrice(uint64_t mid_price) const {
    double spread_offset = mid_price * params_.spread_bps / 10000.0;
    uint64_t bid_price = mid_price - static_cast<uint64_t>(spread_offset);

    // Round to tick size
    bid_price = (bid_price / static_cast<uint64_t>(params_.tick_size)) *
                static_cast<uint64_t>(params_.tick_size);

    return bid_price;
}

uint64_t MarketMakingStrategy::calculateAskPrice(uint64_t mid_price) const {
    double spread_offset = mid_price * params_.spread_bps / 10000.0;
    uint64_t ask_price = mid_price + static_cast<uint64_t>(spread_offset);

    // Round to tick size
    ask_price = ((ask_price + static_cast<uint64_t>(params_.tick_size) - 1) /
                static_cast<uint64_t>(params_.tick_size)) *
                static_cast<uint64_t>(params_.tick_size);

    return ask_price;
}

uint32_t MarketMakingStrategy::calculateQuoteSize(market_data::Side side) const {
    int64_t current_position = position_manager_.getNetPosition(symbol_);

    // Reduce quote size as position grows
    double position_ratio = static_cast<double>(std::abs(current_position)) /
                           params_.max_position;

    uint32_t size = params_.quote_size;

    if (side == market_data::Side::BUY && current_position > 0) {
        size = static_cast<uint32_t>(size * (1.0 - position_ratio * 0.5));
    } else if (side == market_data::Side::SELL && current_position < 0) {
        size = static_cast<uint32_t>(size * (1.0 - position_ratio * 0.5));
    }

    return std::max(size, 1u);
}

} // namespace strategy
} // namespace hft

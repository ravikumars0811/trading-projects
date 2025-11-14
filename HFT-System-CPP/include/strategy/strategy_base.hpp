#pragma once

#include "market_data/order_book.hpp"
#include "oms/order_manager.hpp"
#include "oms/position_manager.hpp"
#include <string>
#include <memory>

namespace hft {
namespace strategy {

/**
 * Base class for trading strategies
 */
class StrategyBase {
public:
    StrategyBase(const std::string& name,
                oms::OrderManager& order_manager,
                oms::PositionManager& position_manager)
        : name_(name),
          order_manager_(order_manager),
          position_manager_(position_manager),
          enabled_(false) {}

    virtual ~StrategyBase() = default;

    // Strategy lifecycle
    virtual void initialize() = 0;
    virtual void start() { enabled_ = true; }
    virtual void stop() { enabled_ = false; }
    virtual void shutdown() = 0;

    // Market data callback
    virtual void onMarketData(const std::string& symbol,
                             const market_data::OrderBook& order_book) = 0;

    // Order callbacks
    virtual void onOrderUpdate(const oms::OrderState& state) {}
    virtual void onFill(const oms::Fill& fill) {}

    // Configuration
    void setSymbol(const std::string& symbol) { symbol_ = symbol; }
    const std::string& getSymbol() const { return symbol_; }

    const std::string& getName() const { return name_; }
    bool isEnabled() const { return enabled_; }

protected:
    std::string name_;
    std::string symbol_;
    oms::OrderManager& order_manager_;
    oms::PositionManager& position_manager_;
    bool enabled_;
};

} // namespace strategy
} // namespace hft

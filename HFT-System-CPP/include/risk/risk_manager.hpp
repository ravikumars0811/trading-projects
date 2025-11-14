#pragma once

#include "oms/order_manager.hpp"
#include "oms/position_manager.hpp"
#include <unordered_map>
#include <string>

namespace hft {
namespace risk {

/**
 * Risk check results
 */
enum class RiskCheckResult {
    PASS,
    FAIL_POSITION_LIMIT,
    FAIL_ORDER_SIZE,
    FAIL_PRICE_COLLAR,
    FAIL_PNL_LIMIT,
    FAIL_ORDER_RATE
};

/**
 * Risk limits per symbol
 */
struct RiskLimits {
    int64_t max_position = 1000;
    uint32_t max_order_size = 500;
    double max_loss_per_symbol = 10000.0;
    double price_collar_percent = 5.0;  // Maximum price deviation from market
    uint32_t max_orders_per_second = 100;
};

/**
 * Risk Manager
 * Performs pre-trade and post-trade risk checks
 */
class RiskManager {
public:
    explicit RiskManager(oms::PositionManager& position_manager);

    // Risk limit configuration
    void setGlobalRiskLimits(const RiskLimits& limits);
    void setSymbolRiskLimits(const std::string& symbol, const RiskLimits& limits);

    // Pre-trade risk checks
    RiskCheckResult checkOrder(const oms::OrderRequest& request,
                              uint64_t market_price) const;

    // Post-trade risk checks
    void updateOrderMetrics(const std::string& symbol);
    bool checkPnLLimit(const std::string& symbol, double current_price) const;

    // Risk status queries
    bool isPositionLimitBreached(const std::string& symbol) const;
    bool isPnLLimitBreached(const std::string& symbol, double current_price) const;

    const RiskLimits& getRiskLimits(const std::string& symbol) const;

private:
    oms::PositionManager& position_manager_;

    RiskLimits global_limits_;
    std::unordered_map<std::string, RiskLimits> symbol_limits_;

    // Order rate tracking
    mutable std::unordered_map<std::string, uint64_t> last_order_time_;
    mutable std::unordered_map<std::string, uint32_t> order_count_;
};

} // namespace risk
} // namespace hft

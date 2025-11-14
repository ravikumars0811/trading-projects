#include "oms/position_manager.hpp"
#include "core/logger.hpp"
#include <cmath>

namespace hft {
namespace oms {

void PositionManager::updatePosition(const std::string& symbol, int64_t quantity_delta,
                                    double price, uint64_t timestamp) {
    Position& pos = positions_[symbol];

    if (pos.symbol.empty()) {
        pos.symbol = symbol;
    }

    int64_t old_quantity = pos.quantity;
    int64_t new_quantity = old_quantity + quantity_delta;

    // Calculate realized PnL
    if ((old_quantity > 0 && quantity_delta < 0) ||
        (old_quantity < 0 && quantity_delta > 0)) {
        // Position is closing (at least partially)
        int64_t closing_quantity = std::min(std::abs(old_quantity), std::abs(quantity_delta));
        double pnl_per_unit = (price - pos.average_price) * (old_quantity > 0 ? 1 : -1);
        pos.realized_pnl += pnl_per_unit * closing_quantity;
    }

    // Update average price for opening trades
    if ((old_quantity >= 0 && quantity_delta > 0) ||
        (old_quantity <= 0 && quantity_delta < 0)) {
        // Position is opening or adding
        if (old_quantity == 0) {
            pos.average_price = price;
        } else {
            double total_cost = pos.average_price * std::abs(old_quantity) +
                              price * std::abs(quantity_delta);
            pos.average_price = total_cost / std::abs(new_quantity);
        }
    } else if (new_quantity == 0) {
        // Position is fully closed
        pos.average_price = 0.0;
    }

    pos.quantity = new_quantity;
    pos.last_update_time = timestamp;

    LOG_INFO("Position updated: ", symbol,
             " Qty=", pos.quantity,
             " AvgPrice=", pos.average_price,
             " RealizedPnL=", pos.realized_pnl);
}

const Position* PositionManager::getPosition(const std::string& symbol) const {
    auto it = positions_.find(symbol);
    return (it != positions_.end()) ? &it->second : nullptr;
}

std::vector<Position> PositionManager::getAllPositions() const {
    std::vector<Position> all_positions;
    all_positions.reserve(positions_.size());

    for (const auto& [symbol, pos] : positions_) {
        all_positions.push_back(pos);
    }

    return all_positions;
}

int64_t PositionManager::getNetPosition(const std::string& symbol) const {
    auto* pos = getPosition(symbol);
    return pos ? pos->quantity : 0;
}

double PositionManager::getRealizedPnL(const std::string& symbol) const {
    auto* pos = getPosition(symbol);
    return pos ? pos->realized_pnl : 0.0;
}

double PositionManager::getUnrealizedPnL(const std::string& symbol, double current_price) const {
    auto* pos = getPosition(symbol);
    if (!pos || pos->quantity == 0) {
        return 0.0;
    }

    return (current_price - pos->average_price) * pos->quantity;
}

double PositionManager::getTotalPnL(const std::string& symbol, double current_price) const {
    return getRealizedPnL(symbol) + getUnrealizedPnL(symbol, current_price);
}

double PositionManager::getTotalRealizedPnL() const {
    double total = 0.0;
    for (const auto& [symbol, pos] : positions_) {
        total += pos.realized_pnl;
    }
    return total;
}

double PositionManager::getTotalUnrealizedPnL(
    const std::unordered_map<std::string, double>& current_prices) const {

    double total = 0.0;
    for (const auto& [symbol, pos] : positions_) {
        auto price_it = current_prices.find(symbol);
        if (price_it != current_prices.end()) {
            total += getUnrealizedPnL(symbol, price_it->second);
        }
    }
    return total;
}

double PositionManager::getGrossExposure(
    const std::unordered_map<std::string, double>& current_prices) const {

    double exposure = 0.0;
    for (const auto& [symbol, pos] : positions_) {
        auto price_it = current_prices.find(symbol);
        if (price_it != current_prices.end()) {
            exposure += std::abs(pos.quantity * price_it->second);
        }
    }
    return exposure;
}

double PositionManager::getNetExposure(
    const std::unordered_map<std::string, double>& current_prices) const {

    double exposure = 0.0;
    for (const auto& [symbol, pos] : positions_) {
        auto price_it = current_prices.find(symbol);
        if (price_it != current_prices.end()) {
            exposure += pos.quantity * price_it->second;
        }
    }
    return exposure;
}

} // namespace oms
} // namespace hft

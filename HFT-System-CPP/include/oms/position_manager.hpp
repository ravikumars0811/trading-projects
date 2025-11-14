#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace hft {
namespace oms {

/**
 * Position tracking for a single instrument
 */
struct Position {
    std::string symbol;
    int64_t quantity = 0;           // Positive = long, negative = short
    double average_price = 0.0;
    double realized_pnl = 0.0;
    double unrealized_pnl = 0.0;
    uint64_t last_update_time = 0;
};

/**
 * Position Manager
 * Tracks positions and calculates PnL
 */
class PositionManager {
public:
    PositionManager() = default;
    ~PositionManager() = default;

    // Position updates
    void updatePosition(const std::string& symbol, int64_t quantity_delta,
                       double price, uint64_t timestamp);

    // Position queries
    const Position* getPosition(const std::string& symbol) const;
    std::vector<Position> getAllPositions() const;

    int64_t getNetPosition(const std::string& symbol) const;
    double getRealizedPnL(const std::string& symbol) const;
    double getUnrealizedPnL(const std::string& symbol, double current_price) const;
    double getTotalPnL(const std::string& symbol, double current_price) const;

    // Portfolio-level queries
    double getTotalRealizedPnL() const;
    double getTotalUnrealizedPnL(const std::unordered_map<std::string, double>& current_prices) const;

    // Risk metrics
    double getGrossExposure(const std::unordered_map<std::string, double>& current_prices) const;
    double getNetExposure(const std::unordered_map<std::string, double>& current_prices) const;

private:
    void calculateUnrealizedPnL(Position& pos, double current_price) const;

    std::unordered_map<std::string, Position> positions_;
};

} // namespace oms
} // namespace hft

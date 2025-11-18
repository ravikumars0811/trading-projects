#pragma once

#include <string>
#include <chrono>
#include <cstdint>

namespace trading {

// Type aliases for better clarity and performance
using Price = double;
using Quantity = double;
using OrderId = uint64_t;
using Symbol = std::string;
using Timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;

// Enumerations
enum class OrderSide {
    BUY,
    SELL
};

enum class OrderType {
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT,
    ICEBERG,
    TWAP,
    VWAP
};

enum class OrderStatus {
    PENDING,
    SUBMITTED,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED,
    REJECTED,
    EXPIRED
};

enum class TimeInForce {
    DAY,
    GTC,  // Good Till Cancel
    IOC,  // Immediate or Cancel
    FOK   // Fill or Kill
};

enum class StrategyType {
    MOMENTUM,
    MEAN_REVERSION,
    ARBITRAGE,
    MARKET_MAKING,
    ML_BASED,
    CUSTOM
};

// Helper functions
inline std::string to_string(OrderSide side) {
    return side == OrderSide::BUY ? "BUY" : "SELL";
}

inline std::string to_string(OrderType type) {
    switch(type) {
        case OrderType::MARKET: return "MARKET";
        case OrderType::LIMIT: return "LIMIT";
        case OrderType::STOP: return "STOP";
        case OrderType::STOP_LIMIT: return "STOP_LIMIT";
        case OrderType::ICEBERG: return "ICEBERG";
        case OrderType::TWAP: return "TWAP";
        case OrderType::VWAP: return "VWAP";
        default: return "UNKNOWN";
    }
}

inline std::string to_string(OrderStatus status) {
    switch(status) {
        case OrderStatus::PENDING: return "PENDING";
        case OrderStatus::SUBMITTED: return "SUBMITTED";
        case OrderStatus::PARTIALLY_FILLED: return "PARTIALLY_FILLED";
        case OrderStatus::FILLED: return "FILLED";
        case OrderStatus::CANCELLED: return "CANCELLED";
        case OrderStatus::REJECTED: return "REJECTED";
        case OrderStatus::EXPIRED: return "EXPIRED";
        default: return "UNKNOWN";
    }
}

inline Timestamp now() {
    return std::chrono::time_point_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now()
    );
}

} // namespace trading

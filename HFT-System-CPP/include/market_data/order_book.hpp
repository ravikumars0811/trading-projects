#pragma once

#include <array>
#include <vector>
#include <functional>
#include <cstdint>
#include "core/memory_pool.hpp"

namespace hft {
namespace market_data {

enum class Side : uint8_t {
    BUY = 0,
    SELL = 1
};

struct Order {
    uint64_t order_id;
    uint64_t price;  // Price in ticks (e.g., cents)
    uint32_t quantity;
    Side side;
    uint64_t timestamp;

    Order() = default;
    Order(uint64_t id, uint64_t p, uint32_t q, Side s, uint64_t ts)
        : order_id(id), price(p), quantity(q), side(s), timestamp(ts) {}
};

struct Trade {
    uint64_t buy_order_id;
    uint64_t sell_order_id;
    uint64_t price;
    uint32_t quantity;
    uint64_t timestamp;
};

/**
 * Ultra-low-latency order book implementation
 * Uses array-based price levels for O(1) access
 * Optimized for minimal cache misses and branch prediction
 */
class OrderBook {
public:
    using TradeCallback = std::function<void(const Trade&)>;

    explicit OrderBook(uint32_t num_price_levels = 10000);
    ~OrderBook() = default;

    // Order operations
    bool addOrder(const Order& order);
    bool cancelOrder(uint64_t order_id);
    bool modifyOrder(uint64_t order_id, uint32_t new_quantity);

    // Market data queries
    uint64_t getBestBid() const { return best_bid_; }
    uint64_t getBestAsk() const { return best_ask_; }
    uint64_t getMidPrice() const;
    uint64_t getSpread() const;

    uint32_t getBidDepth(size_t levels = 5) const;
    uint32_t getAskDepth(size_t levels = 5) const;

    // Market data snapshot
    struct Level {
        uint64_t price;
        uint32_t quantity;
        uint32_t order_count;
    };

    std::vector<Level> getBids(size_t depth = 10) const;
    std::vector<Level> getAsks(size_t depth = 10) const;

    // Callbacks
    void setTradeCallback(TradeCallback cb) { trade_callback_ = std::move(cb); }

    // Statistics
    uint64_t getTotalVolume() const { return total_volume_; }
    uint64_t getTradeCount() const { return trade_count_; }

private:
    struct OrderNode {
        Order order;
        OrderNode* next = nullptr;
        OrderNode* prev = nullptr;
    };

    struct PriceLevel {
        uint64_t price = 0;
        uint32_t total_quantity = 0;
        uint32_t order_count = 0;
        OrderNode* head = nullptr;
        OrderNode* tail = nullptr;
    };

    void matchOrder(Order& order);
    void addToBook(const Order& order);
    void removeFromBook(OrderNode* node, PriceLevel* level);
    PriceLevel* getPriceLevel(uint64_t price, Side side);
    void updateBestPrices();

    void executeTrade(OrderNode* passive_node, Order& aggressive_order, uint32_t trade_qty);

    static constexpr uint32_t MAX_PRICE_LEVELS = 20000;
    std::array<PriceLevel, MAX_PRICE_LEVELS> bid_levels_;
    std::array<PriceLevel, MAX_PRICE_LEVELS> ask_levels_;

    uint64_t best_bid_ = 0;
    uint64_t best_ask_ = UINT64_MAX;

    uint64_t base_price_ = 10000;  // Base price for indexing
    uint32_t num_price_levels_;

    core::MemoryPool<OrderNode> order_pool_;
    std::unordered_map<uint64_t, OrderNode*> order_map_;

    TradeCallback trade_callback_;
    uint64_t total_volume_ = 0;
    uint64_t trade_count_ = 0;
};

} // namespace market_data
} // namespace hft

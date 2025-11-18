#pragma once

#include "types.hpp"
#include <atomic>
#include <memory>

namespace trading {

struct Order {
    OrderId order_id;
    Symbol symbol;
    OrderSide side;
    OrderType type;
    Price price;
    Quantity quantity;
    Quantity filled_quantity;
    OrderStatus status;
    TimeInForce time_in_force;
    Timestamp created_at;
    Timestamp updated_at;
    std::string strategy_id;
    std::string client_order_id;

    // For advanced order types
    Price stop_price{0.0};
    Price limit_price{0.0};
    Quantity display_quantity{0.0}; // For iceberg orders

    Order() = default;

    Order(const Symbol& sym, OrderSide sd, OrderType tp, Price pr, Quantity qty,
          TimeInForce tif = TimeInForce::DAY, const std::string& strat_id = "")
        : order_id(0), symbol(sym), side(sd), type(tp), price(pr),
          quantity(qty), filled_quantity(0.0), status(OrderStatus::PENDING),
          time_in_force(tif), created_at(now()), updated_at(now()),
          strategy_id(strat_id) {}

    bool is_complete() const {
        return status == OrderStatus::FILLED ||
               status == OrderStatus::CANCELLED ||
               status == OrderStatus::REJECTED ||
               status == OrderStatus::EXPIRED;
    }

    bool is_active() const {
        return !is_complete();
    }

    Quantity remaining_quantity() const {
        return quantity - filled_quantity;
    }
};

struct Trade {
    uint64_t trade_id;
    OrderId order_id;
    Symbol symbol;
    OrderSide side;
    Price price;
    Quantity quantity;
    Timestamp timestamp;
    std::string exchange;
    double commission{0.0};

    Trade() = default;

    Trade(OrderId oid, const Symbol& sym, OrderSide sd, Price pr, Quantity qty)
        : trade_id(0), order_id(oid), symbol(sym), side(sd),
          price(pr), quantity(qty), timestamp(now()) {}
};

struct MarketData {
    Symbol symbol;
    Price bid;
    Price ask;
    Quantity bid_size;
    Quantity ask_size;
    Price last_price;
    Quantity last_size;
    uint64_t volume;
    Timestamp timestamp;

    // Level 2 data (order book)
    struct Level {
        Price price;
        Quantity quantity;
        uint32_t num_orders;
    };

    std::vector<Level> bid_levels;
    std::vector<Level> ask_levels;

    MarketData() = default;

    Price mid_price() const {
        return (bid + ask) / 2.0;
    }

    Price spread() const {
        return ask - bid;
    }

    double spread_bps() const {
        return (spread() / mid_price()) * 10000.0;
    }
};

struct Position {
    Symbol symbol;
    Quantity quantity;  // Positive for long, negative for short
    Price avg_price;
    double realized_pnl;
    double unrealized_pnl;
    Timestamp last_update;

    Position() : quantity(0.0), avg_price(0.0), realized_pnl(0.0),
                 unrealized_pnl(0.0), last_update(now()) {}

    Position(const Symbol& sym, Quantity qty, Price price)
        : symbol(sym), quantity(qty), avg_price(price),
          realized_pnl(0.0), unrealized_pnl(0.0), last_update(now()) {}

    void update_with_trade(const Trade& trade) {
        if (trade.side == OrderSide::BUY) {
            if (quantity >= 0) {
                // Adding to long position
                avg_price = (avg_price * quantity + trade.price * trade.quantity) /
                           (quantity + trade.quantity);
                quantity += trade.quantity;
            } else {
                // Closing short position
                realized_pnl += (avg_price - trade.price) * std::min(trade.quantity, -quantity);
                quantity += trade.quantity;
                if (quantity > 0) {
                    avg_price = trade.price;
                }
            }
        } else {
            if (quantity <= 0) {
                // Adding to short position
                avg_price = (avg_price * -quantity + trade.price * trade.quantity) /
                           (-quantity + trade.quantity);
                quantity -= trade.quantity;
            } else {
                // Closing long position
                realized_pnl += (trade.price - avg_price) * std::min(trade.quantity, quantity);
                quantity -= trade.quantity;
                if (quantity < 0) {
                    avg_price = trade.price;
                }
            }
        }
        last_update = now();
    }

    void update_unrealized_pnl(Price current_price) {
        if (quantity > 0) {
            unrealized_pnl = (current_price - avg_price) * quantity;
        } else if (quantity < 0) {
            unrealized_pnl = (avg_price - current_price) * -quantity;
        } else {
            unrealized_pnl = 0.0;
        }
    }

    double total_pnl(Price current_price) {
        update_unrealized_pnl(current_price);
        return realized_pnl + unrealized_pnl;
    }
};

} // namespace trading

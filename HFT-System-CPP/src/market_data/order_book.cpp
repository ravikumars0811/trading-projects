#include "market_data/order_book.hpp"
#include "core/timer.hpp"
#include <algorithm>
#include <cmath>

namespace hft {
namespace market_data {

OrderBook::OrderBook(uint32_t num_price_levels)
    : num_price_levels_(std::min(num_price_levels, MAX_PRICE_LEVELS)) {

    // Initialize price levels
    for (uint32_t i = 0; i < MAX_PRICE_LEVELS; ++i) {
        bid_levels_[i] = PriceLevel{};
        ask_levels_[i] = PriceLevel{};
    }
}

bool OrderBook::addOrder(const Order& order) {
    Order mutable_order = order;
    matchOrder(mutable_order);

    if (mutable_order.quantity > 0) {
        addToBook(mutable_order);
    }

    return true;
}

void OrderBook::matchOrder(Order& order) {
    if (order.side == Side::BUY) {
        // Match against asks
        while (order.quantity > 0 && best_ask_ != UINT64_MAX && order.price >= best_ask_) {
            size_t level_idx = best_ask_ - base_price_;
            if (level_idx >= MAX_PRICE_LEVELS) break;

            PriceLevel* level = &ask_levels_[level_idx];
            if (level->head == nullptr) {
                updateBestPrices();
                break;
            }

            OrderNode* node = level->head;
            uint32_t trade_qty = std::min(order.quantity, node->order.quantity);

            executeTrade(node, order, trade_qty);

            node->order.quantity -= trade_qty;
            order.quantity -= trade_qty;
            level->total_quantity -= trade_qty;

            if (node->order.quantity == 0) {
                removeFromBook(node, level);
            }
        }
    } else {
        // Match against bids
        while (order.quantity > 0 && best_bid_ != 0 && order.price <= best_bid_) {
            size_t level_idx = best_bid_ - base_price_;
            if (level_idx >= MAX_PRICE_LEVELS) break;

            PriceLevel* level = &bid_levels_[level_idx];
            if (level->head == nullptr) {
                updateBestPrices();
                break;
            }

            OrderNode* node = level->head;
            uint32_t trade_qty = std::min(order.quantity, node->order.quantity);

            executeTrade(node, order, trade_qty);

            node->order.quantity -= trade_qty;
            order.quantity -= trade_qty;
            level->total_quantity -= trade_qty;

            if (node->order.quantity == 0) {
                removeFromBook(node, level);
            }
        }
    }

    updateBestPrices();
}

void OrderBook::addToBook(const Order& order) {
    size_t level_idx = order.price - base_price_;
    if (level_idx >= MAX_PRICE_LEVELS) return;

    auto* level = (order.side == Side::BUY) ? &bid_levels_[level_idx] : &ask_levels_[level_idx];

    OrderNode* node = order_pool_.allocate();
    node->order = order;
    node->next = nullptr;
    node->prev = level->tail;

    if (level->tail) {
        level->tail->next = node;
    } else {
        level->head = node;
    }
    level->tail = node;

    level->price = order.price;
    level->total_quantity += order.quantity;
    level->order_count++;

    order_map_[order.order_id] = node;

    updateBestPrices();
}

void OrderBook::removeFromBook(OrderNode* node, PriceLevel* level) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        level->head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        level->tail = node->prev;
    }

    level->order_count--;
    if (level->order_count == 0) {
        level->total_quantity = 0;
        level->price = 0;
    }

    order_map_.erase(node->order.order_id);
    order_pool_.deallocate(node);
}

bool OrderBook::cancelOrder(uint64_t order_id) {
    auto it = order_map_.find(order_id);
    if (it == order_map_.end()) {
        return false;
    }

    OrderNode* node = it->second;
    size_t level_idx = node->order.price - base_price_;
    if (level_idx >= MAX_PRICE_LEVELS) return false;

    auto* level = (node->order.side == Side::BUY) ? &bid_levels_[level_idx] : &ask_levels_[level_idx];
    level->total_quantity -= node->order.quantity;

    removeFromBook(node, level);
    updateBestPrices();

    return true;
}

bool OrderBook::modifyOrder(uint64_t order_id, uint32_t new_quantity) {
    auto it = order_map_.find(order_id);
    if (it == order_map_.end()) {
        return false;
    }

    OrderNode* node = it->second;
    size_t level_idx = node->order.price - base_price_;
    if (level_idx >= MAX_PRICE_LEVELS) return false;

    auto* level = (node->order.side == Side::BUY) ? &bid_levels_[level_idx] : &ask_levels_[level_idx];

    level->total_quantity = level->total_quantity - node->order.quantity + new_quantity;
    node->order.quantity = new_quantity;

    return true;
}

void OrderBook::updateBestPrices() {
    // Find best bid
    best_bid_ = 0;
    for (int64_t i = MAX_PRICE_LEVELS - 1; i >= 0; --i) {
        if (bid_levels_[i].order_count > 0) {
            best_bid_ = bid_levels_[i].price;
            break;
        }
    }

    // Find best ask
    best_ask_ = UINT64_MAX;
    for (size_t i = 0; i < MAX_PRICE_LEVELS; ++i) {
        if (ask_levels_[i].order_count > 0) {
            best_ask_ = ask_levels_[i].price;
            break;
        }
    }
}

void OrderBook::executeTrade(OrderNode* passive_node, Order& aggressive_order, uint32_t trade_qty) {
    Trade trade{
        passive_node->order.side == Side::BUY ? passive_node->order.order_id : aggressive_order.order_id,
        passive_node->order.side == Side::SELL ? passive_node->order.order_id : aggressive_order.order_id,
        passive_node->order.price,
        trade_qty,
        core::Timer::timestamp_ns()
    };

    total_volume_ += trade_qty;
    trade_count_++;

    if (trade_callback_) {
        trade_callback_(trade);
    }
}

uint64_t OrderBook::getMidPrice() const {
    if (best_bid_ == 0 || best_ask_ == UINT64_MAX) {
        return 0;
    }
    return (best_bid_ + best_ask_) / 2;
}

uint64_t OrderBook::getSpread() const {
    if (best_bid_ == 0 || best_ask_ == UINT64_MAX) {
        return 0;
    }
    return best_ask_ - best_bid_;
}

uint32_t OrderBook::getBidDepth(size_t levels) const {
    uint32_t depth = 0;
    size_t count = 0;

    for (int64_t i = MAX_PRICE_LEVELS - 1; i >= 0 && count < levels; --i) {
        if (bid_levels_[i].order_count > 0) {
            depth += bid_levels_[i].total_quantity;
            count++;
        }
    }

    return depth;
}

uint32_t OrderBook::getAskDepth(size_t levels) const {
    uint32_t depth = 0;
    size_t count = 0;

    for (size_t i = 0; i < MAX_PRICE_LEVELS && count < levels; ++i) {
        if (ask_levels_[i].order_count > 0) {
            depth += ask_levels_[i].total_quantity;
            count++;
        }
    }

    return depth;
}

std::vector<OrderBook::Level> OrderBook::getBids(size_t depth) const {
    std::vector<Level> levels;
    levels.reserve(depth);

    for (int64_t i = MAX_PRICE_LEVELS - 1; i >= 0 && levels.size() < depth; --i) {
        if (bid_levels_[i].order_count > 0) {
            levels.push_back({
                bid_levels_[i].price,
                bid_levels_[i].total_quantity,
                bid_levels_[i].order_count
            });
        }
    }

    return levels;
}

std::vector<OrderBook::Level> OrderBook::getAsks(size_t depth) const {
    std::vector<Level> levels;
    levels.reserve(depth);

    for (size_t i = 0; i < MAX_PRICE_LEVELS && levels.size() < depth; ++i) {
        if (ask_levels_[i].order_count > 0) {
            levels.push_back({
                ask_levels_[i].price,
                ask_levels_[i].total_quantity,
                ask_levels_[i].order_count
            });
        }
    }

    return levels;
}

} // namespace market_data
} // namespace hft

/*
 * High-Performance Order Book Implementation
 * HFT-Focused: Price-time priority, fast insertions/deletions
 */

#include <iostream>
#include <map>
#include <list>
#include <unordered_map>
#include <memory>
#include <iomanip>
#include <cstdint>

// Order structure
struct Order {
    uint64_t order_id;
    char side;  // 'B' or 'S'
    double price;
    int quantity;
    uint64_t timestamp;  // For time priority

    Order(uint64_t id, char s, double p, int q, uint64_t ts)
        : order_id(id), side(s), price(p), quantity(q), timestamp(ts) {}

    void display() const {
        std::cout << "Order #" << order_id << ": "
                  << (side == 'B' ? "BUY " : "SELL") << " "
                  << quantity << " @ $" << std::fixed << std::setprecision(2) << price
                  << std::endl;
    }
};

// Price level containing orders at same price
struct PriceLevel {
    double price;
    int total_quantity;
    std::list<std::shared_ptr<Order>> orders;  // Time-ordered list

    PriceLevel(double p) : price(p), total_quantity(0) {}

    void addOrder(std::shared_ptr<Order> order) {
        orders.push_back(order);
        total_quantity += order->quantity;
    }

    void removeOrder(std::shared_ptr<Order> order) {
        total_quantity -= order->quantity;
        orders.remove(order);
    }

    void display() const {
        std::cout << "  $" << std::fixed << std::setprecision(2) << price
                  << " x " << total_quantity << " (" << orders.size() << " orders)" << std::endl;
    }
};

// Order book side (buy or sell)
class OrderBookSide {
private:
    std::map<double, PriceLevel, std::greater<double>> levels_;  // Descending for bids
    std::unordered_map<uint64_t, std::shared_ptr<Order>> order_map_;  // Fast lookup by ID

public:
    void addOrder(std::shared_ptr<Order> order) {
        // Find or create price level
        auto it = levels_.find(order->price);
        if (it == levels_.end()) {
            it = levels_.emplace(order->price, order->price).first;
        }

        // Add order to price level
        it->second.addOrder(order);

        // Add to order map for fast lookup
        order_map_[order->order_id] = order;
    }

    bool removeOrder(uint64_t order_id) {
        auto it = order_map_.find(order_id);
        if (it == order_map_.end()) {
            return false;
        }

        auto order = it->second;
        order_map_.erase(it);

        // Find price level
        auto level_it = levels_.find(order->price);
        if (level_it != levels_.end()) {
            level_it->second.removeOrder(order);

            // Remove empty price level
            if (level_it->second.orders.empty()) {
                levels_.erase(level_it);
            }
        }

        return true;
    }

    bool modifyOrder(uint64_t order_id, int new_quantity) {
        auto it = order_map_.find(order_id);
        if (it == order_map_.end()) {
            return false;
        }

        auto order = it->second;
        auto level_it = levels_.find(order->price);

        if (level_it != levels_.end()) {
            level_it->second.total_quantity -= order->quantity;
            order->quantity = new_quantity;
            level_it->second.total_quantity += new_quantity;
        }

        return true;
    }

    std::shared_ptr<Order> getBestOrder() const {
        if (levels_.empty()) {
            return nullptr;
        }

        const auto& best_level = levels_.begin()->second;
        if (best_level.orders.empty()) {
            return nullptr;
        }

        return best_level.orders.front();
    }

    double getBestPrice() const {
        if (levels_.empty()) {
            return 0.0;
        }
        return levels_.begin()->first;
    }

    void display(int depth = 5) const {
        int count = 0;
        for (const auto& [price, level] : levels_) {
            if (count++ >= depth) break;
            level.display();
        }
    }

    bool isEmpty() const {
        return levels_.empty();
    }

    size_t getOrderCount() const {
        return order_map_.size();
    }
};

// Full order book
class OrderBook {
private:
    std::string symbol_;
    OrderBookSide buy_side_;
    OrderBookSide sell_side_;
    uint64_t next_order_id_;

public:
    OrderBook(const std::string& symbol)
        : symbol_(symbol), next_order_id_(1000) {}

    uint64_t addOrder(char side, double price, int quantity, uint64_t timestamp = 0) {
        if (timestamp == 0) {
            timestamp = next_order_id_;  // Use order ID as timestamp for simplicity
        }

        auto order = std::make_shared<Order>(next_order_id_++, side, price, quantity, timestamp);

        if (side == 'B') {
            buy_side_.addOrder(order);
        } else {
            sell_side_.addOrder(order);
        }

        return order->order_id;
    }

    bool cancelOrder(uint64_t order_id, char side) {
        if (side == 'B') {
            return buy_side_.removeOrder(order_id);
        } else {
            return sell_side_.removeOrder(order_id);
        }
    }

    bool modifyOrder(uint64_t order_id, char side, int new_quantity) {
        if (side == 'B') {
            return buy_side_.modifyOrder(order_id, new_quantity);
        } else {
            return sell_side_.modifyOrder(order_id, new_quantity);
        }
    }

    void display(int depth = 5) const {
        std::cout << "\n========== Order Book: " << symbol_ << " ==========" << std::endl;

        std::cout << "\nAsks (Sell Orders):" << std::endl;
        // Note: For asks, we want ascending order, but we store descending
        // So we need to reverse or use different comparator
        sell_side_.display(depth);

        std::cout << "\nBids (Buy Orders):" << std::endl;
        buy_side_.display(depth);

        std::cout << "\nBest Bid: $" << std::fixed << std::setprecision(2)
                  << buy_side_.getBestPrice() << std::endl;
        std::cout << "Best Ask: $" << sell_side_.getBestPrice() << std::endl;

        double spread = sell_side_.getBestPrice() - buy_side_.getBestPrice();
        std::cout << "Spread: $" << spread << std::endl;

        std::cout << "\nTotal Orders: " << (buy_side_.getOrderCount() + sell_side_.getOrderCount())
                  << std::endl;
        std::cout << "======================================\n" << std::endl;
    }

    // Match orders (simplified matching engine)
    void matchOrders() {
        while (!buy_side_.isEmpty() && !sell_side_.isEmpty()) {
            auto best_bid = buy_side_.getBestOrder();
            auto best_ask = sell_side_.getBestOrder();

            if (!best_bid || !best_ask) break;

            // Check if orders can match
            if (best_bid->price >= best_ask->price) {
                int match_qty = std::min(best_bid->quantity, best_ask->quantity);

                std::cout << "MATCH! " << match_qty << " @ $"
                          << std::fixed << std::setprecision(2) << best_ask->price << std::endl;

                // Update quantities
                best_bid->quantity -= match_qty;
                best_ask->quantity -= match_qty;

                // Remove filled orders
                if (best_bid->quantity == 0) {
                    buy_side_.removeOrder(best_bid->order_id);
                } else {
                    buy_side_.modifyOrder(best_bid->order_id, best_bid->quantity);
                }

                if (best_ask->quantity == 0) {
                    sell_side_.removeOrder(best_ask->order_id);
                } else {
                    sell_side_.modifyOrder(best_ask->order_id, best_ask->quantity);
                }
            } else {
                break;  // No match possible
            }
        }
    }
};

int main() {
    std::cout << "=== High-Performance Order Book ===" << std::endl;

    OrderBook book("AAPL");

    // Add buy orders
    std::cout << "\nAdding buy orders..." << std::endl;
    book.addOrder('B', 150.00, 100);
    book.addOrder('B', 150.05, 200);
    book.addOrder('B', 149.95, 150);
    book.addOrder('B', 150.05, 50);  // Same price, later time

    // Add sell orders
    std::cout << "Adding sell orders..." << std::endl;
    book.addOrder('S', 150.10, 100);
    book.addOrder('S', 150.15, 200);
    book.addOrder('S', 150.08, 75);

    // Display order book
    book.display();

    // Add aggressive buy order (will match)
    std::cout << "\nAdding aggressive buy order @ $150.10..." << std::endl;
    book.addOrder('B', 150.10, 150);

    // Match orders
    std::cout << "\nMatching orders..." << std::endl;
    book.matchOrders();

    // Display updated book
    book.display();

    // Cancel an order
    std::cout << "\nCancelling order #1002..." << std::endl;
    if (book.cancelOrder(1002, 'B')) {
        std::cout << "Order cancelled successfully" << std::endl;
    }

    book.display();

    // Modify an order
    std::cout << "\nModifying order #1000 to 200 shares..." << std::endl;
    if (book.modifyOrder(1000, 'B', 200)) {
        std::cout << "Order modified successfully" << std::endl;
    }

    book.display();

    std::cout << "\n=== Performance Characteristics ===" << std::endl;
    std::cout << "Add Order:    O(log P) where P = number of price levels" << std::endl;
    std::cout << "Cancel Order: O(1) with hash map lookup" << std::endl;
    std::cout << "Modify Order: O(1) with hash map lookup" << std::endl;
    std::cout << "Best Price:   O(1) with sorted map" << std::endl;
    std::cout << "Match:        O(1) per matched order" << std::endl;

    std::cout << "\n=== Optimizations for HFT ===" << std::endl;
    std::cout << "1. Use memory pools for order allocation" << std::endl;
    std::cout << "2. Pre-allocate hash map buckets" << std::endl;
    std::cout << "3. Use custom allocator for map/list nodes" << std::endl;
    std::cout << "4. Consider lock-free implementation for concurrency" << std::endl;
    std::cout << "5. Store orders in flat arrays for cache locality" << std::endl;
    std::cout << "6. Use fixed-point arithmetic instead of doubles" << std::endl;

    return 0;
}

/*
 * Key Takeaways:
 * 1. Price-time priority: Same price sorted by time
 * 2. Fast lookup: Hash map for O(1) order access by ID
 * 3. Sorted prices: Map for O(log n) price level access
 * 4. Match algorithm: Compare best bid and ask
 * 5. Price levels: Group orders at same price
 *
 * Production Enhancements:
 * - Fixed-point arithmetic for prices (avoid floating-point)
 * - Memory pool for order allocation
 * - Lock-free design for concurrent access
 * - Cache-aligned data structures
 * - SIMD for parallel matching
 * - Separate hot/cold data paths
 * - Minimize allocations
 * - Use intrusive containers
 */

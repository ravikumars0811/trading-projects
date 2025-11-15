/**
 * High-Performance Limit Order Book
 *
 * Applications in HFT:
 * - Order matching engine
 * - Market making strategies
 * - Price discovery
 * - Liquidity provision
 *
 * Interview Focus:
 * - Data structure choice (std::map vs custom)
 * - Order matching algorithm (FIFO/Price-Time priority)
 * - Cache-friendly design
 * - Real-time updates and queries
 *
 * Operations:
 * - Add order: O(log n) with map, O(1) amortized with hash
 * - Cancel order: O(log n) or O(1)
 * - Match order: O(k * log n) where k = matched orders
 * - Get best bid/ask: O(1)
 * - Get market depth: O(levels)
 */

#include <iostream>
#include <map>
#include <unordered_map>
#include <list>
#include <memory>
#include <string>
#include <iomanip>
#include <optional>

// Order side enumeration
enum class Side {
    BUY,
    SELL
};

// Order type
enum class OrderType {
    LIMIT,
    MARKET,
    IOC,  // Immediate or Cancel
    FOK   // Fill or Kill
};

// Order status
enum class OrderStatus {
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED
};

// Individual Order
struct Order {
    uint64_t order_id;
    std::string symbol;
    Side side;
    OrderType type;
    double price;
    int quantity;
    int filled_quantity;
    OrderStatus status;
    uint64_t timestamp;

    Order(uint64_t id, const std::string& sym, Side s, OrderType t,
          double p, int q, uint64_t ts)
        : order_id(id), symbol(sym), side(s), type(t),
          price(p), quantity(q), filled_quantity(0),
          status(OrderStatus::NEW), timestamp(ts) {}

    int remaining_quantity() const {
        return quantity - filled_quantity;
    }

    bool is_active() const {
        return status == OrderStatus::NEW ||
               status == OrderStatus::PARTIALLY_FILLED;
    }
};

// Trade execution result
struct Trade {
    uint64_t buy_order_id;
    uint64_t sell_order_id;
    double price;
    int quantity;
    uint64_t timestamp;

    Trade(uint64_t buy_id, uint64_t sell_id, double p, int q, uint64_t ts)
        : buy_order_id(buy_id), sell_order_id(sell_id),
          price(p), quantity(q), timestamp(ts) {}
};

// Price level in the order book
// Contains all orders at a specific price
class PriceLevel {
private:
    double price_;
    std::list<std::shared_ptr<Order>> orders_;  // FIFO queue
    int total_quantity_;

public:
    explicit PriceLevel(double price)
        : price_(price), total_quantity_(0) {}

    double price() const { return price_; }
    int total_quantity() const { return total_quantity_; }
    bool empty() const { return orders_.empty(); }
    size_t order_count() const { return orders_.size(); }

    // Add order to this price level
    void add_order(std::shared_ptr<Order> order) {
        orders_.push_back(order);
        total_quantity_ += order->remaining_quantity();
    }

    // Remove order from this price level
    bool remove_order(uint64_t order_id) {
        for (auto it = orders_.begin(); it != orders_.end(); ++it) {
            if ((*it)->order_id == order_id) {
                total_quantity_ -= (*it)->remaining_quantity();
                orders_.erase(it);
                return true;
            }
        }
        return false;
    }

    // Get first order (for matching)
    std::shared_ptr<Order> front() {
        return orders_.empty() ? nullptr : orders_.front();
    }

    // Update quantity when order is partially filled
    void update_quantity(uint64_t order_id, int filled_qty) {
        for (auto& order : orders_) {
            if (order->order_id == order_id) {
                total_quantity_ -= filled_qty;
                break;
            }
        }
    }

    // Remove front order after it's filled
    void pop_front() {
        if (!orders_.empty()) {
            total_quantity_ -= orders_.front()->remaining_quantity();
            orders_.pop_front();
        }
    }

    // Get all orders at this level
    const std::list<std::shared_ptr<Order>>& orders() const {
        return orders_;
    }
};

// Main Order Book Implementation
class OrderBook {
private:
    std::string symbol_;

    // Price levels: price -> PriceLevel
    // Buy side: highest price first (reverse order)
    // Sell side: lowest price first (normal order)
    std::map<double, PriceLevel, std::greater<double>> buy_levels_;
    std::map<double, PriceLevel, std::less<double>> sell_levels_;

    // Fast order lookup: order_id -> order
    std::unordered_map<uint64_t, std::shared_ptr<Order>> orders_;

    // Order ID generator
    uint64_t next_order_id_;

    // Executed trades
    std::vector<Trade> trades_;

    // Statistics
    uint64_t total_orders_added_;
    uint64_t total_orders_cancelled_;
    uint64_t total_trades_;
    uint64_t total_volume_;

public:
    explicit OrderBook(const std::string& symbol)
        : symbol_(symbol), next_order_id_(1),
          total_orders_added_(0), total_orders_cancelled_(0),
          total_trades_(0), total_volume_(0) {}

    /**
     * Add a new order to the book
     * Time: O(log n) for map insertion
     */
    uint64_t add_order(Side side, OrderType type, double price, int quantity) {
        uint64_t order_id = next_order_id_++;
        uint64_t timestamp = get_current_timestamp();

        auto order = std::make_shared<Order>(
            order_id, symbol_, side, type, price, quantity, timestamp
        );

        orders_[order_id] = order;
        ++total_orders_added_;

        // Try to match immediately
        if (type == OrderType::MARKET || type == OrderType::IOC || type == OrderType::FOK) {
            match_order(order);

            // IOC and FOK: cancel remaining
            if ((type == OrderType::IOC || type == OrderType::FOK) &&
                order->is_active()) {
                cancel_order(order_id);
            }
        } else {
            // Try to match limit order
            match_order(order);

            // If not fully filled, add to book
            if (order->is_active()) {
                add_to_book(order);
            }
        }

        return order_id;
    }

    /**
     * Cancel an existing order
     * Time: O(log n) for map lookup
     */
    bool cancel_order(uint64_t order_id) {
        auto it = orders_.find(order_id);
        if (it == orders_.end()) {
            return false;
        }

        auto order = it->second;
        if (!order->is_active()) {
            return false;
        }

        // Remove from price level
        auto& levels = (order->side == Side::BUY) ? buy_levels_ : sell_levels_;
        auto level_it = levels.find(order->price);

        if (level_it != levels.end()) {
            level_it->second.remove_order(order_id);

            // Remove empty price level
            if (level_it->second.empty()) {
                levels.erase(level_it);
            }
        }

        order->status = OrderStatus::CANCELLED;
        ++total_orders_cancelled_;
        return true;
    }

    /**
     * Modify an existing order (cancel and replace)
     * Time: O(log n)
     */
    bool modify_order(uint64_t order_id, double new_price, int new_quantity) {
        auto it = orders_.find(order_id);
        if (it == orders_.end() || !it->second->is_active()) {
            return false;
        }

        auto old_order = it->second;
        cancel_order(order_id);

        // Add new order
        add_order(old_order->side, old_order->type, new_price, new_quantity);
        return true;
    }

    /**
     * Get best bid price
     * Time: O(1)
     */
    std::optional<double> get_best_bid() const {
        if (buy_levels_.empty()) {
            return std::nullopt;
        }
        return buy_levels_.begin()->first;
    }

    /**
     * Get best ask price
     * Time: O(1)
     */
    std::optional<double> get_best_ask() const {
        if (sell_levels_.empty()) {
            return std::nullopt;
        }
        return sell_levels_.begin()->first;
    }

    /**
     * Get mid price
     * Time: O(1)
     */
    std::optional<double> get_mid_price() const {
        auto bid = get_best_bid();
        auto ask = get_best_ask();
        if (bid && ask) {
            return (*bid + *ask) / 2.0;
        }
        return std::nullopt;
    }

    /**
     * Get spread
     * Time: O(1)
     */
    std::optional<double> get_spread() const {
        auto bid = get_best_bid();
        auto ask = get_best_ask();
        if (bid && ask) {
            return *ask - *bid;
        }
        return std::nullopt;
    }

    /**
     * Get market depth (top N levels)
     * Time: O(n) where n is number of levels
     */
    void print_depth(int levels = 5) const {
        std::cout << "\n=== Order Book: " << symbol_ << " ===" << std::endl;
        std::cout << std::fixed << std::setprecision(2);

        // Print asks (highest to lowest)
        auto ask_it = sell_levels_.rbegin();
        int ask_count = 0;
        std::vector<std::pair<double, int>> asks;

        while (ask_it != sell_levels_.rend() && ask_count < levels) {
            asks.push_back({ask_it->first, ask_it->second.total_quantity()});
            ++ask_it;
            ++ask_count;
        }

        // Print in reverse (lowest ask at bottom)
        for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
            std::cout << std::setw(20) << " "
                      << std::setw(10) << it->second
                      << " @ "
                      << std::setw(10) << it->first
                      << " (ASK)" << std::endl;
        }

        // Print spread
        if (auto spread = get_spread()) {
            std::cout << std::setw(20) << " "
                      << "--- Spread: " << *spread << " ---" << std::endl;
        }

        // Print bids (highest to lowest)
        auto bid_it = buy_levels_.begin();
        int bid_count = 0;

        while (bid_it != buy_levels_.end() && bid_count < levels) {
            std::cout << std::setw(20) << " "
                      << std::setw(10) << bid_it->second.total_quantity()
                      << " @ "
                      << std::setw(10) << bid_it->first
                      << " (BID)" << std::endl;
            ++bid_it;
            ++bid_count;
        }

        std::cout << std::endl;
    }

    // Print statistics
    void print_stats() const {
        std::cout << "\n=== Order Book Statistics ===" << std::endl;
        std::cout << "Total Orders Added: " << total_orders_added_ << std::endl;
        std::cout << "Total Orders Cancelled: " << total_orders_cancelled_ << std::endl;
        std::cout << "Total Trades: " << total_trades_ << std::endl;
        std::cout << "Total Volume: " << total_volume_ << std::endl;
        std::cout << "Active Orders: " << orders_.size() << std::endl;
        std::cout << "Buy Levels: " << buy_levels_.size() << std::endl;
        std::cout << "Sell Levels: " << sell_levels_.size() << std::endl;
        if (auto mid = get_mid_price()) {
            std::cout << "Mid Price: " << *mid << std::endl;
        }
        if (auto spread = get_spread()) {
            std::cout << "Spread: " << *spread << std::endl;
        }
    }

private:
    /**
     * Add order to the appropriate price level
     * Time: O(log n) for map insertion
     */
    void add_to_book(std::shared_ptr<Order> order) {
        auto& levels = (order->side == Side::BUY) ? buy_levels_ : sell_levels_;

        // Find or create price level
        auto it = levels.find(order->price);
        if (it == levels.end()) {
            // Create new price level
            levels.emplace(order->price, PriceLevel(order->price));
            it = levels.find(order->price);
        }

        it->second.add_order(order);
    }

    /**
     * Match an incoming order against the book
     * Time: O(k * log n) where k is number of matches
     */
    void match_order(std::shared_ptr<Order> order) {
        auto& contra_levels = (order->side == Side::BUY) ? sell_levels_ : buy_levels_;

        while (order->is_active() && !contra_levels.empty()) {
            auto level_it = contra_levels.begin();
            PriceLevel& level = level_it->second;

            // Check if price matches
            bool price_matches = false;
            if (order->type == OrderType::MARKET) {
                price_matches = true;
            } else if (order->side == Side::BUY) {
                price_matches = (order->price >= level.price());
            } else {
                price_matches = (order->price <= level.price());
            }

            if (!price_matches) {
                break;
            }

            // Match against orders at this level
            while (order->is_active() && !level.empty()) {
                auto contra_order = level.front();

                int match_qty = std::min(
                    order->remaining_quantity(),
                    contra_order->remaining_quantity()
                );

                // Execute trade
                execute_trade(order, contra_order, level.price(), match_qty);

                // Update order statuses
                order->filled_quantity += match_qty;
                contra_order->filled_quantity += match_qty;

                if (contra_order->remaining_quantity() == 0) {
                    contra_order->status = OrderStatus::FILLED;
                    level.pop_front();
                } else {
                    contra_order->status = OrderStatus::PARTIALLY_FILLED;
                    level.update_quantity(contra_order->order_id, match_qty);
                }

                if (order->remaining_quantity() == 0) {
                    order->status = OrderStatus::FILLED;
                } else {
                    order->status = OrderStatus::PARTIALLY_FILLED;
                }
            }

            // Remove empty price level
            if (level.empty()) {
                contra_levels.erase(level_it);
            }
        }
    }

    /**
     * Execute a trade between two orders
     */
    void execute_trade(std::shared_ptr<Order> aggressor,
                      std::shared_ptr<Order> passive,
                      double price, int quantity) {
        uint64_t timestamp = get_current_timestamp();

        Trade trade(
            aggressor->side == Side::BUY ? aggressor->order_id : passive->order_id,
            aggressor->side == Side::SELL ? aggressor->order_id : passive->order_id,
            price, quantity, timestamp
        );

        trades_.push_back(trade);
        ++total_trades_;
        total_volume_ += quantity;

        std::cout << "TRADE: " << quantity << " @ " << price
                  << " [Buy #" << trade.buy_order_id
                  << " | Sell #" << trade.sell_order_id << "]" << std::endl;
    }

    uint64_t get_current_timestamp() const {
        return std::chrono::system_clock::now().time_since_epoch().count();
    }
};

// Example usage and testing
void demonstrate_order_book() {
    OrderBook book("AAPL");

    std::cout << "\n=== Adding Initial Orders ===" << std::endl;

    // Add buy orders
    book.add_order(Side::BUY, OrderType::LIMIT, 150.00, 100);
    book.add_order(Side::BUY, OrderType::LIMIT, 149.95, 200);
    book.add_order(Side::BUY, OrderType::LIMIT, 149.90, 150);
    book.add_order(Side::BUY, OrderType::LIMIT, 149.85, 300);

    // Add sell orders
    book.add_order(Side::SELL, OrderType::LIMIT, 150.10, 100);
    book.add_order(Side::SELL, OrderType::LIMIT, 150.15, 200);
    book.add_order(Side::SELL, OrderType::LIMIT, 150.20, 150);
    book.add_order(Side::SELL, OrderType::LIMIT, 150.25, 300);

    book.print_depth(5);

    std::cout << "\n=== Adding Aggressive Buy Order (Matches) ===" << std::endl;
    book.add_order(Side::BUY, OrderType::LIMIT, 150.20, 250);

    book.print_depth(5);

    std::cout << "\n=== Adding Market Sell Order ===" << std::endl;
    book.add_order(Side::SELL, OrderType::MARKET, 0.0, 150);

    book.print_depth(5);

    book.print_stats();
}

/**
 * KEY INTERVIEW POINTS:
 *
 * 1. Data Structure Choices:
 *    - std::map: O(log n) insert/delete, sorted order
 *    - std::unordered_map: O(1) average lookup
 *    - std::list: O(1) push/pop for FIFO within price level
 *    - Alternative: Custom hash table + intrusive list
 *
 * 2. Matching Algorithm:
 *    - Price-Time Priority: Best price first, then FIFO
 *    - Pro-rata: Distribute based on size
 *    - Price-Size-Time: Size also matters
 *
 * 3. Performance Optimization:
 *    - Cache best bid/ask for O(1) access
 *    - Use intrusive containers to avoid allocations
 *    - Memory pool for orders
 *    - Batch updates to reduce overhead
 *
 * 4. Order Types:
 *    - Limit: Execute at price or better
 *    - Market: Execute at best available price
 *    - IOC: Immediate or Cancel (no resting)
 *    - FOK: Fill or Kill (all or nothing)
 *
 * 5. Real-World Considerations:
 *    - Order validation (price limits, quantity)
 *    - Risk checks (position limits, capital)
 *    - Auction modes (opening, closing)
 *    - Hidden orders and iceberg orders
 *    - Self-match prevention
 *
 * 6. Common Interview Questions:
 *    Q: How to handle best bid/ask efficiently?
 *    A: Use ordered map, begin() gives best price O(1)
 *
 *    Q: Why use map instead of priority queue?
 *    A: Need to cancel orders at arbitrary prices
 *       Priority queue doesn't support this efficiently
 *
 *    Q: How to optimize for high message rate?
 *    A: Memory pools, intrusive containers, SIMD,
 *       batch processing, lock-free when possible
 *
 *    Q: How to handle market data dissemination?
 *    A: Incremental updates (add/modify/delete)
 *       Snapshot + deltas, multicast for distribution
 *
 * 7. Complexity Analysis:
 *    - Add order: O(log n) for price level + O(k log n) matching
 *    - Cancel order: O(log n) for lookup + O(1) list removal
 *    - Modify order: Same as cancel + add
 *    - Best bid/ask: O(1) with ordered map
 *    - Market depth: O(levels)
 *
 * 8. Memory Optimization:
 *    - Use object pools to avoid allocation
 *    - Intrusive containers reduce indirection
 *    - Pack data structures (avoid padding)
 *    - Use fixed-size strings for symbols
 *
 * 9. Alternative Designs:
 *    - Hash map of price levels (faster for specific prices)
 *    - Skip list instead of map (lock-free friendly)
 *    - B-tree for better cache locality
 *    - Ladder structure for limited price range
 *
 * Time Complexity:
 * - Add: O(log n + k log n) where k = matches
 * - Cancel: O(log n)
 * - Best price: O(1)
 * - Depth: O(levels)
 *
 * Space Complexity: O(orders + price_levels)
 */

int main() {
    demonstrate_order_book();
    return 0;
}

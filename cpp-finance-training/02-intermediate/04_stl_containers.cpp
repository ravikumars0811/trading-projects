/*
 * STL Containers
 * HFT/Finance Context: Order books, price history, portfolios
 */

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <queue>
#include <deque>
#include <list>
#include <array>
#include <string>
#include <iomanip>
#include <algorithm>

struct Order {
    uint64_t id;
    std::string symbol;
    double price;
    int quantity;
    char side;  // 'B' or 'S'

    void display() const {
        std::cout << "Order #" << id << ": "
                  << (side == 'B' ? "BUY " : "SELL") << " "
                  << quantity << " " << symbol << " @ $" << price << std::endl;
    }
};

// Comparator for price-time priority
struct BuyOrderCompare {
    bool operator()(const Order& a, const Order& b) const {
        // Higher price has priority (for buy orders)
        if (a.price != b.price) return a.price < b.price;  // Note: < for max-heap
        return a.id > b.id;  // Earlier orders (lower ID) have priority
    }
};

int main() {
    std::cout << "=== STL Containers ===" << std::endl << std::endl;

    // 1. Vector: Dynamic array - price history, tick data
    std::cout << "--- std::vector (Dynamic Array) ---" << std::endl;
    std::vector<double> prices;

    // Adding elements
    prices.push_back(150.25);
    prices.push_back(150.30);
    prices.push_back(150.28);
    prices.push_back(150.35);
    prices.push_back(150.32);

    std::cout << "Prices: ";
    for (double price : prices) {
        std::cout << "$" << price << " ";
    }
    std::cout << "\nSize: " << prices.size() << ", Capacity: " << prices.capacity() << std::endl;

    // Access elements
    std::cout << "First price: $" << prices.front() << std::endl;
    std::cout << "Last price: $" << prices.back() << std::endl;
    std::cout << "Price at index 2: $" << prices[2] << std::endl;

    // Reserve capacity to avoid reallocations
    std::vector<double> efficient_prices;
    efficient_prices.reserve(1000);  // Pre-allocate
    std::cout << "Reserved capacity: " << efficient_prices.capacity() << std::endl;
    std::cout << std::endl;

    // 2. Map: Ordered key-value pairs - symbol to price mapping
    std::cout << "--- std::map (Ordered Map) ---" << std::endl;
    std::map<std::string, double> stock_prices;

    // Inserting
    stock_prices["AAPL"] = 150.25;
    stock_prices["MSFT"] = 375.50;
    stock_prices["GOOGL"] = 140.75;
    stock_prices.insert({"AMZN", 185.30});

    // Accessing
    std::cout << "AAPL price: $" << stock_prices["AAPL"] << std::endl;

    // Iterating (keys are sorted)
    std::cout << "\nAll stock prices (sorted by symbol):" << std::endl;
    for (const auto& pair : stock_prices) {
        std::cout << "  " << pair.first << ": $" << pair.second << std::endl;
    }

    // Check if key exists
    if (stock_prices.find("TSLA") != stock_prices.end()) {
        std::cout << "TSLA found" << std::endl;
    } else {
        std::cout << "TSLA not found" << std::endl;
    }
    std::cout << std::endl;

    // 3. Unordered Map: Hash table - faster lookups, no ordering
    std::cout << "--- std::unordered_map (Hash Table) ---" << std::endl;
    std::unordered_map<std::string, int> positions;  // Symbol -> quantity

    positions["AAPL"] = 1000;
    positions["MSFT"] = 500;
    positions["GOOGL"] = 750;

    std::cout << "Positions (unordered):" << std::endl;
    for (const auto& [symbol, qty] : positions) {  // C++17 structured binding
        std::cout << "  " << symbol << ": " << qty << " shares" << std::endl;
    }

    // Average O(1) lookup
    std::cout << "AAPL position: " << positions["AAPL"] << " shares" << std::endl;
    std::cout << std::endl;

    // 4. Set: Ordered unique elements - watchlist
    std::cout << "--- std::set (Ordered Set) ---" << std::endl;
    std::set<std::string> watchlist;

    watchlist.insert("AAPL");
    watchlist.insert("MSFT");
    watchlist.insert("GOOGL");
    watchlist.insert("AAPL");  // Duplicate, won't be inserted

    std::cout << "Watchlist (" << watchlist.size() << " symbols):" << std::endl;
    for (const auto& symbol : watchlist) {
        std::cout << "  " << symbol << std::endl;
    }
    std::cout << std::endl;

    // 5. Priority Queue: Order book implementation
    std::cout << "--- std::priority_queue (Heap) ---" << std::endl;
    std::priority_queue<Order, std::vector<Order>, BuyOrderCompare> buy_orders;

    buy_orders.push({1001, "AAPL", 150.25, 100, 'B'});
    buy_orders.push({1002, "AAPL", 150.30, 200, 'B'});  // Higher price
    buy_orders.push({1003, "AAPL", 150.28, 150, 'B'});

    std::cout << "Buy orders (best price first):" << std::endl;
    while (!buy_orders.empty()) {
        const Order& top = buy_orders.top();
        top.display();
        buy_orders.pop();
    }
    std::cout << std::endl;

    // 6. Deque: Double-ended queue - sliding window for moving average
    std::cout << "--- std::deque (Double-Ended Queue) ---" << std::endl;
    std::deque<double> price_window;
    int window_size = 3;

    std::vector<double> tick_prices = {150.25, 150.30, 150.28, 150.35, 150.32};
    for (double price : tick_prices) {
        price_window.push_back(price);
        if (price_window.size() > window_size) {
            price_window.pop_front();  // Remove oldest
        }

        // Calculate moving average
        double sum = 0.0;
        for (double p : price_window) sum += p;
        double ma = sum / price_window.size();

        std::cout << "Price: $" << price << ", MA(" << price_window.size() << "): $"
                  << std::fixed << std::setprecision(2) << ma << std::endl;
    }
    std::cout << std::endl;

    // 7. List: Doubly-linked list - frequent insertions/deletions
    std::cout << "--- std::list (Linked List) ---" << std::endl;
    std::list<Order> order_list;

    order_list.push_back({1001, "AAPL", 150.25, 100, 'B'});
    order_list.push_back({1002, "MSFT", 375.50, 200, 'S'});
    order_list.push_front({1000, "GOOGL", 140.75, 150, 'B'});  // Insert at front

    std::cout << "Orders in list:" << std::endl;
    for (const auto& order : order_list) {
        order.display();
    }

    // Remove specific order
    order_list.remove_if([](const Order& o) { return o.id == 1001; });
    std::cout << "After removing order 1001:" << std::endl;
    for (const auto& order : order_list) {
        order.display();
    }
    std::cout << std::endl;

    // 8. Array: Fixed-size array (C++11)
    std::cout << "--- std::array (Fixed-Size Array) ---" << std::endl;
    std::array<double, 5> bid_ask_spreads = {0.01, 0.02, 0.015, 0.025, 0.01};

    std::cout << "Bid-Ask spreads: ";
    for (double spread : bid_ask_spreads) {
        std::cout << "$" << spread << " ";
    }
    std::cout << "\nSize: " << bid_ask_spreads.size() << std::endl;
    std::cout << std::endl;

    // 9. Multimap: Multiple values per key - time series by symbol
    std::cout << "--- std::multimap (Multiple Values per Key) ---" << std::endl;
    std::multimap<std::string, double> price_history;

    price_history.insert({"AAPL", 150.25});
    price_history.insert({"AAPL", 150.30});
    price_history.insert({"AAPL", 150.28});
    price_history.insert({"MSFT", 375.50});
    price_history.insert({"MSFT", 375.55});

    std::cout << "AAPL price history:" << std::endl;
    auto range = price_history.equal_range("AAPL");
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << "  $" << it->second << std::endl;
    }
    std::cout << std::endl;

    // Container comparison
    std::cout << "--- Container Performance Comparison ---" << std::endl;
    std::cout << "Vector:         Random access O(1), Insert/delete at end O(1)" << std::endl;
    std::cout << "Map:            Ordered, lookup O(log n), insert O(log n)" << std::endl;
    std::cout << "Unordered_map:  Hash table, average O(1) lookup, no ordering" << std::endl;
    std::cout << "Set:            Ordered unique, O(log n) operations" << std::endl;
    std::cout << "Deque:          Fast insert/delete at both ends" << std::endl;
    std::cout << "List:           Fast insert/delete anywhere, slow random access" << std::endl;
    std::cout << "Priority_queue: Heap, O(log n) insert, O(1) access max/min" << std::endl;
    std::cout << std::endl;

    // Container choice for trading
    std::cout << "--- Container Choice for Trading Systems ---" << std::endl;
    std::cout << "Price History: vector (fast, sequential access)" << std::endl;
    std::cout << "Order Book: map or priority_queue (price-ordered)" << std::endl;
    std::cout << "Symbol Lookup: unordered_map (fast O(1) lookups)" << std::endl;
    std::cout << "Watchlist: set or unordered_set (unique symbols)" << std::endl;
    std::cout << "Recent Ticks: deque (sliding window)" << std::endl;
    std::cout << "Order Queue: list or deque (frequent cancellations)" << std::endl;

    return 0;
}

/*
 * Key Takeaways:
 * 1. vector: Best default choice, fast random access
 * 2. map: Ordered key-value, slower than unordered_map
 * 3. unordered_map: Fast hash-based lookup, use for symbol tables
 * 4. set: Unique ordered elements
 * 5. priority_queue: Heap for maintaining min/max
 * 6. deque: Double-ended queue, good for sliding windows
 * 7. list: Linked list, good for frequent insertions/deletions
 * 8. array: Fixed-size, stack-allocated
 *
 * Performance Tips for HFT:
 * - Reserve vector capacity if size known: vec.reserve(n)
 * - Use unordered_map for symbol lookups (faster than map)
 * - Avoid frequent reallocations in hot paths
 * - Consider custom allocators for memory pools
 * - Profile container operations in your specific use case
 * - For ultra-low latency: consider lock-free containers
 */

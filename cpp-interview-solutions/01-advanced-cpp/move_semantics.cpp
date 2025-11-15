/**
 * Move Semantics - Critical for HFT Performance
 *
 * Applications in HFT:
 * - Zero-copy message passing between threads
 * - Efficient order transfer through pipeline stages
 * - Avoiding deep copies of large market data structures
 *
 * Interview Focus:
 * - Understanding lvalue vs rvalue
 * - Perfect forwarding
 * - Move constructors/assignment
 * - std::move vs std::forward
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <cstring>

// Example 1: Basic Move Semantics
class MarketOrder {
private:
    char* symbol_;
    double price_;
    int quantity_;
    uint64_t timestamp_;

public:
    // Constructor
    MarketOrder(const char* symbol, double price, int quantity)
        : price_(price), quantity_(quantity) {
        size_t len = std::strlen(symbol) + 1;
        symbol_ = new char[len];
        std::strcpy(symbol_, symbol);
        timestamp_ = std::chrono::system_clock::now().time_since_epoch().count();
        std::cout << "Constructor called for " << symbol_ << std::endl;
    }

    // Copy Constructor (expensive - avoid in HFT)
    MarketOrder(const MarketOrder& other)
        : price_(other.price_), quantity_(other.quantity_), timestamp_(other.timestamp_) {
        size_t len = std::strlen(other.symbol_) + 1;
        symbol_ = new char[len];
        std::strcpy(symbol_, other.symbol_);
        std::cout << "Copy constructor called for " << symbol_ << " (EXPENSIVE!)" << std::endl;
    }

    // Move Constructor (fast - preferred in HFT)
    MarketOrder(MarketOrder&& other) noexcept
        : symbol_(other.symbol_),
          price_(other.price_),
          quantity_(other.quantity_),
          timestamp_(other.timestamp_) {
        other.symbol_ = nullptr;  // Leave source in valid state
        std::cout << "Move constructor called (FAST!)" << std::endl;
    }

    // Copy Assignment
    MarketOrder& operator=(const MarketOrder& other) {
        if (this != &other) {
            delete[] symbol_;
            size_t len = std::strlen(other.symbol_) + 1;
            symbol_ = new char[len];
            std::strcpy(symbol_, other.symbol_);
            price_ = other.price_;
            quantity_ = other.quantity_;
            timestamp_ = other.timestamp_;
            std::cout << "Copy assignment called (EXPENSIVE!)" << std::endl;
        }
        return *this;
    }

    // Move Assignment
    MarketOrder& operator=(MarketOrder&& other) noexcept {
        if (this != &other) {
            delete[] symbol_;
            symbol_ = other.symbol_;
            price_ = other.price_;
            quantity_ = other.quantity_;
            timestamp_ = other.timestamp_;
            other.symbol_ = nullptr;
            std::cout << "Move assignment called (FAST!)" << std::endl;
        }
        return *this;
    }

    ~MarketOrder() {
        if (symbol_) {
            std::cout << "Destructor called for " << symbol_ << std::endl;
        }
        delete[] symbol_;
    }

    void display() const {
        if (symbol_) {
            std::cout << "Order: " << symbol_ << ", Price: " << price_
                      << ", Qty: " << quantity_ << std::endl;
        }
    }
};

// Example 2: Perfect Forwarding - Critical for template libraries
template<typename T>
class OrderFactory {
public:
    // Perfect forwarding: preserves lvalue/rvalue nature
    template<typename... Args>
    static T createOrder(Args&&... args) {
        std::cout << "Factory creating order with perfect forwarding" << std::endl;
        return T(std::forward<Args>(args)...);
    }
};

// Example 3: Move in containers (HFT order queues)
class OrderQueue {
private:
    std::vector<MarketOrder> orders_;

public:
    // Using move to avoid copies when adding orders
    void addOrder(MarketOrder&& order) {
        orders_.push_back(std::move(order));  // Move instead of copy
    }

    // emplace_back constructs in-place (even better!)
    template<typename... Args>
    void emplaceOrder(Args&&... args) {
        orders_.emplace_back(std::forward<Args>(args)...);
    }

    size_t size() const { return orders_.size(); }
};

// Example 4: Return Value Optimization (RVO) and Move
class MarketDataSnapshot {
private:
    std::vector<double> prices_;
    std::vector<int> volumes_;

public:
    MarketDataSnapshot(size_t size) {
        prices_.reserve(size);
        volumes_.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            prices_.push_back(100.0 + i * 0.01);
            volumes_.push_back(1000 * (i + 1));
        }
    }

    // Return by value - RVO or move will kick in
    static MarketDataSnapshot createSnapshot(size_t size) {
        return MarketDataSnapshot(size);  // RVO - no copy or move!
    }

    size_t size() const { return prices_.size(); }
};

// Example 5: Move-only types (like unique_ptr) - Common in HFT
class UniqueOrder {
private:
    std::unique_ptr<char[]> symbol_;
    double price_;

public:
    UniqueOrder(const char* symbol, double price) : price_(price) {
        size_t len = std::strlen(symbol) + 1;
        symbol_ = std::make_unique<char[]>(len);
        std::strcpy(symbol_.get(), symbol);
    }

    // Delete copy operations - enforce move-only semantics
    UniqueOrder(const UniqueOrder&) = delete;
    UniqueOrder& operator=(const UniqueOrder&) = delete;

    // Default move operations
    UniqueOrder(UniqueOrder&&) = default;
    UniqueOrder& operator=(UniqueOrder&&) = default;

    const char* getSymbol() const { return symbol_.get(); }
    double getPrice() const { return price_; }
};

// Example 6: Forwarding Reference vs Rvalue Reference
template<typename T>
void process_order_forwarding(T&& order) {  // Forwarding reference (universal reference)
    // T&& is forwarding reference when T is template parameter
    std::cout << "Processing with forwarding reference" << std::endl;
}

void process_order_rvalue(MarketOrder&& order) {  // Rvalue reference only
    // MarketOrder&& is rvalue reference (not a template)
    std::cout << "Processing rvalue only" << std::endl;
}

// Demonstration and Performance Analysis
void demonstrate_move_semantics() {
    std::cout << "\n=== Basic Move Semantics ===" << std::endl;
    MarketOrder order1("AAPL", 150.0, 100);
    MarketOrder order2 = std::move(order1);  // Move constructor
    // order1 is now in valid but unspecified state

    std::cout << "\n=== Copy vs Move Performance ===" << std::endl;
    MarketOrder order3("GOOGL", 2800.0, 50);
    MarketOrder order4("MSFT", 300.0, 75);

    // Copy - expensive
    MarketOrder order5 = order3;

    // Move - cheap
    MarketOrder order6 = std::move(order4);

    std::cout << "\n=== Perfect Forwarding ===" << std::endl;
    auto order7 = OrderFactory<MarketOrder>::createOrder("TSLA", 700.0, 25);

    std::cout << "\n=== Move in Containers ===" << std::endl;
    OrderQueue queue;
    queue.addOrder(MarketOrder("AMZN", 3300.0, 10));  // Temporary - moves automatically
    queue.emplaceOrder("NVDA", 500.0, 200);  // Construct in-place - best performance!

    std::cout << "Queue size: " << queue.size() << std::endl;

    std::cout << "\n=== RVO Demonstration ===" << std::endl;
    auto snapshot = MarketDataSnapshot::createSnapshot(1000000);
    std::cout << "Snapshot size: " << snapshot.size() << std::endl;

    std::cout << "\n=== Move-Only Type ===" << std::endl;
    UniqueOrder unique1("FB", 350.0);
    UniqueOrder unique2 = std::move(unique1);  // Must use move
    // UniqueOrder unique3 = unique2;  // ERROR: copy deleted
    std::cout << "Unique order: " << unique2.getSymbol() << std::endl;

    std::cout << "\n=== Forwarding vs Rvalue Reference ===" << std::endl;
    MarketOrder order8("NFLX", 500.0, 30);
    process_order_forwarding(order8);           // OK - lvalue
    process_order_forwarding(MarketOrder("DIS", 180.0, 40));  // OK - rvalue
    // process_order_rvalue(order8);            // ERROR: lvalue not allowed
    process_order_rvalue(MarketOrder("PYPL", 250.0, 20));  // OK - rvalue only
}

/**
 * KEY INTERVIEW POINTS:
 *
 * 1. Move Semantics Benefits:
 *    - Eliminates unnecessary copies
 *    - Critical for HFT latency reduction
 *    - Enables efficient resource transfer
 *
 * 2. When Move Happens:
 *    - std::move() creates rvalue reference
 *    - Returning local objects (RVO preferred)
 *    - Temporaries (automatic move)
 *    - Moving from containers
 *
 * 3. Rule of Five:
 *    If you define one of: destructor, copy constructor, copy assignment,
 *    move constructor, move assignment -> consider defining all five
 *
 * 4. noexcept:
 *    - Move operations should be noexcept
 *    - Enables optimizations in STL containers
 *    - std::vector won't use move if not noexcept
 *
 * 5. Performance Impact in HFT:
 *    - Copy: O(n) time for n bytes
 *    - Move: O(1) time - just pointer swap
 *    - For 1MB object at 1M ops/sec: 1TB/sec vs 8MB/sec!
 *
 * 6. Common Mistakes:
 *    - Using std::move on const objects (becomes copy)
 *    - Not marking move operations noexcept
 *    - Moving from objects you'll use again
 *    - Confusion between std::move and std::forward
 *
 * Time Complexity:
 * - Copy: O(n) where n is object size
 * - Move: O(1)
 *
 * Space Complexity:
 * - Copy: O(n) - duplicate memory
 * - Move: O(1) - transfer ownership
 */

int main() {
    demonstrate_move_semantics();
    return 0;
}

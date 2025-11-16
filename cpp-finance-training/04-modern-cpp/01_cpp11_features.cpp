/*
 * C++11 Features for HFT and Finance
 * Modern C++ improvements that enhance performance and code quality
 */

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>
#include <algorithm>
#include <functional>

// 1. Auto and Type Inference
void demonstrateAuto() {
    std::cout << "\n--- Auto and Type Inference ---" << std::endl;

    auto price = 150.25;  // double
    auto symbol = "AAPL";  // const char*
    auto quantity = 1000;  // int
    auto is_buy = true;  // bool

    std::vector<double> prices = {150.25, 150.30, 150.28};
    auto it = prices.begin();  // std::vector<double>::iterator

    // Avoid verbose types
    std::unordered_map<std::string, double> price_map;
    for (auto& [sym, price] : price_map) {  // C++17 structured binding
        // Much cleaner than std::pair<const std::string, double>&
    }

    std::cout << "Auto simplifies type declarations!" << std::endl;
}

// 2. Nullptr (type-safe null pointer)
void processOrder(int* quantity) {
    if (quantity == nullptr) {
        std::cout << "Null quantity pointer" << std::endl;
        return;
    }
    std::cout << "Quantity: " << *quantity << std::endl;
}

void demonstrateNullptr() {
    std::cout << "\n--- Nullptr ---" << std::endl;
    int qty = 100;
    processOrder(&qty);
    processOrder(nullptr);  // Better than NULL or 0
}

// 3. Range-based for loops
void demonstrateRangeFor() {
    std::cout << "\n--- Range-Based For Loops ---" << std::endl;

    std::vector<double> prices = {150.25, 150.30, 150.28, 150.35};

    // Old way
    std::cout << "Old way: ";
    for (size_t i = 0; i < prices.size(); ++i) {
        std::cout << prices[i] << " ";
    }
    std::cout << std::endl;

    // New way (cleaner, safer)
    std::cout << "New way: ";
    for (double price : prices) {
        std::cout << price << " ";
    }
    std::cout << std::endl;

    // By reference (can modify)
    for (double& price : prices) {
        price += 0.01;  // Increment each price
    }

    // Const reference (efficient, read-only)
    for (const auto& price : prices) {
        std::cout << price << " ";
    }
    std::cout << std::endl;
}

// 4. Lambda Expressions
void demonstrateLambdas() {
    std::cout << "\n--- Lambda Expressions ---" << std::endl;

    // Basic lambda
    auto add = [](double a, double b) { return a + b; };
    std::cout << "150.25 + 0.05 = " << add(150.25, 0.05) << std::endl;

    // Capture by value
    double commission_rate = 0.0001;
    auto calculateCommission = [commission_rate](double trade_value) {
        return trade_value * commission_rate;
    };
    std::cout << "Commission: $" << calculateCommission(10000.0) << std::endl;

    // Capture by reference
    int trade_count = 0;
    auto incrementTrades = [&trade_count]() {
        trade_count++;
    };
    incrementTrades();
    incrementTrades();
    std::cout << "Trade count: " << trade_count << std::endl;

    // Capture all by value [=] or by reference [&]
    double bid = 150.25, ask = 150.27;
    auto getMidPrice = [=]() { return (bid + ask) / 2.0; };
    std::cout << "Mid price: $" << getMidPrice() << std::endl;

    // Using lambdas with STL algorithms
    std::vector<double> prices = {150.25, 149.80, 151.00, 150.50, 148.90};

    // Count prices above 150
    int count = std::count_if(prices.begin(), prices.end(),
                              [](double p) { return p > 150.0; });
    std::cout << "Prices above $150: " << count << std::endl;

    // Sort in descending order
    std::sort(prices.begin(), prices.end(),
              [](double a, double b) { return a > b; });
    std::cout << "Sorted (desc): ";
    for (auto p : prices) std::cout << p << " ";
    std::cout << std::endl;
}

// 5. Smart Pointers
class Order {
private:
    std::string symbol_;
    double price_;
    int quantity_;

public:
    Order(const std::string& symbol, double price, int quantity)
        : symbol_(symbol), price_(price), quantity_(quantity) {
        std::cout << "  Order created: " << symbol_ << std::endl;
    }

    ~Order() {
        std::cout << "  Order destroyed: " << symbol_ << std::endl;
    }

    void display() const {
        std::cout << symbol_ << " " << quantity_ << " @ $" << price_ << std::endl;
    }
};

void demonstrateSmartPointers() {
    std::cout << "\n--- Smart Pointers ---" << std::endl;

    // unique_ptr: Exclusive ownership
    {
        auto order1 = std::make_unique<Order>("AAPL", 150.25, 100);
        order1->display();
        // Automatically deleted at end of scope
    }

    // shared_ptr: Shared ownership with reference counting
    {
        auto order2 = std::make_shared<Order>("MSFT", 375.50, 200);
        {
            auto order2_copy = order2;  // Reference count = 2
            std::cout << "  Ref count: " << order2.use_count() << std::endl;
        }  // Reference count back to 1
        std::cout << "  Ref count: " << order2.use_count() << std::endl;
    }  // Reference count to 0, object deleted

    std::cout << "All orders automatically cleaned up!" << std::endl;
}

// 6. Strongly typed enums
enum class OrderType : uint8_t {
    Market = 0,
    Limit = 1,
    Stop = 2,
    StopLimit = 3
};

enum class Side : uint8_t {
    Buy = 0,
    Sell = 1
};

void demonstrateEnums() {
    std::cout << "\n--- Strongly Typed Enums ---" << std::endl;

    OrderType type = OrderType::Limit;
    Side side = Side::Buy;

    // Type safe - won't compile:
    // if (type == side) { }  // ERROR: different types

    // Can specify underlying type
    std::cout << "Size of OrderType: " << sizeof(OrderType) << " byte" << std::endl;
    std::cout << "Size of Side: " << sizeof(Side) << " byte" << std::endl;
}

// 7. constexpr - Compile-time evaluation
constexpr double calculateFutureValue(double pv, double rate, int years) {
    double result = pv;
    for (int i = 0; i < years; ++i) {
        result *= (1.0 + rate);
    }
    return result;
}

void demonstrateConstexpr() {
    std::cout << "\n--- Constexpr ---" << std::endl;

    // Computed at compile time!
    constexpr double fv = calculateFutureValue(1000.0, 0.05, 10);
    std::cout << "Future value (compile-time): $" << fv << std::endl;

    // Can also be runtime
    int years;
    std::cout << "Enter years: ";
    // years = 5;  // For demo, using fixed value
    years = 5;
    double fv_runtime = calculateFutureValue(1000.0, 0.05, years);
    std::cout << "Future value (runtime): $" << fv_runtime << std::endl;
}

// 8. static_assert - Compile-time assertions
template<typename T>
class PriceBuffer {
    static_assert(std::is_arithmetic<T>::value,
                  "PriceBuffer requires arithmetic type");
    // ...
};

void demonstrateStaticAssert() {
    std::cout << "\n--- Static Assert ---" << std::endl;
    std::cout << "Compile-time type checking ensures correctness" << std::endl;

    PriceBuffer<double> valid_buffer;  // OK
    // PriceBuffer<std::string> invalid_buffer;  // Compile error!
}

// 9. Chrono - High-resolution timing
void demonstrateChrono() {
    std::cout << "\n--- Chrono (High-Resolution Timing) ---" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    // Simulate some work
    double sum = 0.0;
    for (int i = 0; i < 1000000; ++i) {
        sum += i * 0.001;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    std::cout << "Calculation took: " << duration.count() << " nanoseconds" << std::endl;
    std::cout << "                  " << duration.count() / 1000.0 << " microseconds" << std::endl;

    // Get current time
    auto now = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();
    std::cout << "Current time (ns since epoch): " << now_ns << std::endl;
}

// 10. Atomic operations (lock-free)
std::atomic<int> order_count{0};

void demonstrateAtomic() {
    std::cout << "\n--- Atomic Operations ---" << std::endl;

    // Thread-safe increment without locks
    order_count.fetch_add(1);
    order_count++;  // Also atomic

    std::cout << "Order count: " << order_count.load() << std::endl;
    std::cout << "Lock-free: " << (order_count.is_lock_free() ? "Yes" : "No") << std::endl;

    // Atomic compare-and-swap (CAS)
    int expected = 1;
    int desired = 2;
    bool success = order_count.compare_exchange_strong(expected, desired);
    std::cout << "CAS success: " << (success ? "Yes" : "No") << std::endl;
    std::cout << "New value: " << order_count.load() << std::endl;
}

int main() {
    std::cout << "=== C++11 Features for HFT & Finance ===" << std::endl;

    demonstrateAuto();
    demonstrateNullptr();
    demonstrateRangeFor();
    demonstrateLambdas();
    demonstrateSmartPointers();
    demonstrateEnums();
    demonstrateConstexpr();
    demonstrateStaticAssert();
    demonstrateChrono();
    demonstrateAtomic();

    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "C++11 is a major upgrade that modernizes C++" << std::endl;
    std::cout << "Key for HFT:" << std::endl;
    std::cout << "  - Move semantics: Zero-copy performance" << std::endl;
    std::cout << "  - Lambdas: Inline strategies, cleaner code" << std::endl;
    std::cout << "  - Smart pointers: No memory leaks" << std::endl;
    std::cout << "  - Chrono: Nanosecond-precision timing" << std::endl;
    std::cout << "  - Atomics: Lock-free concurrency" << std::endl;
    std::cout << "  - constexpr: Compile-time optimization" << std::endl;

    return 0;
}

/*
 * Key Takeaways:
 * 1. Auto reduces verbosity and prevents type mismatches
 * 2. Nullptr is type-safe replacement for NULL
 * 3. Range-for loops are cleaner and safer
 * 4. Lambdas enable functional programming style
 * 5. Smart pointers eliminate manual memory management
 * 6. Enum classes prevent namespace pollution
 * 7. constexpr moves computations to compile-time
 * 8. static_assert catches errors at compile-time
 * 9. Chrono provides high-resolution timing
 * 10. Atomics enable lock-free concurrent programming
 *
 * HFT Best Practices:
 * - Use auto for complex iterator types
 * - Prefer lambdas for inline predicates
 * - Use unique_ptr for ownership, shared_ptr sparingly
 * - constexpr for lookup tables and constants
 * - Atomics for lock-free data structures
 * - Chrono for latency measurement
 * - Always compile with -std=c++11 or later
 */

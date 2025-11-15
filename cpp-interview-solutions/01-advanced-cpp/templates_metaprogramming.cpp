/**
 * Template Metaprogramming & Compile-Time Computation
 *
 * Applications in HFT:
 * - Zero-runtime-cost abstractions
 * - Compile-time optimizations
 * - Type-safe interfaces without overhead
 * - Static dispatch for performance
 *
 * Interview Focus:
 * - Template specialization
 * - SFINAE and enable_if
 * - Variadic templates
 * - constexpr and compile-time computation
 * - Type traits
 */

#include <iostream>
#include <type_traits>
#include <chrono>
#include <cmath>
#include <array>

// Example 1: Basic Template Specialization
template<typename T>
struct PriceFormatter {
    static std::string format(T price) {
        return "Price: " + std::to_string(price);
    }
};

// Specialization for double - higher precision
template<>
struct PriceFormatter<double> {
    static std::string format(double price) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Price: %.4f", price);
        return buffer;
    }
};

// Example 2: Compile-Time Factorial (Classic)
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N - 1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

// Example 3: constexpr for compile-time calculation
constexpr double compute_option_price(double spot, double strike, double rate, double time) {
    // Simplified Black-Scholes (compile-time if inputs are constexpr)
    return spot - strike * (1.0 - rate * time);
}

// Example 4: Variadic Templates - Generic Order Printer
template<typename... Args>
void log_message(Args... args) {
    // Fold expression (C++17)
    ((std::cout << args << " "), ...);
    std::cout << std::endl;
}

// Recursive variadic template (pre-C++17 style)
template<typename T>
void print_orders(T first) {
    std::cout << first << std::endl;
}

template<typename T, typename... Args>
void print_orders(T first, Args... rest) {
    std::cout << first << ", ";
    print_orders(rest...);
}

// Example 5: SFINAE - Enable functions based on type traits
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
calculate_commission(T quantity, double rate) {
    return static_cast<T>(quantity * rate);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
calculate_commission(T notional, double rate) {
    return notional * rate;
}

// Example 6: C++17 if constexpr - Compile-time branching
template<typename T>
auto process_market_data(T data) {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "Processing integer market data: " << data << std::endl;
        return data * 2;
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "Processing floating point market data: " << data << std::endl;
        return data * 1.5;
    } else {
        std::cout << "Processing other market data" << std::endl;
        return data;
    }
}

// Example 7: Type Traits for Optimization
template<typename T>
struct is_trivially_copyable {
    static constexpr bool value = std::is_trivially_copyable_v<T>;
};

template<typename T>
void copy_orders(T* dest, const T* src, size_t count) {
    if constexpr (is_trivially_copyable<T>::value) {
        // Fast path: use memcpy for trivially copyable types
        std::memcpy(dest, src, count * sizeof(T));
        std::cout << "Using memcpy (fast path)" << std::endl;
    } else {
        // Slow path: use copy constructor
        for (size_t i = 0; i < count; ++i) {
            new (&dest[i]) T(src[i]);
        }
        std::cout << "Using copy constructor (slow path)" << std::endl;
    }
}

// Example 8: Policy-Based Design (Andrei Alexandrescu)
template<typename PricingPolicy>
class OptionPricer {
private:
    PricingPolicy policy_;

public:
    double price(double spot, double strike, double rate, double volatility, double time) {
        return policy_.calculate(spot, strike, rate, volatility, time);
    }
};

struct BlackScholesPolicy {
    double calculate(double S, double K, double r, double sigma, double T) {
        // Simplified calculation
        return S - K * exp(-r * T);
    }
};

struct BinomialTreePolicy {
    double calculate(double S, double K, double r, double sigma, double T) {
        // Simplified binomial model
        return (S + K) * 0.5 * (1 - r * T);
    }
};

// Example 9: Template Template Parameters
template<template<typename> class Container, typename T>
class OrderManager {
private:
    Container<T> orders_;

public:
    void add(const T& order) {
        orders_.push_back(order);
    }

    size_t size() const {
        return orders_.size();
    }
};

// Example 10: constexpr for compile-time lookup tables
constexpr std::array<double, 100> generate_price_table() {
    std::array<double, 100> table{};
    for (size_t i = 0; i < 100; ++i) {
        table[i] = 100.0 + i * 0.25;
    }
    return table;
}

constexpr auto PRICE_TABLE = generate_price_table();

// Example 11: CRTP (Curiously Recurring Template Pattern)
template<typename Derived>
class OrderBase {
public:
    void execute() {
        static_cast<Derived*>(this)->execute_impl();
    }

    double calculate_value() {
        return static_cast<Derived*>(this)->calculate_value_impl();
    }
};

class LimitOrder : public OrderBase<LimitOrder> {
private:
    double price_;
    int quantity_;

public:
    LimitOrder(double price, int qty) : price_(price), quantity_(qty) {}

    void execute_impl() {
        std::cout << "Executing limit order at " << price_ << std::endl;
    }

    double calculate_value_impl() {
        return price_ * quantity_;
    }
};

class MarketOrder : public OrderBase<MarketOrder> {
private:
    double market_price_;
    int quantity_;

public:
    MarketOrder(double price, int qty) : market_price_(price), quantity_(qty) {}

    void execute_impl() {
        std::cout << "Executing market order at " << market_price_ << std::endl;
    }

    double calculate_value_impl() {
        return market_price_ * quantity_;
    }
};

// Example 12: Perfect Forwarding with Variadic Templates
template<typename T, typename... Args>
T* create_order(Args&&... args) {
    return new T(std::forward<Args>(args)...);
}

// Example 13: Fold Expressions (C++17)
template<typename... T>
auto sum_prices(T... prices) {
    return (prices + ...);  // Unary right fold
}

template<typename... T>
auto multiply_quantities(T... quantities) {
    return (quantities * ...);
}

// Example 14: constexpr Lambda (C++17)
constexpr auto square = [](int x) { return x * x; };

// Example 15: Concepts (C++20 style with enable_if)
template<typename T>
using EnableIfNumeric = std::enable_if_t<std::is_arithmetic_v<T>>;

template<typename T, typename = EnableIfNumeric<T>>
class PriceLevel {
private:
    T price_;
    int quantity_;

public:
    PriceLevel(T price, int qty) : price_(price), quantity_(qty) {}

    T get_price() const { return price_; }
    int get_quantity() const { return quantity_; }
};

// Example 16: Tag Dispatch for compile-time polymorphism
struct fast_path_tag {};
struct safe_path_tag {};

template<typename T>
void process_order_impl(const T& order, fast_path_tag) {
    std::cout << "Fast path: Direct processing" << std::endl;
    // Optimized code path
}

template<typename T>
void process_order_impl(const T& order, safe_path_tag) {
    std::cout << "Safe path: Validated processing" << std::endl;
    // Validated code path
}

template<typename T>
void process_order(const T& order) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        process_order_impl(order, fast_path_tag{});
    } else {
        process_order_impl(order, safe_path_tag{});
    }
}

// Example 17: Compile-time String Processing
template<size_t N>
struct CompileTimeString {
    char data[N];

    constexpr CompileTimeString(const char (&str)[N]) {
        for (size_t i = 0; i < N; ++i) {
            data[i] = str[i];
        }
    }

    constexpr size_t length() const { return N - 1; }
};

// Demonstration
void demonstrate_templates() {
    std::cout << "\n=== Template Specialization ===" << std::endl;
    std::cout << PriceFormatter<int>::format(100) << std::endl;
    std::cout << PriceFormatter<double>::format(100.1234) << std::endl;

    std::cout << "\n=== Compile-Time Factorial ===" << std::endl;
    constexpr int fact5 = Factorial<5>::value;
    std::cout << "5! = " << fact5 << std::endl;

    std::cout << "\n=== constexpr Function ===" << std::endl;
    constexpr double option = compute_option_price(100.0, 95.0, 0.05, 1.0);
    std::cout << "Option price: " << option << std::endl;

    std::cout << "\n=== Variadic Templates ===" << std::endl;
    log_message("Order:", "AAPL", 150.0, 100);
    print_orders("GOOGL", "MSFT", "TSLA", "AMZN");

    std::cout << "\n=== SFINAE ===" << std::endl;
    std::cout << "Commission (int): " << calculate_commission(100, 0.01) << std::endl;
    std::cout << "Commission (double): " << calculate_commission(10000.0, 0.001) << std::endl;

    std::cout << "\n=== if constexpr ===" << std::endl;
    process_market_data(42);
    process_market_data(3.14159);

    std::cout << "\n=== Type Traits Optimization ===" << std::endl;
    struct TrivialOrder { int id; double price; };
    TrivialOrder src[3] = {{1, 100.0}, {2, 101.0}, {3, 102.0}};
    TrivialOrder dest[3];
    copy_orders(dest, src, 3);

    std::cout << "\n=== Policy-Based Design ===" << std::endl;
    OptionPricer<BlackScholesPolicy> bs_pricer;
    OptionPricer<BinomialTreePolicy> bt_pricer;
    std::cout << "Black-Scholes price: " << bs_pricer.price(100, 95, 0.05, 0.2, 1.0) << std::endl;
    std::cout << "Binomial Tree price: " << bt_pricer.price(100, 95, 0.05, 0.2, 1.0) << std::endl;

    std::cout << "\n=== Compile-Time Lookup Table ===" << std::endl;
    std::cout << "Price at index 10: " << PRICE_TABLE[10] << std::endl;
    std::cout << "Price at index 50: " << PRICE_TABLE[50] << std::endl;

    std::cout << "\n=== CRTP Pattern ===" << std::endl;
    LimitOrder limit(150.0, 100);
    MarketOrder market(150.5, 100);
    limit.execute();
    market.execute();
    std::cout << "Limit value: " << limit.calculate_value() << std::endl;
    std::cout << "Market value: " << market.calculate_value() << std::endl;

    std::cout << "\n=== Fold Expressions ===" << std::endl;
    std::cout << "Sum: " << sum_prices(100.0, 150.0, 200.0, 250.0) << std::endl;
    std::cout << "Product: " << multiply_quantities(2, 3, 4) << std::endl;

    std::cout << "\n=== constexpr Lambda ===" << std::endl;
    constexpr int sq = square(5);
    std::cout << "Square of 5: " << sq << std::endl;

    std::cout << "\n=== Tag Dispatch ===" << std::endl;
    TrivialOrder order1{1, 100.0};
    process_order(order1);
}

/**
 * KEY INTERVIEW POINTS:
 *
 * 1. Template Specialization:
 *    - Full specialization: completely different implementation
 *    - Partial specialization: specialize some template parameters
 *    - Use case: Optimize for specific types (e.g., integral types)
 *
 * 2. SFINAE (Substitution Failure Is Not An Error):
 *    - Enable/disable functions based on type traits
 *    - std::enable_if, std::void_t
 *    - Detection idiom for checking member functions
 *
 * 3. constexpr:
 *    - Evaluated at compile time if possible
 *    - Zero runtime cost
 *    - Can fall back to runtime if needed
 *    - Critical for HFT: move work to compile time
 *
 * 4. Variadic Templates:
 *    - Accept any number of arguments
 *    - Fold expressions (C++17): (args + ...)
 *    - Recursive expansion (pre-C++17)
 *
 * 5. if constexpr (C++17):
 *    - Compile-time branching
 *    - Eliminates dead code paths
 *    - Better than runtime if or function overloading
 *
 * 6. CRTP (Curiously Recurring Template Pattern):
 *    - Compile-time polymorphism
 *    - Zero virtual function overhead
 *    - Perfect for HFT where virtual calls are too slow
 *
 * 7. Policy-Based Design:
 *    - Behavior injection via template parameters
 *    - Compile-time strategy pattern
 *    - Mix-and-match policies at compile time
 *
 * 8. Type Traits:
 *    - std::is_integral, std::is_trivially_copyable, etc.
 *    - Enable optimization based on type properties
 *    - Zero runtime overhead
 *
 * 9. Tag Dispatch:
 *    - Alternative to SFINAE
 *    - Easier to read and debug
 *    - Compiler selects overload at compile time
 *
 * 10. Performance Implications (HFT):
 *     - Virtual function call: ~5-20ns (cache miss)
 *     - Template/inline: 0ns (compile-time resolved)
 *     - constexpr: 0ns (compile-time computed)
 *     - For 1M calls/sec: 20ms saved per strategy!
 *
 * 11. Common Interview Questions:
 *     Q: Template vs Virtual Function?
 *     A: Template: compile-time, zero overhead, code bloat
 *        Virtual: runtime, overhead, no code bloat
 *
 *     Q: When to use constexpr?
 *     A: Constants, lookup tables, simple calculations
 *        Anything that can be computed at compile time
 *
 *     Q: CRTP vs Virtual inheritance?
 *     A: CRTP: compile-time, faster, less flexible
 *        Virtual: runtime, slower, more flexible
 *
 *     Q: What is SFINAE used for?
 *     A: Enable/disable functions based on type properties
 *        Avoid compilation errors, provide type-specific optimizations
 *
 * 12. Best Practices:
 *     - Use constexpr for compile-time computation
 *     - Prefer if constexpr over SFINAE when possible
 *     - Use type traits to optimize based on type properties
 *     - CRTP for zero-cost polymorphism in hot paths
 *     - Watch code bloat with templates
 *
 * Time Complexity:
 * - Compile time: varies (can be significant for deep recursion)
 * - Runtime: O(1) - all resolved at compile time
 *
 * Space Complexity:
 * - Code size: can increase (template instantiation)
 * - Runtime memory: same as hand-written code
 */

int main() {
    demonstrate_templates();
    return 0;
}

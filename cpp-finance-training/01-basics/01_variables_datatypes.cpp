/*
 * Variables, Data Types, and Operators
 * HFT/Finance Context: Price calculations, precision, and numeric types
 */

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <limits>

int main() {
    std::cout << "=== C++ Data Types for Finance ===" << std::endl << std::endl;

    // Integer types - commonly used for quantities, lot sizes
    int quantity = 1000;
    long long order_id = 1234567890123LL;

    // Fixed-width integers (critical for HFT - predictable sizes)
    int32_t shares = 500;
    int64_t market_cap = 1000000000000LL;  // $1 trillion
    uint64_t timestamp_nanos = 1699564800000000000ULL;

    std::cout << "Order Quantity: " << quantity << " shares" << std::endl;
    std::cout << "Order ID: " << order_id << std::endl;
    std::cout << "Market Cap: $" << market_cap << std::endl;
    std::cout << "Timestamp (nanoseconds): " << timestamp_nanos << std::endl;
    std::cout << std::endl;

    // Floating-point types - prices, returns, ratios
    // WARNING: Be careful with float/double for money! Consider fixed-point or integer cents
    float bid_price = 150.25f;      // Less precision
    double ask_price = 150.26;      // Double precision
    long double pnl = 1234567.89L;  // Extended precision

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Bid Price: $" << bid_price << std::endl;
    std::cout << "Ask Price: $" << ask_price << std::endl;
    std::cout << "P&L: $" << pnl << std::endl;
    std::cout << std::endl;

    // Better approach: Store prices as integer cents to avoid floating-point errors
    int64_t bid_cents = 15025;  // $150.25 as cents
    int64_t ask_cents = 15026;  // $150.26 as cents

    std::cout << "Bid Price (cents): " << bid_cents << " ($"
              << bid_cents / 100.0 << ")" << std::endl;
    std::cout << "Ask Price (cents): " << ask_cents << " ($"
              << ask_cents / 100.0 << ")" << std::endl;
    std::cout << std::endl;

    // Arithmetic operators - calculating trade values
    int64_t trade_quantity = 100;
    int64_t price_cents = 15025;
    int64_t total_value_cents = trade_quantity * price_cents;

    std::cout << "Trade: " << trade_quantity << " shares @ $"
              << price_cents / 100.0 << std::endl;
    std::cout << "Total Value: $" << total_value_cents / 100.0 << std::endl;
    std::cout << std::endl;

    // Spread calculation
    int64_t spread = ask_cents - bid_cents;
    std::cout << "Bid-Ask Spread: " << spread << " cents ($"
              << spread / 100.0 << ")" << std::endl;
    std::cout << std::endl;

    // Boolean type - market conditions
    bool is_market_open = true;
    bool is_halted = false;
    bool can_trade = is_market_open && !is_halted;

    std::cout << std::boolalpha;
    std::cout << "Market Open: " << is_market_open << std::endl;
    std::cout << "Trading Halted: " << is_halted << std::endl;
    std::cout << "Can Trade: " << can_trade << std::endl;
    std::cout << std::endl;

    // Character type - symbol parsing
    char order_side = 'B';  // 'B' for Buy, 'S' for Sell
    std::cout << "Order Side: " << order_side << std::endl;
    std::cout << std::endl;

    // Const keyword - values that shouldn't change
    const double COMMISSION_RATE = 0.0001;  // 1 basis point
    const int MAX_ORDER_SIZE = 10000;

    double commission = total_value_cents / 100.0 * COMMISSION_RATE;
    std::cout << "Commission Rate: " << COMMISSION_RATE * 10000 << " bps" << std::endl;
    std::cout << "Commission: $" << commission << std::endl;
    std::cout << "Max Order Size: " << MAX_ORDER_SIZE << " shares" << std::endl;
    std::cout << std::endl;

    // Type limits (important for overflow detection)
    std::cout << "=== Type Limits (Overflow Protection) ===" << std::endl;
    std::cout << "int32_t max: " << std::numeric_limits<int32_t>::max() << std::endl;
    std::cout << "int64_t max: " << std::numeric_limits<int64_t>::max() << std::endl;
    std::cout << "double max: " << std::numeric_limits<double>::max() << std::endl;
    std::cout << "double epsilon: " << std::numeric_limits<double>::epsilon() << std::endl;

    // Demonstrate floating-point precision issues
    std::cout << std::endl << "=== Floating-Point Precision Issues ===" << std::endl;
    double price1 = 0.1 + 0.2;  // Infamous floating-point issue
    std::cout << std::setprecision(17);
    std::cout << "0.1 + 0.2 = " << price1 << " (not exactly 0.3!)" << std::endl;
    std::cout << "This is why we use integer cents for money!" << std::endl;

    return 0;
}

/*
 * Key Takeaways:
 * 1. Use fixed-width types (int32_t, int64_t) for predictable behavior across platforms
 * 2. Store monetary values as integer cents/pips to avoid floating-point errors
 * 3. Use const for values that shouldn't change
 * 4. Be aware of type limits to prevent overflow
 * 5. Understand precision limitations of floating-point types
 */

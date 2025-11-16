/*
 * Control Flow: Conditional Statements and Loops
 * HFT/Finance Context: Order validation, market data processing
 */

#include <iostream>
#include <vector>
#include <string>

// Order validation using if-else
bool validateOrder(char side, int quantity, double price) {
    std::cout << "Validating order: " << side << " " << quantity
              << " @ $" << price << std::endl;

    // Check order side
    if (side != 'B' && side != 'S') {
        std::cout << "  ❌ Invalid side (must be 'B' or 'S')" << std::endl;
        return false;
    }

    // Check quantity
    if (quantity <= 0) {
        std::cout << "  ❌ Invalid quantity (must be positive)" << std::endl;
        return false;
    }

    // Check quantity limits
    if (quantity > 100000) {
        std::cout << "  ❌ Quantity exceeds limit (max 100,000)" << std::endl;
        return false;
    }

    // Check price
    if (price <= 0.0) {
        std::cout << "  ❌ Invalid price (must be positive)" << std::endl;
        return false;
    }

    // Minimum price increment check (tick size)
    double tick_size = 0.01;
    double price_mod = price * 100.0;  // Convert to cents
    int price_cents = static_cast<int>(price_mod);
    if (price_mod - price_cents > 0.0001) {
        std::cout << "  ❌ Invalid price increment (tick size is $"
                  << tick_size << ")" << std::endl;
        return false;
    }

    std::cout << "  ✓ Order validated successfully" << std::endl;
    return true;
}

// Market state check using switch
std::string getMarketState(int hour) {
    switch (hour) {
        case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
            return "Pre-Market Closed";
        case 9:
            return "Pre-Market Open (9:00-9:30 AM)";
        case 10: case 11: case 12: case 13: case 14: case 15:
            return "Market Open (9:30 AM - 4:00 PM)";
        case 16:
            return "Market Closing (4:00 PM)";
        case 17: case 18: case 19:
            return "After-Hours Trading";
        default:
            return "Market Closed";
    }
}

// Process tick data using loops
void processMarketData(const std::vector<double>& prices) {
    std::cout << "Processing " << prices.size() << " price ticks..." << std::endl;

    // For loop - calculating VWAP (simplified)
    double total_price = 0.0;
    for (size_t i = 0; i < prices.size(); ++i) {
        total_price += prices[i];
    }
    double average_price = total_price / prices.size();
    std::cout << "  Average Price: $" << average_price << std::endl;

    // Range-based for loop (C++11) - more elegant
    double min_price = prices[0];
    double max_price = prices[0];
    for (double price : prices) {
        if (price < min_price) min_price = price;
        if (price > max_price) max_price = price;
    }
    std::cout << "  Min Price: $" << min_price << std::endl;
    std::cout << "  Max Price: $" << max_price << std::endl;
    std::cout << "  Range: $" << (max_price - min_price) << std::endl;

    // While loop - finding first price above threshold
    double threshold = 150.50;
    size_t idx = 0;
    while (idx < prices.size() && prices[idx] <= threshold) {
        ++idx;
    }
    if (idx < prices.size()) {
        std::cout << "  First price above $" << threshold << " at index "
                  << idx << ": $" << prices[idx] << std::endl;
    }

    // Do-while loop - ensure at least one iteration
    double cumulative_return = 0.0;
    size_t i = 1;
    do {
        if (i >= prices.size()) break;
        double ret = (prices[i] - prices[i-1]) / prices[i-1];
        cumulative_return += ret;
        ++i;
    } while (i < prices.size());
    std::cout << "  Cumulative Return: " << (cumulative_return * 100) << "%" << std::endl;
}

// Risk check with nested conditions
enum class RiskLevel { LOW, MEDIUM, HIGH, CRITICAL };

RiskLevel assessRisk(double position_size, double volatility, double var) {
    std::cout << "Assessing risk..." << std::endl;
    std::cout << "  Position Size: $" << position_size << std::endl;
    std::cout << "  Volatility: " << volatility << "%" << std::endl;
    std::cout << "  VaR: $" << var << std::endl;

    if (position_size > 10000000) {  // $10M
        if (volatility > 30.0) {
            return RiskLevel::CRITICAL;
        } else if (var > 500000) {  // $500K
            return RiskLevel::HIGH;
        } else {
            return RiskLevel::MEDIUM;
        }
    } else if (position_size > 1000000) {  // $1M
        if (volatility > 50.0 || var > 100000) {
            return RiskLevel::HIGH;
        } else {
            return RiskLevel::MEDIUM;
        }
    } else {
        return volatility > 40.0 ? RiskLevel::MEDIUM : RiskLevel::LOW;
    }
}

int main() {
    std::cout << "=== Control Flow in Trading Systems ===" << std::endl << std::endl;

    // If-else: Order validation
    std::cout << "--- Order Validation ---" << std::endl;
    validateOrder('B', 1000, 150.25);
    validateOrder('S', -100, 150.25);    // Invalid quantity
    validateOrder('X', 1000, 150.25);    // Invalid side
    validateOrder('B', 1000, 150.255);   // Invalid tick size
    std::cout << std::endl;

    // Switch: Market hours
    std::cout << "--- Market Hours ---" << std::endl;
    for (int hour : {8, 9, 12, 16, 20}) {
        std::cout << hour << ":00 - " << getMarketState(hour) << std::endl;
    }
    std::cout << std::endl;

    // Loops: Market data processing
    std::cout << "--- Market Data Processing ---" << std::endl;
    std::vector<double> prices = {150.25, 150.30, 150.20, 150.55, 150.60, 150.45};
    processMarketData(prices);
    std::cout << std::endl;

    // Nested conditions: Risk assessment
    std::cout << "--- Risk Assessment ---" << std::endl;
    RiskLevel risk = assessRisk(15000000, 35.0, 450000);
    std::cout << "  Risk Level: ";
    switch (risk) {
        case RiskLevel::LOW:      std::cout << "LOW ✓" << std::endl; break;
        case RiskLevel::MEDIUM:   std::cout << "MEDIUM ⚠" << std::endl; break;
        case RiskLevel::HIGH:     std::cout << "HIGH ⚠⚠" << std::endl; break;
        case RiskLevel::CRITICAL: std::cout << "CRITICAL ❌" << std::endl; break;
    }
    std::cout << std::endl;

    // Ternary operator - concise conditional
    char side = 'B';
    std::string action = (side == 'B') ? "BUY" : "SELL";
    std::cout << "--- Ternary Operator ---" << std::endl;
    std::cout << "Order action: " << action << std::endl;

    // Continue and break in loops
    std::cout << std::endl << "--- Loop Control (break/continue) ---" << std::endl;
    std::cout << "Processing orders, skip invalid ones:" << std::endl;
    std::vector<int> quantities = {100, -50, 200, 0, 150, 300};
    for (int qty : quantities) {
        if (qty <= 0) {
            std::cout << "  Skipping invalid quantity: " << qty << std::endl;
            continue;  // Skip this iteration
        }
        std::cout << "  Processing order: " << qty << " shares" << std::endl;
        if (qty >= 300) {
            std::cout << "  Large order detected, stopping processing" << std::endl;
            break;  // Exit loop
        }
    }

    return 0;
}

/*
 * Key Takeaways:
 * 1. Use if-else for complex validation logic
 * 2. Switch statements are great for state machines
 * 3. Range-based for loops (C++11) are cleaner and safer
 * 4. While loops for conditional iteration
 * 5. Do-while ensures at least one iteration
 * 6. Break and continue provide fine-grained loop control
 * 7. Ternary operator for simple conditionals
 */

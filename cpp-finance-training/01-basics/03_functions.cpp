/*
 * Functions and Function Overloading
 * HFT/Finance Context: Modular trade calculations and pricing functions
 */

#include <iostream>
#include <cmath>
#include <string>
#include <vector>

// Basic function: Calculate commission
double calculateCommission(double trade_value, double commission_rate = 0.0001) {
    return trade_value * commission_rate;
}

// Function with multiple parameters: Calculate trade P&L
double calculatePnL(double entry_price, double exit_price, int quantity, char side) {
    double pnl;
    if (side == 'B') {  // Long position
        pnl = (exit_price - entry_price) * quantity;
    } else {  // Short position
        pnl = (entry_price - exit_price) * quantity;
    }
    return pnl;
}

// Function overloading: Different versions for different inputs
// Version 1: Simple interest calculation
double calculateInterest(double principal, double rate, double time) {
    return principal * rate * time;
}

// Version 2: Compound interest calculation
double calculateInterest(double principal, double rate, double time, int compounds_per_year) {
    return principal * std::pow(1.0 + rate / compounds_per_year, compounds_per_year * time) - principal;
}

// Version 3: Continuous compounding
double calculateInterest(double principal, double rate, double time, bool continuous) {
    if (continuous) {
        return principal * (std::exp(rate * time) - 1.0);
    }
    return calculateInterest(principal, rate, time);  // Fall back to simple
}

// Pass by value
void updatePriceByValue(double price, double change) {
    price += change;  // Local copy modified, original unchanged
    std::cout << "  Inside function (by value): $" << price << std::endl;
}

// Pass by reference
void updatePriceByReference(double& price, double change) {
    price += change;  // Original modified
    std::cout << "  Inside function (by reference): $" << price << std::endl;
}

// Pass by const reference (efficient for large objects, prevents modification)
double calculatePortfolioValue(const std::vector<double>& positions,
                                const std::vector<double>& prices) {
    double total = 0.0;
    for (size_t i = 0; i < positions.size() && i < prices.size(); ++i) {
        total += positions[i] * prices[i];
    }
    return total;
}

// Function returning multiple values via references
void calculateBidAskSpread(double bid, double ask, double& spread, double& mid_price) {
    spread = ask - bid;
    mid_price = (bid + ask) / 2.0;
}

// Inline function (hint to compiler for performance)
inline double basisPointsToDecimal(double bps) {
    return bps / 10000.0;
}

// Recursive function: Calculate factorial (used in option pricing formulas)
unsigned long long factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

// Function with default parameters
double calculateFutureValue(double present_value,
                            double interest_rate = 0.05,
                            double years = 1.0,
                            int compounds_per_year = 1) {
    double rate_per_period = interest_rate / compounds_per_year;
    int total_periods = compounds_per_year * years;
    return present_value * std::pow(1.0 + rate_per_period, total_periods);
}

// Black-Scholes helper: Normal cumulative distribution function (simplified)
double normalCDF(double x) {
    return 0.5 * std::erfc(-x * M_SQRT1_2);
}

// Black-Scholes option pricing (demonstrates complex calculation)
double blackScholesCall(double S, double K, double T, double r, double sigma) {
    // S = spot price, K = strike, T = time to maturity,
    // r = risk-free rate, sigma = volatility
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    return S * normalCDF(d1) - K * std::exp(-r * T) * normalCDF(d2);
}

// Lambda function preview (will cover more in modern C++ section)
void demonstrateLambda() {
    auto calculateReturn = [](double initial, double final_val) -> double {
        return (final_val - initial) / initial;
    };

    double ret = calculateReturn(100.0, 110.0);
    std::cout << "Return: " << (ret * 100) << "%" << std::endl;
}

int main() {
    std::cout << "=== Functions in Trading Systems ===" << std::endl << std::endl;

    // Basic function call
    std::cout << "--- Commission Calculation ---" << std::endl;
    double trade_value = 150000.0;
    double commission = calculateCommission(trade_value);
    std::cout << "Trade Value: $" << trade_value << std::endl;
    std::cout << "Commission: $" << commission << std::endl;
    std::cout << std::endl;

    // Function with multiple parameters
    std::cout << "--- P&L Calculation ---" << std::endl;
    double pnl_long = calculatePnL(150.25, 152.50, 1000, 'B');
    double pnl_short = calculatePnL(150.25, 152.50, 1000, 'S');
    std::cout << "Long Position P&L: $" << pnl_long << std::endl;
    std::cout << "Short Position P&L: $" << pnl_short << std::endl;
    std::cout << std::endl;

    // Function overloading
    std::cout << "--- Interest Calculation (Overloading) ---" << std::endl;
    double principal = 100000.0;
    double rate = 0.05;  // 5%
    double time = 2.0;   // 2 years

    double simple = calculateInterest(principal, rate, time);
    double compound = calculateInterest(principal, rate, time, 12);  // Monthly
    double continuous = calculateInterest(principal, rate, time, true);

    std::cout << "Principal: $" << principal << std::endl;
    std::cout << "Simple Interest: $" << simple << std::endl;
    std::cout << "Compound Interest (monthly): $" << compound << std::endl;
    std::cout << "Continuous Compounding: $" << continuous << std::endl;
    std::cout << std::endl;

    // Pass by value vs reference
    std::cout << "--- Pass by Value vs Reference ---" << std::endl;
    double price = 150.0;
    std::cout << "Original price: $" << price << std::endl;
    updatePriceByValue(price, 5.0);
    std::cout << "After by-value call: $" << price << " (unchanged)" << std::endl;
    updatePriceByReference(price, 5.0);
    std::cout << "After by-reference call: $" << price << " (modified)" << std::endl;
    std::cout << std::endl;

    // Pass by const reference (efficient)
    std::cout << "--- Portfolio Valuation ---" << std::endl;
    std::vector<double> positions = {100, 200, 150, 300};
    std::vector<double> prices = {150.25, 75.50, 200.00, 50.75};
    double portfolio_value = calculatePortfolioValue(positions, prices);
    std::cout << "Portfolio Value: $" << portfolio_value << std::endl;
    std::cout << std::endl;

    // Multiple return values via references
    std::cout << "--- Bid-Ask Spread Analysis ---" << std::endl;
    double bid = 150.25, ask = 150.27;
    double spread, mid_price;
    calculateBidAskSpread(bid, ask, spread, mid_price);
    std::cout << "Bid: $" << bid << ", Ask: $" << ask << std::endl;
    std::cout << "Spread: $" << spread << std::endl;
    std::cout << "Mid Price: $" << mid_price << std::endl;
    std::cout << std::endl;

    // Inline function
    std::cout << "--- Basis Points Conversion ---" << std::endl;
    double bps = 50.0;
    double decimal = basisPointsToDecimal(bps);
    std::cout << bps << " bps = " << decimal << " (" << (decimal * 100) << "%)" << std::endl;
    std::cout << std::endl;

    // Recursive function
    std::cout << "--- Factorial (Recursive) ---" << std::endl;
    int n = 5;
    std::cout << n << "! = " << factorial(n) << std::endl;
    std::cout << std::endl;

    // Default parameters
    std::cout << "--- Future Value Calculation ---" << std::endl;
    double pv = 10000.0;
    std::cout << "Present Value: $" << pv << std::endl;
    std::cout << "FV (defaults): $" << calculateFutureValue(pv) << std::endl;
    std::cout << "FV (10%, 3 years): $" << calculateFutureValue(pv, 0.10, 3.0) << std::endl;
    std::cout << "FV (10%, 3 years, quarterly): $"
              << calculateFutureValue(pv, 0.10, 3.0, 4) << std::endl;
    std::cout << std::endl;

    // Black-Scholes option pricing
    std::cout << "--- Black-Scholes Call Option ---" << std::endl;
    double S = 100.0;    // Spot price
    double K = 105.0;    // Strike price
    double T = 0.25;     // 3 months
    double r = 0.05;     // 5% risk-free rate
    double sigma = 0.20; // 20% volatility
    double call_price = blackScholesCall(S, K, T, r, sigma);
    std::cout << "Spot: $" << S << ", Strike: $" << K << std::endl;
    std::cout << "Time to Expiry: " << T << " years" << std::endl;
    std::cout << "Volatility: " << (sigma * 100) << "%" << std::endl;
    std::cout << "Call Option Price: $" << call_price << std::endl;
    std::cout << std::endl;

    // Lambda preview
    std::cout << "--- Lambda Function Preview ---" << std::endl;
    demonstrateLambda();

    return 0;
}

/*
 * Key Takeaways:
 * 1. Functions improve code modularity and reusability
 * 2. Function overloading allows same name with different parameters
 * 3. Pass by reference for efficiency and when modification is needed
 * 4. Pass by const reference for large objects when no modification needed
 * 5. Default parameters make functions more flexible
 * 6. Inline functions can hint compiler to optimize (though modern compilers auto-inline)
 * 7. Recursion useful for mathematical formulas but watch stack depth
 * 8. Use meaningful function names that describe what they do
 */

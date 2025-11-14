#include <gtest/gtest.h>
#include "../include/option_pricer.hpp"
#include <cmath>

using namespace pricing;

// Test fixture for option pricing
class OptionPricingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Standard test parameters
        spot = 100.0;
        strike = 100.0;
        rate = 0.05;
        vol = 0.2;
        maturity = 1.0;
        div_yield = 0.0;
    }

    double spot, strike, rate, vol, maturity, div_yield;
    const double tolerance = 1e-4;
};

// Black-Scholes Tests

TEST_F(OptionPricingTest, BlackScholesCallAtTheMoney) {
    MarketData data(spot, strike, rate, vol, maturity, div_yield);
    BlackScholesPricer pricer(data, OptionType::CALL);

    double price = pricer.price();

    // Expected price around 10.45 for ATM call with these parameters
    EXPECT_NEAR(price, 10.45, 0.5);
    EXPECT_GT(price, 0.0);
}

TEST_F(OptionPricingTest, BlackScholesPutAtTheMoney) {
    MarketData data(spot, strike, rate, vol, maturity, div_yield);
    BlackScholesPricer pricer(data, OptionType::PUT);

    double price = pricer.price();

    // Expected price around 5.57 for ATM put with these parameters
    EXPECT_NEAR(price, 5.57, 0.5);
    EXPECT_GT(price, 0.0);
}

TEST_F(OptionPricingTest, PutCallParity) {
    MarketData data(spot, strike, rate, vol, maturity, div_yield);

    BlackScholesPricer call_pricer(data, OptionType::CALL);
    BlackScholesPricer put_pricer(data, OptionType::PUT);

    double call_price = call_pricer.price();
    double put_price = put_pricer.price();

    // Put-Call Parity: C - P = S - K*exp(-rT)
    double lhs = call_price - put_price;
    double rhs = spot - strike * std::exp(-rate * maturity);

    EXPECT_NEAR(lhs, rhs, tolerance);
}

TEST_F(OptionPricingTest, CallDeltaRange) {
    MarketData data(spot, strike, rate, vol, maturity, div_yield);
    BlackScholesPricer pricer(data, OptionType::CALL);

    Greeks greeks = pricer.calculate_greeks();

    // Call delta should be between 0 and 1
    EXPECT_GE(greeks.delta, 0.0);
    EXPECT_LE(greeks.delta, 1.0);
}

TEST_F(OptionPricingTest, PutDeltaRange) {
    MarketData data(spot, strike, rate, vol, maturity, div_yield);
    BlackScholesPricer pricer(data, OptionType::PUT);

    Greeks greeks = pricer.calculate_greeks();

    // Put delta should be between -1 and 0
    EXPECT_GE(greeks.delta, -1.0);
    EXPECT_LE(greeks.delta, 0.0);
}

TEST_F(OptionPricingTest, GammaPositive) {
    MarketData data(spot, strike, rate, vol, maturity, div_yield);
    BlackScholesPricer call_pricer(data, OptionType::CALL);
    BlackScholesPricer put_pricer(data, OptionType::PUT);

    Greeks call_greeks = call_pricer.calculate_greeks();
    Greeks put_greeks = put_pricer.calculate_greeks();

    // Gamma should be positive and same for call and put
    EXPECT_GT(call_greeks.gamma, 0.0);
    EXPECT_GT(put_greeks.gamma, 0.0);
    EXPECT_NEAR(call_greeks.gamma, put_greeks.gamma, tolerance);
}

TEST_F(OptionPricingTest, VegaPositive) {
    MarketData data(spot, strike, rate, vol, maturity, div_yield);
    BlackScholesPricer pricer(data, OptionType::CALL);

    Greeks greeks = pricer.calculate_greeks();

    // Vega should be positive
    EXPECT_GT(greeks.vega, 0.0);
}

// Binomial Tree Tests

TEST_F(OptionPricingTest, BinomialConvergesToBlackScholes) {
    MarketData data(spot, strike, rate, vol, maturity, div_yield);

    BlackScholesPricer bs_pricer(data, OptionType::CALL);
    BinomialTreePricer bin_pricer(data, OptionType::CALL, OptionStyle::EUROPEAN, 500);

    double bs_price = bs_pricer.price();
    double bin_price = bin_pricer.price();

    // Binomial should converge to Black-Scholes for European options
    EXPECT_NEAR(bs_price, bin_price, 0.5);
}

TEST_F(OptionPricingTest, AmericanCallNoEarlyExerciseWithoutDividends) {
    MarketData data(spot, strike, rate, vol, maturity, div_yield);

    BinomialTreePricer euro_pricer(data, OptionType::CALL, OptionStyle::EUROPEAN, 100);
    BinomialTreePricer amer_pricer(data, OptionType::CALL, OptionStyle::AMERICAN, 100);

    double euro_price = euro_pricer.price();
    double amer_price = amer_pricer.price();

    // American call without dividends should equal European call
    EXPECT_NEAR(euro_price, amer_price, 0.1);
}

TEST_F(OptionPricingTest, AmericanPutGreaterThanEuropean) {
    MarketData data(spot, strike, rate, vol, maturity, div_yield);

    BinomialTreePricer euro_pricer(data, OptionType::PUT, OptionStyle::EUROPEAN, 100);
    BinomialTreePricer amer_pricer(data, OptionType::PUT, OptionStyle::AMERICAN, 100);

    double euro_price = euro_pricer.price();
    double amer_price = amer_pricer.price();

    // American put should be worth at least as much as European put
    EXPECT_GE(amer_price, euro_price - tolerance);
}

// Edge Cases

TEST_F(OptionPricingTest, DeepInTheMoneyCall) {
    MarketData data(150.0, 100.0, rate, vol, maturity, div_yield);
    BlackScholesPricer pricer(data, OptionType::CALL);

    double price = pricer.price();

    // Deep ITM call should be worth at least intrinsic value
    double intrinsic = 150.0 - 100.0;
    EXPECT_GE(price, intrinsic);
}

TEST_F(OptionPricingTest, DeepOutOfTheMoneyPut) {
    MarketData data(150.0, 100.0, rate, vol, maturity, div_yield);
    BlackScholesPricer pricer(data, OptionType::PUT);

    double price = pricer.price();

    // Deep OTM put should be worth very little
    EXPECT_LT(price, 1.0);
    EXPECT_GT(price, 0.0);
}

TEST_F(OptionPricingTest, ZeroVolatility) {
    MarketData data(spot, strike, rate, 0.0, maturity, div_yield);
    BlackScholesPricer pricer(data, OptionType::CALL);

    double price = pricer.price();

    // With zero volatility, price should be discounted intrinsic value
    double expected = std::max(0.0, spot - strike * std::exp(-rate * maturity));
    EXPECT_NEAR(price, expected, tolerance);
}

// Main function
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

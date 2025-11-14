#pragma once

#include <cmath>
#include <vector>
#include <memory>
#include <stdexcept>

namespace pricing {

// Enum for option types
enum class OptionType {
    CALL,
    PUT
};

// Enum for option style
enum class OptionStyle {
    EUROPEAN,
    AMERICAN
};

// Structure to hold market data
struct MarketData {
    double spot_price;        // Current stock price
    double strike_price;      // Strike price
    double risk_free_rate;    // Risk-free interest rate
    double volatility;        // Implied volatility
    double time_to_maturity;  // Time to maturity in years
    double dividend_yield;    // Continuous dividend yield

    MarketData(double s, double k, double r, double sigma, double t, double q = 0.0)
        : spot_price(s), strike_price(k), risk_free_rate(r),
          volatility(sigma), time_to_maturity(t), dividend_yield(q) {
        validate();
    }

    void validate() const {
        if (spot_price <= 0) throw std::invalid_argument("Spot price must be positive");
        if (strike_price <= 0) throw std::invalid_argument("Strike price must be positive");
        if (volatility < 0) throw std::invalid_argument("Volatility cannot be negative");
        if (time_to_maturity < 0) throw std::invalid_argument("Time to maturity cannot be negative");
    }
};

// Structure to hold Greeks
struct Greeks {
    double delta;     // Rate of change of option price w.r.t. underlying price
    double gamma;     // Rate of change of delta w.r.t. underlying price
    double theta;     // Rate of change of option price w.r.t. time
    double vega;      // Rate of change of option price w.r.t. volatility
    double rho;       // Rate of change of option price w.r.t. interest rate

    Greeks() : delta(0), gamma(0), theta(0), vega(0), rho(0) {}
};

// Base class for option pricing
class OptionPricer {
protected:
    MarketData market_data_;
    OptionType option_type_;
    OptionStyle option_style_;

public:
    OptionPricer(const MarketData& data, OptionType type, OptionStyle style)
        : market_data_(data), option_type_(type), option_style_(style) {}

    virtual ~OptionPricer() = default;

    virtual double price() const = 0;
    virtual Greeks calculate_greeks() const = 0;

    // Getters
    const MarketData& get_market_data() const { return market_data_; }
    OptionType get_option_type() const { return option_type_; }
    OptionStyle get_option_style() const { return option_style_; }
};

// Black-Scholes pricing for European options
class BlackScholesPricer : public OptionPricer {
private:
    // Standard normal cumulative distribution function
    double norm_cdf(double x) const;

    // Standard normal probability density function
    double norm_pdf(double x) const;

    // Calculate d1 parameter
    double calculate_d1() const;

    // Calculate d2 parameter
    double calculate_d2() const;

public:
    BlackScholesPricer(const MarketData& data, OptionType type)
        : OptionPricer(data, type, OptionStyle::EUROPEAN) {}

    double price() const override;
    Greeks calculate_greeks() const override;

    // Implied volatility calculation using Newton-Raphson
    static double implied_volatility(const MarketData& data, OptionType type,
                                    double market_price, double tolerance = 1e-6,
                                    int max_iterations = 100);
};

// Binomial tree pricing for American options
class BinomialTreePricer : public OptionPricer {
private:
    int steps_;  // Number of time steps in the tree

    // Calculate payoff at maturity
    double calculate_payoff(double spot) const;

public:
    BinomialTreePricer(const MarketData& data, OptionType type,
                      OptionStyle style = OptionStyle::AMERICAN, int steps = 100)
        : OptionPricer(data, type, style), steps_(steps) {
        if (steps_ < 1) throw std::invalid_argument("Steps must be at least 1");
    }

    double price() const override;
    Greeks calculate_greeks() const override;

    void set_steps(int steps) {
        if (steps < 1) throw std::invalid_argument("Steps must be at least 1");
        steps_ = steps;
    }

    int get_steps() const { return steps_; }
};

// Monte Carlo simulation for options
class MonteCarloOptionPricer : public OptionPricer {
private:
    int num_simulations_;
    unsigned int seed_;

    // Generate random paths
    std::vector<double> generate_paths() const;

public:
    MonteCarloOptionPricer(const MarketData& data, OptionType type,
                          OptionStyle style = OptionStyle::EUROPEAN,
                          int num_simulations = 100000,
                          unsigned int seed = 42)
        : OptionPricer(data, type, style),
          num_simulations_(num_simulations), seed_(seed) {
        if (num_simulations_ < 1)
            throw std::invalid_argument("Number of simulations must be at least 1");
    }

    double price() const override;
    Greeks calculate_greeks() const override;

    void set_num_simulations(int n) {
        if (n < 1) throw std::invalid_argument("Number of simulations must be at least 1");
        num_simulations_ = n;
    }

    int get_num_simulations() const { return num_simulations_; }
};

} // namespace pricing

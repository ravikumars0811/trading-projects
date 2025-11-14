#include "../include/option_pricer.hpp"
#include <algorithm>
#include <numeric>
#include <random>

namespace pricing {

// Black-Scholes Implementation

double BlackScholesPricer::norm_cdf(double x) const {
    return 0.5 * std::erfc(-x * M_SQRT1_2);
}

double BlackScholesPricer::norm_pdf(double x) const {
    return std::exp(-0.5 * x * x) / std::sqrt(2.0 * M_PI);
}

double BlackScholesPricer::calculate_d1() const {
    const auto& md = market_data_;
    double numerator = std::log(md.spot_price / md.strike_price) +
                      (md.risk_free_rate - md.dividend_yield +
                       0.5 * md.volatility * md.volatility) * md.time_to_maturity;
    double denominator = md.volatility * std::sqrt(md.time_to_maturity);

    if (denominator == 0.0) {
        throw std::runtime_error("Volatility or time to maturity is zero");
    }

    return numerator / denominator;
}

double BlackScholesPricer::calculate_d2() const {
    return calculate_d1() - market_data_.volatility *
           std::sqrt(market_data_.time_to_maturity);
}

double BlackScholesPricer::price() const {
    const auto& md = market_data_;

    // Handle edge case: at maturity
    if (md.time_to_maturity <= 0) {
        double intrinsic = (option_type_ == OptionType::CALL) ?
            std::max(0.0, md.spot_price - md.strike_price) :
            std::max(0.0, md.strike_price - md.spot_price);
        return intrinsic;
    }

    double d1 = calculate_d1();
    double d2 = calculate_d2();

    double discount = std::exp(-md.dividend_yield * md.time_to_maturity);
    double pv_strike = md.strike_price *
                      std::exp(-md.risk_free_rate * md.time_to_maturity);

    if (option_type_ == OptionType::CALL) {
        return md.spot_price * discount * norm_cdf(d1) -
               pv_strike * norm_cdf(d2);
    } else {
        return pv_strike * norm_cdf(-d2) -
               md.spot_price * discount * norm_cdf(-d1);
    }
}

Greeks BlackScholesPricer::calculate_greeks() const {
    Greeks greeks;
    const auto& md = market_data_;

    if (md.time_to_maturity <= 0) {
        return greeks;  // All Greeks are zero at maturity
    }

    double d1 = calculate_d1();
    double d2 = calculate_d2();
    double sqrt_t = std::sqrt(md.time_to_maturity);
    double discount = std::exp(-md.dividend_yield * md.time_to_maturity);

    // Delta
    if (option_type_ == OptionType::CALL) {
        greeks.delta = discount * norm_cdf(d1);
    } else {
        greeks.delta = -discount * norm_cdf(-d1);
    }

    // Gamma (same for call and put)
    greeks.gamma = discount * norm_pdf(d1) /
                   (md.spot_price * md.volatility * sqrt_t);

    // Vega (same for call and put) - per 1% change in volatility
    greeks.vega = md.spot_price * discount * norm_pdf(d1) * sqrt_t / 100.0;

    // Theta - per day
    double term1 = -(md.spot_price * discount * norm_pdf(d1) *
                    md.volatility) / (2.0 * sqrt_t);
    double term2 = md.dividend_yield * md.spot_price * discount;
    double term3 = md.risk_free_rate * md.strike_price *
                  std::exp(-md.risk_free_rate * md.time_to_maturity);

    if (option_type_ == OptionType::CALL) {
        greeks.theta = (term1 + term2 * norm_cdf(d1) -
                       term3 * norm_cdf(d2)) / 365.0;
    } else {
        greeks.theta = (term1 - term2 * norm_cdf(-d1) +
                       term3 * norm_cdf(-d2)) / 365.0;
    }

    // Rho - per 1% change in interest rate
    double pv_strike = md.strike_price * md.time_to_maturity *
                      std::exp(-md.risk_free_rate * md.time_to_maturity);

    if (option_type_ == OptionType::CALL) {
        greeks.rho = pv_strike * norm_cdf(d2) / 100.0;
    } else {
        greeks.rho = -pv_strike * norm_cdf(-d2) / 100.0;
    }

    return greeks;
}

double BlackScholesPricer::implied_volatility(const MarketData& data,
                                              OptionType type,
                                              double market_price,
                                              double tolerance,
                                              int max_iterations) {
    // Newton-Raphson method
    double sigma = 0.5;  // Initial guess

    for (int i = 0; i < max_iterations; ++i) {
        MarketData temp_data = data;
        temp_data.volatility = sigma;

        BlackScholesPricer pricer(temp_data, type);
        double price = pricer.price();
        double diff = price - market_price;

        if (std::abs(diff) < tolerance) {
            return sigma;
        }

        // Vega is the derivative of price w.r.t. volatility
        Greeks greeks = pricer.calculate_greeks();
        double vega = greeks.vega * 100.0;  // Convert back from per 1%

        if (std::abs(vega) < 1e-10) {
            throw std::runtime_error("Vega too small, cannot compute implied volatility");
        }

        sigma = sigma - diff / vega;

        // Ensure volatility stays positive
        if (sigma <= 0) {
            sigma = 0.01;
        }
    }

    throw std::runtime_error("Implied volatility did not converge");
}

// Binomial Tree Implementation

double BinomialTreePricer::calculate_payoff(double spot) const {
    if (option_type_ == OptionType::CALL) {
        return std::max(0.0, spot - market_data_.strike_price);
    } else {
        return std::max(0.0, market_data_.strike_price - spot);
    }
}

double BinomialTreePricer::price() const {
    const auto& md = market_data_;
    double dt = md.time_to_maturity / steps_;

    // Calculate up and down factors
    double u = std::exp(md.volatility * std::sqrt(dt));
    double d = 1.0 / u;

    // Risk-neutral probability
    double a = std::exp((md.risk_free_rate - md.dividend_yield) * dt);
    double p = (a - d) / (u - d);

    if (p < 0 || p > 1) {
        throw std::runtime_error("Invalid risk-neutral probability");
    }

    // Build price tree (we only need to store two levels for memory efficiency)
    std::vector<double> option_values(steps_ + 1);

    // Terminal payoffs
    for (int i = 0; i <= steps_; ++i) {
        double spot_at_maturity = md.spot_price * std::pow(u, steps_ - i) *
                                 std::pow(d, i);
        option_values[i] = calculate_payoff(spot_at_maturity);
    }

    // Backward induction
    double discount = std::exp(-md.risk_free_rate * dt);

    for (int step = steps_ - 1; step >= 0; --step) {
        for (int i = 0; i <= step; ++i) {
            // Expected value (discounted)
            double expected = discount * (p * option_values[i] +
                                        (1.0 - p) * option_values[i + 1]);

            // For American options, check early exercise
            if (option_style_ == OptionStyle::AMERICAN) {
                double spot_at_node = md.spot_price * std::pow(u, step - i) *
                                     std::pow(d, i);
                double exercise_value = calculate_payoff(spot_at_node);
                option_values[i] = std::max(expected, exercise_value);
            } else {
                option_values[i] = expected;
            }
        }
    }

    return option_values[0];
}

Greeks BinomialTreePricer::calculate_greeks() const {
    Greeks greeks;
    const auto& md = market_data_;

    // Calculate delta and gamma using finite differences
    double h = md.spot_price * 0.01;  // 1% bump

    // Price with spot + h
    MarketData md_up = md;
    md_up.spot_price += h;
    BinomialTreePricer pricer_up(md_up, option_type_, option_style_, steps_);
    double price_up = pricer_up.price();

    // Price with spot - h
    MarketData md_down = md;
    md_down.spot_price -= h;
    BinomialTreePricer pricer_down(md_down, option_type_, option_style_, steps_);
    double price_down = pricer_down.price();

    // Current price
    double price_current = price();

    // Delta (first derivative)
    greeks.delta = (price_up - price_down) / (2.0 * h);

    // Gamma (second derivative)
    greeks.gamma = (price_up - 2.0 * price_current + price_down) / (h * h);

    // Vega - bump volatility by 1%
    MarketData md_vega = md;
    md_vega.volatility += 0.01;
    BinomialTreePricer pricer_vega(md_vega, option_type_, option_style_, steps_);
    greeks.vega = pricer_vega.price() - price_current;

    // Theta - bump time by 1 day
    if (md.time_to_maturity > 1.0 / 365.0) {
        MarketData md_theta = md;
        md_theta.time_to_maturity -= 1.0 / 365.0;
        BinomialTreePricer pricer_theta(md_theta, option_type_, option_style_, steps_);
        greeks.theta = pricer_theta.price() - price_current;
    }

    // Rho - bump rate by 1%
    MarketData md_rho = md;
    md_rho.risk_free_rate += 0.01;
    BinomialTreePricer pricer_rho(md_rho, option_type_, option_style_, steps_);
    greeks.rho = pricer_rho.price() - price_current;

    return greeks;
}

// Monte Carlo Implementation

std::vector<double> MonteCarloOptionPricer::generate_paths() const {
    const auto& md = market_data_;
    std::vector<double> terminal_prices(num_simulations_);

    std::mt19937 gen(seed_);
    std::normal_distribution<double> dist(0.0, 1.0);

    double drift = (md.risk_free_rate - md.dividend_yield -
                   0.5 * md.volatility * md.volatility) * md.time_to_maturity;
    double diffusion = md.volatility * std::sqrt(md.time_to_maturity);

    for (int i = 0; i < num_simulations_; ++i) {
        double z = dist(gen);
        terminal_prices[i] = md.spot_price * std::exp(drift + diffusion * z);
    }

    return terminal_prices;
}

double MonteCarloOptionPricer::price() const {
    const auto& md = market_data_;

    if (option_style_ == OptionStyle::AMERICAN) {
        // Monte Carlo is not ideal for American options
        // This is a simplified approach
        throw std::runtime_error(
            "Monte Carlo pricing for American options requires Longstaff-Schwartz method");
    }

    auto terminal_prices = generate_paths();

    // Calculate payoffs
    double sum_payoffs = 0.0;
    for (double spot : terminal_prices) {
        double payoff;
        if (option_type_ == OptionType::CALL) {
            payoff = std::max(0.0, spot - md.strike_price);
        } else {
            payoff = std::max(0.0, md.strike_price - spot);
        }
        sum_payoffs += payoff;
    }

    // Discount average payoff
    double average_payoff = sum_payoffs / num_simulations_;
    return average_payoff * std::exp(-md.risk_free_rate * md.time_to_maturity);
}

Greeks MonteCarloOptionPricer::calculate_greeks() const {
    Greeks greeks;
    const auto& md = market_data_;

    // Use pathwise method for delta
    auto terminal_prices = generate_paths();

    double price_current = price();

    // Delta using pathwise method
    double delta_sum = 0.0;
    for (double spot : terminal_prices) {
        double payoff_derivative;
        if (option_type_ == OptionType::CALL) {
            payoff_derivative = (spot > md.strike_price) ? spot / md.spot_price : 0.0;
        } else {
            payoff_derivative = (spot < md.strike_price) ? -spot / md.spot_price : 0.0;
        }
        delta_sum += payoff_derivative;
    }
    greeks.delta = (delta_sum / num_simulations_) *
                   std::exp(-md.risk_free_rate * md.time_to_maturity);

    // Other Greeks using finite differences
    double h_spot = md.spot_price * 0.01;

    MarketData md_up = md;
    md_up.spot_price += h_spot;
    MonteCarloOptionPricer pricer_up(md_up, option_type_, option_style_,
                                     num_simulations_, seed_);
    double price_up = pricer_up.price();

    MarketData md_down = md;
    md_down.spot_price -= h_spot;
    MonteCarloOptionPricer pricer_down(md_down, option_type_, option_style_,
                                       num_simulations_, seed_);
    double price_down = pricer_down.price();

    greeks.gamma = (price_up - 2.0 * price_current + price_down) / (h_spot * h_spot);

    // Vega
    MarketData md_vega = md;
    md_vega.volatility += 0.01;
    MonteCarloOptionPricer pricer_vega(md_vega, option_type_, option_style_,
                                       num_simulations_, seed_);
    greeks.vega = pricer_vega.price() - price_current;

    return greeks;
}

} // namespace pricing

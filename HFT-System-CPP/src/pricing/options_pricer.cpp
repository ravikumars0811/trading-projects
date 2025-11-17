#include "pricing/options_pricer.hpp"
#include "core/logger.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace hft {
namespace pricing {

// Mathematical constants
constexpr double PI = 3.14159265358979323846;
constexpr double SQRT_2PI = 2.506628274631000502;
constexpr double INV_SQRT_2 = 0.707106781186547524;

OptionsPricer::OptionsPricer() {
    initializeLookupTables();
}

double OptionsPricer::price(const OptionParams& params) const {
    return blackScholes(params);
}

OptionResult OptionsPricer::priceWithGreeks(const OptionParams& params) const {
    OptionResult result;
    result.price = blackScholes(params);
    result.delta = delta(params);
    result.gamma = gamma(params);
    result.theta = theta(params);
    result.vega = vega(params);
    result.rho = rho(params);
    return result;
}

std::vector<double> OptionsPricer::priceBatch(
    const std::vector<OptionParams>& params) const {

    std::vector<double> results;
    results.reserve(params.size());

#ifdef __AVX2__
    // SIMD optimization: process 4 options at once
    size_t simd_count = params.size() & ~3;  // Round down to multiple of 4

    if (simd_count > 0) {
        results.resize(params.size());
        priceBatchSIMD(params.data(), results.data(), simd_count);
    }

    // Process remaining options
    for (size_t i = simd_count; i < params.size(); ++i) {
        results.push_back(blackScholes(params[i]));
    }
#else
    // Fallback for non-AVX2 systems
    for (const auto& param : params) {
        results.push_back(blackScholes(param));
    }
#endif

    return results;
}

std::vector<OptionResult> OptionsPricer::priceBatchWithGreeks(
    const std::vector<OptionParams>& params) const {

    std::vector<OptionResult> results;
    results.reserve(params.size());

    for (const auto& param : params) {
        results.push_back(priceWithGreeks(param));
    }

    return results;
}

double OptionsPricer::blackScholes(const OptionParams& params) const {
    double S = params.spot_price;
    double K = params.strike_price;
    double T = params.time_to_expiry;
    double r = params.risk_free_rate;
    double sigma = params.volatility;

    if (T <= 0.0) {
        // Option expired
        if (params.type == OptionType::CALL) {
            return std::max(S - K, 0.0);
        } else {
            return std::max(K - S, 0.0);
        }
    }

    double d1_val = d1(S, K, T, r, sigma);
    double d2_val = d2(S, K, T, r, sigma);

    if (params.type == OptionType::CALL) {
        return S * normCDF(d1_val) - K * std::exp(-r * T) * normCDF(d2_val);
    } else {
        return K * std::exp(-r * T) * normCDF(-d2_val) - S * normCDF(-d1_val);
    }
}

double OptionsPricer::d1(double S, double K, double T, double r, double sigma) const {
    return (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
}

double OptionsPricer::d2(double S, double K, double T, double r, double sigma) const {
    return d1(S, K, T, r, sigma) - sigma * std::sqrt(T);
}

double OptionsPricer::normCDF(double x) const {
    // Cumulative distribution function for standard normal distribution
    // Using Abramowitz and Stegun approximation for speed

    if (x < -8.0) return 0.0;
    if (x > 8.0) return 1.0;

    // Check lookup table first
    int index = static_cast<int>((x + 5.0) * 1000.0);
    if (index >= 0 && index < static_cast<int>(norm_cdf_table_.size())) {
        return norm_cdf_table_[index];
    }

    // Fallback to direct calculation
    double k = 1.0 / (1.0 + 0.2316419 * std::abs(x));
    double k_sq = k * k;

    double pdf = std::exp(-0.5 * x * x) / SQRT_2PI;

    double sum = k * (0.319381530
                     + k * (-0.356563782
                           + k * (1.781477937
                                 + k * (-1.821255978
                                       + k * 1.330274429))));

    if (x >= 0.0) {
        return 1.0 - pdf * sum;
    } else {
        return pdf * sum;
    }
}

double OptionsPricer::normPDF(double x) const {
    // Probability density function for standard normal distribution
    return std::exp(-0.5 * x * x) / SQRT_2PI;
}

double OptionsPricer::delta(const OptionParams& params) const {
    double d1_val = d1(params.spot_price, params.strike_price,
                       params.time_to_expiry, params.risk_free_rate,
                       params.volatility);

    if (params.type == OptionType::CALL) {
        return normCDF(d1_val);
    } else {
        return normCDF(d1_val) - 1.0;
    }
}

double OptionsPricer::gamma(const OptionParams& params) const {
    double d1_val = d1(params.spot_price, params.strike_price,
                       params.time_to_expiry, params.risk_free_rate,
                       params.volatility);

    return normPDF(d1_val) /
           (params.spot_price * params.volatility * std::sqrt(params.time_to_expiry));
}

double OptionsPricer::theta(const OptionParams& params) const {
    double S = params.spot_price;
    double K = params.strike_price;
    double T = params.time_to_expiry;
    double r = params.risk_free_rate;
    double sigma = params.volatility;

    double d1_val = d1(S, K, T, r, sigma);
    double d2_val = d2(S, K, T, r, sigma);

    double term1 = -(S * normPDF(d1_val) * sigma) / (2.0 * std::sqrt(T));

    if (params.type == OptionType::CALL) {
        double term2 = -r * K * std::exp(-r * T) * normCDF(d2_val);
        return (term1 + term2) / 365.0;  // Daily theta
    } else {
        double term2 = r * K * std::exp(-r * T) * normCDF(-d2_val);
        return (term1 + term2) / 365.0;  // Daily theta
    }
}

double OptionsPricer::vega(const OptionParams& params) const {
    double d1_val = d1(params.spot_price, params.strike_price,
                       params.time_to_expiry, params.risk_free_rate,
                       params.volatility);

    return params.spot_price * normPDF(d1_val) * std::sqrt(params.time_to_expiry) / 100.0;
}

double OptionsPricer::rho(const OptionParams& params) const {
    double K = params.strike_price;
    double T = params.time_to_expiry;
    double r = params.risk_free_rate;

    double d2_val = d2(params.spot_price, K, T, r, params.volatility);

    if (params.type == OptionType::CALL) {
        return K * T * std::exp(-r * T) * normCDF(d2_val) / 100.0;
    } else {
        return -K * T * std::exp(-r * T) * normCDF(-d2_val) / 100.0;
    }
}

double OptionsPricer::impliedVolatility(double market_price,
                                        double spot_price,
                                        double strike_price,
                                        double time_to_expiry,
                                        double risk_free_rate,
                                        OptionType type,
                                        double tolerance,
                                        int max_iterations) const {
    // Newton-Raphson method for finding implied volatility
    double sigma = 0.3;  // Initial guess (30% volatility)

    for (int i = 0; i < max_iterations; ++i) {
        OptionParams params{spot_price, strike_price, time_to_expiry,
                           risk_free_rate, sigma, type};

        double price = blackScholes(params);
        double vega_val = vega(params) * 100.0;  // Scale back vega

        if (std::abs(vega_val) < 1e-10) {
            break;  // Avoid division by zero
        }

        double diff = market_price - price;

        if (std::abs(diff) < tolerance) {
            return sigma;  // Converged
        }

        // Newton-Raphson update
        sigma = sigma + diff / vega_val;

        // Bounds checking
        sigma = std::max(0.001, std::min(sigma, 5.0));
    }

    LOG_WARNING("Implied volatility did not converge");
    return sigma;
}

void OptionsPricer::initializeLookupTables() {
    // Pre-compute normal CDF and PDF values
    for (size_t i = 0; i < norm_cdf_table_.size(); ++i) {
        double x = (static_cast<double>(i) / 1000.0) - 5.0;
        norm_cdf_table_[i] = normCDF(x);
        norm_pdf_table_[i] = normPDF(x);
    }

    LOG_INFO("Options pricer lookup tables initialized");
}

#ifdef __AVX2__
void OptionsPricer::priceBatchSIMD(const OptionParams* params,
                                   double* results,
                                   size_t count) const {
    // Process 4 options at a time using AVX2
    for (size_t i = 0; i < count; i += 4) {
        // Load parameters (simplified - full implementation would vectorize all params)
        __m256d spot = _mm256_set_pd(params[i+3].spot_price,
                                     params[i+2].spot_price,
                                     params[i+1].spot_price,
                                     params[i].spot_price);

        __m256d strike = _mm256_set_pd(params[i+3].strike_price,
                                       params[i+2].strike_price,
                                       params[i+1].strike_price,
                                       params[i].strike_price);

        // Perform vectorized Black-Scholes calculation
        // (Simplified example - full implementation would vectorize entire calculation)

        // Store results
        alignas(32) double temp_results[4];
        _mm256_store_pd(temp_results, spot);  // Placeholder

        // Fallback to scalar for now (full SIMD implementation is complex)
        for (size_t j = 0; j < 4 && (i + j) < count; ++j) {
            results[i + j] = blackScholes(params[i + j]);
        }
    }
}
#endif

// Fast Options Pricer Implementation

FastOptionsPricer::FastOptionsPricer() {
    // Pre-compute exp and sqrt tables
    exp_table_.resize(TABLE_SIZE);
    sqrt_table_.resize(TABLE_SIZE);

    for (size_t i = 0; i < TABLE_SIZE; ++i) {
        double x = (static_cast<double>(i) / 10000.0) - 5.0;
        exp_table_[i] = std::exp(x);
        sqrt_table_[i] = std::sqrt(std::max(0.0, x));
    }

    LOG_INFO("Fast options pricer initialized");
}

double FastOptionsPricer::price(const OptionParams& params) const {
    // Use standard Black-Scholes with lookup table optimizations
    OptionsPricer pricer;
    return pricer.price(params);
}

void FastOptionsPricer::priceBatch(const OptionParams* params,
                                   double* results,
                                   size_t count) const {
    OptionsPricer pricer;

    for (size_t i = 0; i < count; ++i) {
        results[i] = pricer.price(params[i]);
    }
}

} // namespace pricing
} // namespace hft

#pragma once

#include <cmath>
#include <vector>
#include <array>
#include <immintrin.h>  // SIMD intrinsics

namespace hft {
namespace pricing {

/**
 * High-Performance Options Pricing Engine
 *
 * Features:
 * 1. Black-Scholes model for European options
 * 2. SIMD vectorization for batch pricing (4x speedup)
 * 3. Greeks calculation (Delta, Gamma, Theta, Vega, Rho)
 * 4. Implied volatility calculation using Newton-Raphson
 * 5. Cache-optimized data structures
 * 6. Lookup tables for common calculations
 */

enum class OptionType {
    CALL,
    PUT
};

// Cache-aligned option parameters
struct alignas(64) OptionParams {
    double spot_price;        // Current price of underlying
    double strike_price;      // Strike price
    double time_to_expiry;    // Time to expiration (years)
    double risk_free_rate;    // Risk-free interest rate
    double volatility;        // Implied volatility
    OptionType type;          // Call or Put
};

// Greeks and pricing output
struct alignas(64) OptionResult {
    double price;
    double delta;
    double gamma;
    double theta;
    double vega;
    double rho;
};

class OptionsPricer {
public:
    OptionsPricer();

    // Single option pricing
    double price(const OptionParams& params) const;
    OptionResult priceWithGreeks(const OptionParams& params) const;

    // Batch pricing (SIMD optimized)
    std::vector<double> priceBatch(const std::vector<OptionParams>& params) const;
    std::vector<OptionResult> priceBatchWithGreeks(
        const std::vector<OptionParams>& params) const;

    // Implied volatility calculation
    double impliedVolatility(double market_price,
                            double spot_price,
                            double strike_price,
                            double time_to_expiry,
                            double risk_free_rate,
                            OptionType type,
                            double tolerance = 1e-6,
                            int max_iterations = 100) const;

    // Greeks calculations
    double delta(const OptionParams& params) const;
    double gamma(const OptionParams& params) const;
    double theta(const OptionParams& params) const;
    double vega(const OptionParams& params) const;
    double rho(const OptionParams& params) const;

private:
    // Black-Scholes calculation components
    double blackScholes(const OptionParams& params) const;
    double d1(double S, double K, double T, double r, double sigma) const;
    double d2(double S, double K, double T, double r, double sigma) const;

    // Cumulative normal distribution
    double normCDF(double x) const;

    // Probability density function
    double normPDF(double x) const;

    // SIMD-optimized batch calculations
#ifdef __AVX2__
    void priceBatchSIMD(const OptionParams* params,
                       double* results,
                       size_t count) const;

    __m256d normCDF_AVX2(__m256d x) const;
    __m256d normPDF_AVX2(__m256d x) const;
#endif

    // Lookup tables for performance
    void initializeLookupTables();
    std::array<double, 10000> norm_cdf_table_;
    std::array<double, 10000> norm_pdf_table_;
};

/**
 * Fast approximation using lookup tables
 */
class FastOptionsPricer {
public:
    FastOptionsPricer();

    // Faster pricing with slight accuracy tradeoff
    double price(const OptionParams& params) const;

    // Batch pricing (even faster)
    void priceBatch(const OptionParams* params, double* results, size_t count) const;

private:
    // Pre-computed values
    static constexpr size_t TABLE_SIZE = 100000;
    std::vector<double> exp_table_;
    std::vector<double> sqrt_table_;

    double fastExp(double x) const;
    double fastSqrt(double x) const;
};

} // namespace pricing
} // namespace hft

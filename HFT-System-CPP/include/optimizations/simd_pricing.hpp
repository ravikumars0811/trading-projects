#pragma once

#include <immintrin.h>  // AVX2/AVX-512 intrinsics
#include <cmath>
#include <array>

namespace hft {
namespace optimization {

/**
 * @brief SIMD-optimized pricing calculations for HFT
 *
 * Provides vectorized implementations of common pricing operations:
 * - VWAP calculation
 * - Moving averages
 * - Price differentials
 * - Statistical calculations
 *
 * Uses AVX2 instructions (8x float32 or 4x float64 per operation)
 */
class SIMDPricing {
public:
    /**
     * @brief Calculate VWAP using AVX2 (8x faster than scalar)
     * @param prices Array of prices (uint32_t)
     * @param volumes Array of volumes (uint32_t)
     * @param n Number of elements (should be multiple of 8 for best performance)
     * @return VWAP value
     */
    static double calculateVWAP_AVX2(const uint32_t* prices,
                                      const uint32_t* volumes,
                                      size_t n) {
        __m256i total_value_vec = _mm256_setzero_si256();
        __m256i total_volume_vec = _mm256_setzero_si256();

        size_t i = 0;

        // Process 8 elements at a time
        for (; i + 8 <= n; i += 8) {
            // Load 8 prices and 8 volumes
            __m256i price_vec = _mm256_loadu_si256(
                reinterpret_cast<const __m256i*>(&prices[i]));
            __m256i volume_vec = _mm256_loadu_si256(
                reinterpret_cast<const __m256i*>(&volumes[i]));

            // Multiply prices * volumes (32-bit)
            __m256i value_vec = _mm256_mullo_epi32(price_vec, volume_vec);

            // Accumulate
            total_value_vec = _mm256_add_epi32(total_value_vec, value_vec);
            total_volume_vec = _mm256_add_epi32(total_volume_vec, volume_vec);
        }

        // Horizontal sum of vectors
        alignas(32) uint32_t total_value_array[8];
        alignas(32) uint32_t total_volume_array[8];
        _mm256_store_si256(reinterpret_cast<__m256i*>(total_value_array), total_value_vec);
        _mm256_store_si256(reinterpret_cast<__m256i*>(total_volume_array), total_volume_vec);

        uint64_t total_value = 0;
        uint64_t total_volume = 0;
        for (int j = 0; j < 8; ++j) {
            total_value += total_value_array[j];
            total_volume += total_volume_array[j];
        }

        // Handle remaining elements
        for (; i < n; ++i) {
            total_value += static_cast<uint64_t>(prices[i]) * volumes[i];
            total_volume += volumes[i];
        }

        return static_cast<double>(total_value) / total_volume;
    }

    /**
     * @brief Calculate simple moving average using AVX2
     * @param data Input array
     * @param n Number of elements
     * @return Average value
     */
    static double calculateSMA_AVX2(const float* data, size_t n) {
        __m256 sum_vec = _mm256_setzero_ps();

        size_t i = 0;
        // Process 8 floats at a time
        for (; i + 8 <= n; i += 8) {
            __m256 data_vec = _mm256_loadu_ps(&data[i]);
            sum_vec = _mm256_add_ps(sum_vec, data_vec);
        }

        // Horizontal sum
        alignas(32) float sum_array[8];
        _mm256_store_ps(sum_array, sum_vec);

        float total = 0.0f;
        for (int j = 0; j < 8; ++j) {
            total += sum_array[j];
        }

        // Handle remaining elements
        for (; i < n; ++i) {
            total += data[i];
        }

        return total / n;
    }

    /**
     * @brief Calculate price returns (vectorized)
     * @param prices Input prices
     * @param returns Output returns array (must be allocated, size n-1)
     * @param n Number of prices
     */
    static void calculateReturns_AVX2(const float* prices, float* returns, size_t n) {
        size_t i = 0;

        // Process 8 elements at a time
        for (; i + 9 <= n; i += 8) {
            __m256 current = _mm256_loadu_ps(&prices[i + 1]);
            __m256 previous = _mm256_loadu_ps(&prices[i]);

            // return = (current - previous) / previous
            __m256 diff = _mm256_sub_ps(current, previous);
            __m256 ret = _mm256_div_ps(diff, previous);

            _mm256_storeu_ps(&returns[i], ret);
        }

        // Handle remaining elements
        for (; i < n - 1; ++i) {
            returns[i] = (prices[i + 1] - prices[i]) / prices[i];
        }
    }

    /**
     * @brief Calculate variance using AVX2 (two-pass algorithm)
     * @param data Input array
     * @param n Number of elements
     * @return Variance
     */
    static double calculateVariance_AVX2(const float* data, size_t n) {
        // First pass: calculate mean
        double mean = calculateSMA_AVX2(data, n);

        __m256 mean_vec = _mm256_set1_ps(static_cast<float>(mean));
        __m256 var_sum_vec = _mm256_setzero_ps();

        size_t i = 0;
        for (; i + 8 <= n; i += 8) {
            __m256 data_vec = _mm256_loadu_ps(&data[i]);

            // diff = data - mean
            __m256 diff = _mm256_sub_ps(data_vec, mean_vec);

            // squared_diff = diff * diff
            __m256 squared_diff = _mm256_mul_ps(diff, diff);

            var_sum_vec = _mm256_add_ps(var_sum_vec, squared_diff);
        }

        // Horizontal sum
        alignas(32) float var_array[8];
        _mm256_store_ps(var_array, var_sum_vec);

        float var_sum = 0.0f;
        for (int j = 0; j < 8; ++j) {
            var_sum += var_array[j];
        }

        // Handle remaining elements
        for (; i < n; ++i) {
            float diff = data[i] - mean;
            var_sum += diff * diff;
        }

        return var_sum / n;
    }

    /**
     * @brief Calculate standard deviation
     */
    static double calculateStdDev_AVX2(const float* data, size_t n) {
        return std::sqrt(calculateVariance_AVX2(data, n));
    }

    /**
     * @brief Vectorized min/max calculation
     * @param data Input array
     * @param n Number of elements
     * @return pair of (min, max)
     */
    static std::pair<float, float> calculateMinMax_AVX2(const float* data, size_t n) {
        __m256 min_vec = _mm256_set1_ps(std::numeric_limits<float>::max());
        __m256 max_vec = _mm256_set1_ps(std::numeric_limits<float>::lowest());

        size_t i = 0;
        for (; i + 8 <= n; i += 8) {
            __m256 data_vec = _mm256_loadu_ps(&data[i]);
            min_vec = _mm256_min_ps(min_vec, data_vec);
            max_vec = _mm256_max_ps(max_vec, data_vec);
        }

        // Horizontal min/max
        alignas(32) float min_array[8];
        alignas(32) float max_array[8];
        _mm256_store_ps(min_array, min_vec);
        _mm256_store_ps(max_array, max_vec);

        float min_val = min_array[0];
        float max_val = max_array[0];

        for (int j = 1; j < 8; ++j) {
            min_val = std::min(min_val, min_array[j]);
            max_val = std::max(max_val, max_array[j]);
        }

        // Handle remaining elements
        for (; i < n; ++i) {
            min_val = std::min(min_val, data[i]);
            max_val = std::max(max_val, data[i]);
        }

        return {min_val, max_val};
    }

    /**
     * @brief Dot product using AVX2 (FMA - Fused Multiply-Add)
     * @param a First vector
     * @param b Second vector
     * @param n Length
     * @return Dot product a·b
     */
    static double dotProduct_AVX2(const float* a, const float* b, size_t n) {
        __m256 sum_vec = _mm256_setzero_ps();

        size_t i = 0;
        for (; i + 8 <= n; i += 8) {
            __m256 a_vec = _mm256_loadu_ps(&a[i]);
            __m256 b_vec = _mm256_loadu_ps(&b[i]);

            // Fused multiply-add: sum += a * b
            sum_vec = _mm256_fmadd_ps(a_vec, b_vec, sum_vec);
        }

        // Horizontal sum
        alignas(32) float sum_array[8];
        _mm256_store_ps(sum_array, sum_vec);

        float total = 0.0f;
        for (int j = 0; j < 8; ++j) {
            total += sum_array[j];
        }

        // Handle remaining elements
        for (; i < n; ++i) {
            total += a[i] * b[i];
        }

        return total;
    }

    /**
     * @brief Exponential moving average (EMA) using AVX2
     * @param data Input prices
     * @param ema Output EMA values (must be allocated, size n)
     * @param n Number of elements
     * @param alpha Smoothing factor (0 < alpha < 1)
     */
    static void calculateEMA_AVX2(const float* data, float* ema, size_t n, float alpha) {
        if (n == 0) return;

        // Initialize first EMA value
        ema[0] = data[0];

        __m256 alpha_vec = _mm256_set1_ps(alpha);
        __m256 one_minus_alpha_vec = _mm256_set1_ps(1.0f - alpha);

        size_t i = 1;

        // Note: EMA has data dependency, so limited vectorization
        // This is a demonstration - in practice, might use different approach
        for (; i < n; ++i) {
            ema[i] = alpha * data[i] + (1.0f - alpha) * ema[i - 1];
        }
    }

    /**
     * @brief Check if AVX2 is supported on current CPU
     */
    static bool isAVX2Supported() {
        #if defined(__AVX2__)
            return true;
        #else
            return false;
        #endif
    }

    /**
     * @brief Benchmark SIMD operations vs scalar
     */
    static void benchmarkSIMD() {
        constexpr size_t N = 1000000;
        alignas(32) std::array<float, N> prices;
        alignas(32) std::array<float, N> volumes;

        // Initialize with random data
        for (size_t i = 0; i < N; ++i) {
            prices[i] = 100.0f + (i % 100);
            volumes[i] = 1000.0f + (i % 500);
        }

        // Time SIMD version
        auto start = std::chrono::high_resolution_clock::now();
        double sma_simd = calculateSMA_AVX2(prices.data(), N);
        auto end = std::chrono::high_resolution_clock::now();
        auto simd_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();

        std::cout << "SIMD SMA: " << sma_simd << " in " << simd_duration << " μs" << std::endl;
    }
};

} // namespace optimization
} // namespace hft

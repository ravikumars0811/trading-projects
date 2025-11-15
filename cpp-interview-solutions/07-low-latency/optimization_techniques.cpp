/**
 * Low-Latency Optimization Techniques for HFT
 *
 * Critical optimizations for sub-microsecond latency:
 * 1. Cache optimization and alignment
 * 2. Branch prediction
 * 3. False sharing prevention
 * 4. Memory prefetching
 * 5. SIMD instructions
 * 6. Inline and compiler hints
 * 7. Lock-free programming
 * 8. CPU affinity and isolation
 *
 * Interview Focus:
 * - Understanding CPU architecture
 * - Cache hierarchy (L1, L2, L3)
 * - Memory access patterns
 * - Compiler optimizations
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <immintrin.h>  // SIMD intrinsics
#include <thread>
#include <atomic>

// ============================================================================
// 1. CACHE LINE ALIGNMENT - Prevent False Sharing
// ============================================================================

// Cache line size (typically 64 bytes)
constexpr size_t CACHE_LINE_SIZE = 64;

// BAD: False sharing - counters on same cache line
struct CountersBad {
    std::atomic<int> counter1;  // 4 bytes
    std::atomic<int> counter2;  // 4 bytes (same cache line!)
};

// GOOD: Cache-aligned - each counter on separate cache line
struct CountersGood {
    alignas(CACHE_LINE_SIZE) std::atomic<int> counter1;
    alignas(CACHE_LINE_SIZE) std::atomic<int> counter2;
};

void demonstrate_false_sharing() {
    std::cout << "\n=== False Sharing Impact ===" << std::endl;

    const int ITERATIONS = 10000000;

    // Test with false sharing
    {
        CountersBad counters{};
        auto start = std::chrono::high_resolution_clock::now();

        std::thread t1([&counters]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                ++counters.counter1;
            }
        });

        std::thread t2([&counters]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                ++counters.counter2;
            }
        });

        t1.join();
        t2.join();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "With false sharing: " << duration.count() << " ms" << std::endl;
    }

    // Test without false sharing
    {
        CountersGood counters{};
        auto start = std::chrono::high_resolution_clock::now();

        std::thread t1([&counters]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                ++counters.counter1;
            }
        });

        std::thread t2([&counters]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                ++counters.counter2;
            }
        });

        t1.join();
        t2.join();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Without false sharing: " << duration.count() << " ms" << std::endl;
    }
}

// ============================================================================
// 2. STRUCT PACKING - Minimize Memory Footprint
// ============================================================================

// BAD: Poor layout - lots of padding
struct OrderBad {
    char type;          // 1 byte  [7 bytes padding]
    double price;       // 8 bytes
    char side;          // 1 byte  [3 bytes padding]
    int quantity;       // 4 bytes
    short priority;     // 2 bytes [6 bytes padding]
};  // Total: 32 bytes

// GOOD: Optimized layout - minimal padding
struct OrderGood {
    double price;       // 8 bytes
    int quantity;       // 4 bytes
    short priority;     // 2 bytes
    char type;          // 1 byte
    char side;          // 1 byte
};  // Total: 16 bytes

void demonstrate_struct_packing() {
    std::cout << "\n=== Struct Packing ===" << std::endl;
    std::cout << "OrderBad size: " << sizeof(OrderBad) << " bytes" << std::endl;
    std::cout << "OrderGood size: " << sizeof(OrderGood) << " bytes" << std::endl;
    std::cout << "Space saved: " << (sizeof(OrderBad) - sizeof(OrderGood)) << " bytes" << std::endl;
}

// ============================================================================
// 3. BRANCH PREDICTION - Avoid Unpredictable Branches
// ============================================================================

// Compiler hints for branch prediction
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

// BAD: Unpredictable branch
int process_order_bad(int price, int threshold) {
    int result = 0;
    if (price > threshold) {  // Unpredictable
        result = price * 2;
    } else {
        result = price;
    }
    return result;
}

// GOOD: With branch prediction hint
int process_order_good(int price, int threshold) {
    int result = 0;
    if (likely(price > threshold)) {  // Hint: usually true
        result = price * 2;
    } else {
        result = price;
    }
    return result;
}

// BETTER: Branchless with conditional move
int process_order_branchless(int price, int threshold) {
    int doubled = price * 2;
    int mask = -(price > threshold);  // -1 if true, 0 if false
    return (doubled & mask) | (price & ~mask);
}

void demonstrate_branch_prediction() {
    std::cout << "\n=== Branch Prediction ===" << std::endl;

    const int ITERATIONS = 100000000;
    std::vector<int> prices(1000);
    for (int i = 0; i < 1000; ++i) {
        prices[i] = 100 + i;  // Mostly > threshold
    }

    // Test branchless
    auto start = std::chrono::high_resolution_clock::now();
    volatile int sum = 0;
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        sum += process_order_branchless(prices[iter % 1000], 150);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Branchless: " << duration.count() << " ms" << std::endl;

    // Test with branch
    start = std::chrono::high_resolution_clock::now();
    sum = 0;
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        sum += process_order_good(prices[iter % 1000], 150);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "With branch hint: " << duration.count() << " ms" << std::endl;
}

// ============================================================================
// 4. MEMORY PREFETCHING - Reduce Cache Misses
// ============================================================================

struct Trade {
    double price;
    int quantity;
    uint64_t timestamp;
    char symbol[8];
};

// Without prefetch
double calculate_vwap_slow(const std::vector<Trade>& trades) {
    double total_pv = 0.0;
    int total_volume = 0;

    for (const auto& trade : trades) {
        total_pv += trade.price * trade.quantity;
        total_volume += trade.quantity;
    }

    return total_volume > 0 ? total_pv / total_volume : 0.0;
}

// With prefetch
double calculate_vwap_fast(const std::vector<Trade>& trades) {
    double total_pv = 0.0;
    int total_volume = 0;

    for (size_t i = 0; i < trades.size(); ++i) {
        // Prefetch next trade (2 ahead for better pipelining)
        if (i + 2 < trades.size()) {
            __builtin_prefetch(&trades[i + 2], 0, 3);  // Read, high temporal locality
        }

        total_pv += trades[i].price * trades[i].quantity;
        total_volume += trades[i].quantity;
    }

    return total_volume > 0 ? total_pv / total_volume : 0.0;
}

void demonstrate_prefetching() {
    std::cout << "\n=== Memory Prefetching ===" << std::endl;

    // Create large dataset to avoid cache hits
    std::vector<Trade> trades(1000000);
    for (size_t i = 0; i < trades.size(); ++i) {
        trades[i] = {100.0 + i * 0.01, 100, i, "AAPL"};
    }

    // Test without prefetch
    auto start = std::chrono::high_resolution_clock::now();
    double vwap1 = calculate_vwap_slow(trades);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Test with prefetch
    start = std::chrono::high_resolution_clock::now();
    double vwap2 = calculate_vwap_fast(trades);
    end = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Without prefetch: " << duration1.count() << " μs" << std::endl;
    std::cout << "With prefetch: " << duration2.count() << " μs" << std::endl;
    std::cout << "Speedup: " << (double)duration1.count() / duration2.count() << "x" << std::endl;
}

// ============================================================================
// 5. SIMD VECTORIZATION - Process Multiple Values in Parallel
// ============================================================================

// Scalar version
double calculate_weighted_price_scalar(const double* prices,
                                       const double* weights,
                                       size_t count) {
    double result = 0.0;
    for (size_t i = 0; i < count; ++i) {
        result += prices[i] * weights[i];
    }
    return result;
}

// SIMD version (AVX - 256-bit = 4 doubles)
double calculate_weighted_price_simd(const double* prices,
                                     const double* weights,
                                     size_t count) {
    __m256d sum = _mm256_setzero_pd();

    size_t i = 0;
    // Process 4 doubles at a time
    for (; i + 4 <= count; i += 4) {
        __m256d p = _mm256_loadu_pd(&prices[i]);
        __m256d w = _mm256_loadu_pd(&weights[i]);
        __m256d prod = _mm256_mul_pd(p, w);
        sum = _mm256_add_pd(sum, prod);
    }

    // Extract sum from vector
    double result[4];
    _mm256_storeu_pd(result, sum);
    double total = result[0] + result[1] + result[2] + result[3];

    // Handle remaining elements
    for (; i < count; ++i) {
        total += prices[i] * weights[i];
    }

    return total;
}

void demonstrate_simd() {
    std::cout << "\n=== SIMD Vectorization ===" << std::endl;

    const size_t SIZE = 10000000;
    std::vector<double> prices(SIZE);
    std::vector<double> weights(SIZE);

    for (size_t i = 0; i < SIZE; ++i) {
        prices[i] = 100.0 + i * 0.01;
        weights[i] = 1.0 / SIZE;
    }

    // Scalar version
    auto start = std::chrono::high_resolution_clock::now();
    double result1 = calculate_weighted_price_scalar(prices.data(), weights.data(), SIZE);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // SIMD version
    start = std::chrono::high_resolution_clock::now();
    double result2 = calculate_weighted_price_simd(prices.data(), weights.data(), SIZE);
    end = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Scalar: " << duration1.count() << " μs" << std::endl;
    std::cout << "SIMD:   " << duration2.count() << " μs" << std::endl;
    std::cout << "Speedup: " << (double)duration1.count() / duration2.count() << "x" << std::endl;
}

// ============================================================================
// 6. INLINE AND COMPILER HINTS
// ============================================================================

// Force inline for small critical functions
inline __attribute__((always_inline))
double calculate_mid_price(double bid, double ask) {
    return (bid + ask) * 0.5;
}

// Prevent inlining for large functions
__attribute__((noinline))
void expensive_operation() {
    // Large function that shouldn't be inlined
    for (volatile int i = 0; i < 1000; ++i);
}

// Hot path annotation (profile-guided optimization)
__attribute__((hot))
void process_market_data(double price) {
    // Frequently called function
    volatile double result = price * 1.01;
}

// Cold path annotation
__attribute__((cold))
void handle_error(const char* msg) {
    // Rarely called error handling
    std::cerr << "Error: " << msg << std::endl;
}

// ============================================================================
// 7. DATA-ORIENTED DESIGN - Structure of Arrays vs Array of Structures
// ============================================================================

// Array of Structures (AoS) - BAD for iteration
struct MarketDataAoS {
    struct Quote {
        double bid;
        double ask;
        int bid_size;
        int ask_size;
    };
    std::vector<Quote> quotes;
};

// Structure of Arrays (SoA) - GOOD for iteration
struct MarketDataSoA {
    std::vector<double> bids;
    std::vector<double> asks;
    std::vector<int> bid_sizes;
    std::vector<int> ask_sizes;
};

// Process only bids - AoS must load entire struct
double process_bids_aos(const MarketDataAoS& data) {
    double sum = 0.0;
    for (const auto& quote : data.quotes) {
        sum += quote.bid;  // Loads entire 24-byte struct
    }
    return sum;
}

// Process only bids - SoA loads only bids array
double process_bids_soa(const MarketDataSoA& data) {
    double sum = 0.0;
    for (double bid : data.bids) {
        sum += bid;  // Loads only 8-byte double
    }
    return sum;
}

void demonstrate_data_oriented_design() {
    std::cout << "\n=== Data-Oriented Design ===" << std::endl;

    const size_t SIZE = 10000000;

    // Setup AoS
    MarketDataAoS aos;
    aos.quotes.reserve(SIZE);
    for (size_t i = 0; i < SIZE; ++i) {
        aos.quotes.push_back({100.0 + i*0.01, 100.02 + i*0.01, 100, 100});
    }

    // Setup SoA
    MarketDataSoA soa;
    soa.bids.reserve(SIZE);
    soa.asks.reserve(SIZE);
    soa.bid_sizes.reserve(SIZE);
    soa.ask_sizes.reserve(SIZE);
    for (size_t i = 0; i < SIZE; ++i) {
        soa.bids.push_back(100.0 + i*0.01);
        soa.asks.push_back(100.02 + i*0.01);
        soa.bid_sizes.push_back(100);
        soa.ask_sizes.push_back(100);
    }

    // Test AoS
    auto start = std::chrono::high_resolution_clock::now();
    double sum1 = process_bids_aos(aos);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Test SoA
    start = std::chrono::high_resolution_clock::now();
    double sum2 = process_bids_soa(soa);
    end = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "AoS: " << duration1.count() << " ms" << std::endl;
    std::cout << "SoA: " << duration2.count() << " ms" << std::endl;
    std::cout << "Speedup: " << (double)duration1.count() / duration2.count() << "x" << std::endl;
}

// ============================================================================
// DEMONSTRATION
// ============================================================================

int main() {
    std::cout << "=== LOW-LATENCY OPTIMIZATION TECHNIQUES ===" << std::endl;

    demonstrate_false_sharing();
    demonstrate_struct_packing();
    demonstrate_branch_prediction();
    demonstrate_prefetching();
    demonstrate_simd();
    demonstrate_data_oriented_design();

    return 0;
}

/**
 * KEY INTERVIEW POINTS:
 *
 * 1. Cache Line Alignment:
 *    - L1 cache: 32KB, ~4 cycles
 *    - L2 cache: 256KB, ~10 cycles
 *    - L3 cache: 8MB, ~40 cycles
 *    - RAM: GB, ~100-300 cycles
 *    - False sharing: 10-100x slowdown
 *
 * 2. Struct Packing:
 *    - Align largest members first
 *    - Group similar-sized members
 *    - Use __attribute__((packed)) carefully
 *    - Cache line = 64 bytes, fit hot data
 *
 * 3. Branch Prediction:
 *    - Modern CPUs: ~95% prediction rate
 *    - Misprediction penalty: 10-20 cycles
 *    - Use likely/unlikely for hints
 *    - Branchless: conditional move (CMOV)
 *
 * 4. Memory Prefetching:
 *    - Prefetch 2-3 iterations ahead
 *    - Temporal locality hints (0-3)
 *    - Hardware prefetcher helps sequential
 *    - Manual for pointer chasing
 *
 * 5. SIMD Benefits:
 *    - SSE: 128-bit (2 doubles, 4 floats)
 *    - AVX: 256-bit (4 doubles, 8 floats)
 *    - AVX-512: 512-bit (8 doubles, 16 floats)
 *    - 2-8x speedup for parallel data
 *
 * 6. Compiler Hints:
 *    - always_inline: Force inline
 *    - noinline: Prevent inline
 *    - hot: Optimize for speed
 *    - cold: Optimize for size
 *    - pure: No side effects
 *    - const: No global state access
 *
 * 7. Data-Oriented Design:
 *    - SoA better for cache (fewer misses)
 *    - AoS better for object-oriented
 *    - SoA enables SIMD easily
 *    - Consider access patterns
 *
 * 8. Common Interview Questions:
 *    Q: What is false sharing?
 *    A: Multiple threads accessing different variables
 *       on same cache line, causing invalidation
 *
 *    Q: How to avoid false sharing?
 *    A: Align to cache line (64 bytes)
 *       Padding between frequently accessed fields
 *       Separate per-thread data
 *
 *    Q: When to use SIMD?
 *    A: Parallel data operations (arrays)
 *       Math-heavy calculations
 *       Fixed-size batch processing
 *       When compiler auto-vectorization insufficient
 *
 *    Q: Branch vs branchless code?
 *    A: Predictable: branch is fine
 *       Unpredictable: branchless better
 *       Measure to decide
 *
 * 9. Profiling Tools:
 *    - perf: CPU counters, cache misses
 *    - cachegrind: Cache simulation
 *    - VTune: Intel profiler
 *    - gprof: Function profiling
 *    - Custom: RDTSC for cycle counting
 *
 * 10. Best Practices:
 *     - Profile before optimizing
 *     - Optimize hot paths only
 *     - Measure impact of each change
 *     - Keep code readable
 *     - Document optimization reasons
 *     - Test on target hardware
 *     - Consider maintainability
 *
 * Performance Gains (Typical):
 * - Cache alignment: 2-10x
 * - Struct packing: 1.5-2x
 * - Branch prediction: 1.1-2x
 * - Prefetching: 1.2-1.5x
 * - SIMD: 2-8x
 * - SoA vs AoS: 1.5-3x
 *
 * Combined: 10-100x improvement possible!
 */

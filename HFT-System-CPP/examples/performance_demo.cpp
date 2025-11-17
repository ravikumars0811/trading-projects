/**
 * @file performance_demo.cpp
 * @brief Demonstration of HFT performance optimization techniques
 *
 * This example shows:
 * 1. CPU thread pinning and real-time priority
 * 2. SIMD-accelerated calculations
 * 3. Latency measurement with TSC
 * 4. Performance comparison (scalar vs SIMD)
 *
 * Compile:
 *   g++ -std=c++20 -O3 -march=native -mavx2 -pthread \
 *       performance_demo.cpp -o performance_demo
 *
 * Run:
 *   sudo ./performance_demo
 */

#include "../include/optimizations/cpu_optimizer.hpp"
#include "../include/optimizations/simd_pricing.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <thread>
#include <x86intrin.h>

using namespace hft::optimization;

// ============================================================================
// TSC Timer for Nanosecond Precision
// ============================================================================

class TSCTimer {
private:
    static double tsc_freq_ghz_;
    uint64_t start_tsc_;

public:
    static void calibrate() {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t tsc_start = __rdtsc();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        uint64_t tsc_end = __rdtsc();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start).count();

        tsc_freq_ghz_ = static_cast<double>(tsc_end - tsc_start) / duration_ns;
        std::cout << "TSC Frequency: " << tsc_freq_ghz_ << " GHz" << std::endl;
    }

    void start() {
        _mm_lfence();
        start_tsc_ = __rdtsc();
        _mm_lfence();
    }

    uint64_t elapsed_ns() const {
        _mm_lfence();
        uint64_t end_tsc = __rdtsc();
        _mm_lfence();

        return static_cast<uint64_t>((end_tsc - start_tsc_) / tsc_freq_ghz_);
    }
};

double TSCTimer::tsc_freq_ghz_ = 0.0;

// ============================================================================
// Performance Benchmarks
// ============================================================================

void benchmark_vwap() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "VWAP Calculation Benchmark" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    constexpr size_t N = 1'000'000;

    // Generate random test data
    std::vector<uint32_t> prices(N);
    std::vector<uint32_t> volumes(N);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> price_dist(10000, 11000);
    std::uniform_int_distribution<uint32_t> volume_dist(100, 10000);

    for (size_t i = 0; i < N; ++i) {
        prices[i] = price_dist(gen);
        volumes[i] = volume_dist(gen);
    }

    TSCTimer timer;

    // Scalar version
    timer.start();
    uint64_t total_value = 0;
    uint64_t total_volume = 0;
    for (size_t i = 0; i < N; ++i) {
        total_value += static_cast<uint64_t>(prices[i]) * volumes[i];
        total_volume += volumes[i];
    }
    double vwap_scalar = static_cast<double>(total_value) / total_volume;
    uint64_t scalar_ns = timer.elapsed_ns();

    // SIMD version
    timer.start();
    double vwap_simd = SIMDPricing::calculateVWAP_AVX2(prices.data(), volumes.data(), N);
    uint64_t simd_ns = timer.elapsed_ns();

    std::cout << "\nResults:" << std::endl;
    std::cout << "  Scalar VWAP: " << std::fixed << std::setprecision(2) << vwap_scalar << std::endl;
    std::cout << "  SIMD VWAP:   " << vwap_simd << std::endl;
    std::cout << "\nPerformance:" << std::endl;
    std::cout << "  Scalar time: " << scalar_ns << " ns (" << scalar_ns / 1000 << " μs)" << std::endl;
    std::cout << "  SIMD time:   " << simd_ns << " ns (" << simd_ns / 1000 << " μs)" << std::endl;
    std::cout << "  Speedup:     " << std::setprecision(1) << (static_cast<double>(scalar_ns) / simd_ns) << "x" << std::endl;
}

void benchmark_sma() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Simple Moving Average Benchmark" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    constexpr size_t N = 1'000'000;

    // Generate random price data
    std::vector<float> prices(N);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(100.0f, 110.0f);

    for (size_t i = 0; i < N; ++i) {
        prices[i] = dist(gen);
    }

    TSCTimer timer;

    // SIMD version
    timer.start();
    double sma_simd = SIMDPricing::calculateSMA_AVX2(prices.data(), N);
    uint64_t simd_ns = timer.elapsed_ns();

    // Scalar version
    timer.start();
    double sum = 0.0;
    for (size_t i = 0; i < N; ++i) {
        sum += prices[i];
    }
    double sma_scalar = sum / N;
    uint64_t scalar_ns = timer.elapsed_ns();

    std::cout << "\nResults:" << std::endl;
    std::cout << "  Scalar SMA: " << std::fixed << std::setprecision(4) << sma_scalar << std::endl;
    std::cout << "  SIMD SMA:   " << sma_simd << std::endl;
    std::cout << "\nPerformance:" << std::endl;
    std::cout << "  Scalar time: " << scalar_ns << " ns (" << scalar_ns / 1000 << " μs)" << std::endl;
    std::cout << "  SIMD time:   " << simd_ns << " ns (" << simd_ns / 1000 << " μs)" << std::endl;
    std::cout << "  Speedup:     " << std::setprecision(1) << (static_cast<double>(scalar_ns) / simd_ns) << "x" << std::endl;
}

void benchmark_statistics() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Statistical Calculations Benchmark" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    constexpr size_t N = 1'000'000;

    std::vector<float> data(N);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(100.0f, 10.0f);

    for (size_t i = 0; i < N; ++i) {
        data[i] = dist(gen);
    }

    TSCTimer timer;

    // Variance calculation
    timer.start();
    double variance_simd = SIMDPricing::calculateVariance_AVX2(data.data(), N);
    uint64_t variance_ns = timer.elapsed_ns();

    // Min/Max calculation
    timer.start();
    auto [min_val, max_val] = SIMDPricing::calculateMinMax_AVX2(data.data(), N);
    uint64_t minmax_ns = timer.elapsed_ns();

    std::cout << "\nResults:" << std::endl;
    std::cout << "  Variance: " << std::fixed << std::setprecision(4) << variance_simd << std::endl;
    std::cout << "  Std Dev:  " << std::sqrt(variance_simd) << std::endl;
    std::cout << "  Min:      " << min_val << std::endl;
    std::cout << "  Max:      " << max_val << std::endl;
    std::cout << "\nPerformance:" << std::endl;
    std::cout << "  Variance calc: " << variance_ns / 1000 << " μs" << std::endl;
    std::cout << "  Min/Max calc:  " << minmax_ns / 1000 << " μs" << std::endl;
}

// ============================================================================
// Simulated Trading Loop
// ============================================================================

void simulated_trading_loop() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Simulated Trading Loop with Latency Tracking" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    constexpr size_t NUM_ORDERS = 100'000;

    std::vector<uint64_t> latencies;
    latencies.reserve(NUM_ORDERS);

    TSCTimer timer;

    // Simulate order processing
    for (size_t i = 0; i < NUM_ORDERS; ++i) {
        timer.start();

        // Simulate order processing work
        volatile int result = 0;
        for (int j = 0; j < 100; ++j) {
            result += j;
        }

        latencies.push_back(timer.elapsed_ns());
    }

    // Calculate statistics
    std::sort(latencies.begin(), latencies.end());

    uint64_t total = 0;
    for (uint64_t lat : latencies) {
        total += lat;
    }

    std::cout << "\nLatency Statistics (nanoseconds):" << std::endl;
    std::cout << "  Orders processed: " << NUM_ORDERS << std::endl;
    std::cout << "  Min:    " << latencies.front() << " ns" << std::endl;
    std::cout << "  Max:    " << latencies.back() << " ns" << std::endl;
    std::cout << "  Mean:   " << total / NUM_ORDERS << " ns" << std::endl;
    std::cout << "  Median: " << latencies[NUM_ORDERS / 2] << " ns" << std::endl;
    std::cout << "  P95:    " << latencies[NUM_ORDERS * 95 / 100] << " ns" << std::endl;
    std::cout << "  P99:    " << latencies[NUM_ORDERS * 99 / 100] << " ns" << std::endl;
    std::cout << "  P99.9:  " << latencies[NUM_ORDERS * 999 / 1000] << " ns" << std::endl;
}

// ============================================================================
// Thread Performance Demonstration
// ============================================================================

void threaded_benchmark_worker(int cpu_id, bool use_optimization) {
    if (use_optimization) {
        CPUOptimizer::optimizeThread(cpu_id, 99);
    }

    constexpr size_t ITERATIONS = 10'000'000;

    auto start = std::chrono::high_resolution_clock::now();

    volatile uint64_t sum = 0;
    for (size_t i = 0; i < ITERATIONS; ++i) {
        sum += i;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "  CPU " << cpu_id
              << " (optimized: " << (use_optimization ? "yes" : "no")
              << "): " << duration << " μs" << std::endl;
}

void demonstrate_thread_optimization() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Thread Optimization Comparison" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    std::cout << "\nWithout optimization:" << std::endl;
    std::thread t1(threaded_benchmark_worker, 0, false);
    std::thread t2(threaded_benchmark_worker, 1, false);
    t1.join();
    t2.join();

    std::cout << "\nWith CPU pinning and RT priority:" << std::endl;
    std::thread t3(threaded_benchmark_worker, 2, true);
    std::thread t4(threaded_benchmark_worker, 3, true);
    t3.join();
    t4.join();
}

// ============================================================================
// Main Function
// ============================================================================

int main() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "HFT PERFORMANCE OPTIMIZATION DEMO" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // Check if running as root
    if (geteuid() != 0) {
        std::cout << "\n" << std::string(70, '-') << std::endl;
        std::cout << "WARNING: Not running as root" << std::endl;
        std::cout << "For optimal performance, run with: sudo ./performance_demo" << std::endl;
        std::cout << std::string(70, '-') << std::endl;
    }

    // Display system optimization status
    CPUOptimizer::printOptimizationStatus();

    // Calibrate TSC timer
    std::cout << "\nCalibrating TSC timer..." << std::endl;
    TSCTimer::calibrate();

    // Check SIMD support
    std::cout << "\nSIMD Support:" << std::endl;
    std::cout << "  AVX2: " << (SIMDPricing::isAVX2Supported() ? "YES" : "NO") << std::endl;

    // Run benchmarks
    benchmark_vwap();
    benchmark_sma();
    benchmark_statistics();
    simulated_trading_loop();
    demonstrate_thread_optimization();

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "DEMO COMPLETE" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    std::cout << "\nKey Takeaways:" << std::endl;
    std::cout << "1. SIMD operations provide 4-8x speedup for bulk calculations" << std::endl;
    std::cout << "2. CPU pinning reduces context switches and improves consistency" << std::endl;
    std::cout << "3. TSC-based timing provides nanosecond precision for latency tracking" << std::endl;
    std::cout << "4. Real-time priority ensures critical threads get CPU time" << std::endl;
    std::cout << "\nFor production HFT systems, also consider:" << std::endl;
    std::cout << "- CPU isolation (isolcpus kernel parameter)" << std::endl;
    std::cout << "- Huge pages for memory allocations" << std::endl;
    std::cout << "- Lock-free data structures" << std::endl;
    std::cout << "- Network kernel bypass (DPDK, AF_XDP)" << std::endl;
    std::cout << "- Memory pools to eliminate malloc/free" << std::endl;

    return 0;
}

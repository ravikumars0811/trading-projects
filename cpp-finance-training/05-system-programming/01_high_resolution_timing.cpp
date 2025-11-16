/*
 * High-Resolution Timing for HFT
 * Critical for latency measurement and timestamping
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>

#ifdef __x86_64__
#include <x86intrin.h>  // For RDTSC
#endif

// RDTSC-based timestamp (fastest, but CPU-specific)
class TSCClock {
private:
    static double tsc_frequency_;
    static bool calibrated_;

public:
    static void calibrate() {
        // Calibrate TSC frequency by comparing to std::chrono
        auto start_chrono = std::chrono::high_resolution_clock::now();
        uint64_t start_tsc = rdtsc();

        // Wait for a measurable duration
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto end_chrono = std::chrono::high_resolution_clock::now();
        uint64_t end_tsc = rdtsc();

        auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end_chrono - start_chrono).count();

        tsc_frequency_ = (end_tsc - start_tsc) / (duration_ns / 1e9);
        calibrated_ = true;

        std::cout << "TSC Frequency: " << (tsc_frequency_ / 1e9) << " GHz" << std::endl;
    }

    static uint64_t rdtsc() {
#ifdef __x86_64__
        return __rdtsc();
#else
        // Fallback for non-x86
        return std::chrono::high_resolution_clock::now().time_since_epoch().count();
#endif
    }

    static uint64_t now() {
        return rdtsc();
    }

    static double toNanoseconds(uint64_t tsc) {
        if (!calibrated_) calibrate();
        return (tsc / tsc_frequency_) * 1e9;
    }

    static double elapsedNanoseconds(uint64_t start, uint64_t end) {
        if (!calibrated_) calibrate();
        return toNanoseconds(end - start);
    }
};

double TSCClock::tsc_frequency_ = 0.0;
bool TSCClock::calibrated_ = false;

// Simple timer class
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_;
    std::string name_;

public:
    Timer(const std::string& name = "Timer")
        : start_(std::chrono::high_resolution_clock::now()), name_(name) {}

    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_);
        std::cout << name_ << " took: " << duration.count() << " ns" << std::endl;
    }

    void reset() {
        start_ = std::chrono::high_resolution_clock::now();
    }

    long long elapsedNs() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
    }

    double elapsedUs() const {
        return elapsedNs() / 1000.0;
    }

    double elapsedMs() const {
        return elapsedNs() / 1000000.0;
    }
};

// Latency statistics
struct LatencyStats {
    std::vector<double> samples;

    void addSample(double latency_ns) {
        samples.push_back(latency_ns);
    }

    void calculate() {
        if (samples.empty()) return;

        std::sort(samples.begin(), samples.end());

        double sum = 0.0;
        for (double s : samples) sum += s;

        double mean = sum / samples.size();

        double variance = 0.0;
        for (double s : samples) {
            variance += (s - mean) * (s - mean);
        }
        variance /= samples.size();
        double stddev = std::sqrt(variance);

        std::cout << "\nLatency Statistics (" << samples.size() << " samples):" << std::endl;
        std::cout << "  Min:     " << std::fixed << std::setprecision(2)
                  << samples.front() << " ns" << std::endl;
        std::cout << "  Max:     " << samples.back() << " ns" << std::endl;
        std::cout << "  Mean:    " << mean << " ns" << std::endl;
        std::cout << "  Median:  " << samples[samples.size() / 2] << " ns" << std::endl;
        std::cout << "  StdDev:  " << stddev << " ns" << std::endl;
        std::cout << "  95th:    " << samples[samples.size() * 95 / 100] << " ns" << std::endl;
        std::cout << "  99th:    " << samples[samples.size() * 99 / 100] << " ns" << std::endl;
        std::cout << "  99.9th:  " << samples[samples.size() * 999 / 1000] << " ns" << std::endl;
    }
};

// Benchmark different timing methods
void benchmarkTimingMethods() {
    std::cout << "\n=== Timing Method Benchmark ===" << std::endl;

    constexpr int ITERATIONS = 1000000;

    // Benchmark std::chrono::high_resolution_clock
    {
        Timer timer("std::chrono overhead");
        for (int i = 0; i < ITERATIONS; ++i) {
            volatile auto t = std::chrono::high_resolution_clock::now();
            (void)t;
        }
    }

    // Benchmark RDTSC
    {
        Timer timer("RDTSC overhead");
        for (int i = 0; i < ITERATIONS; ++i) {
            volatile auto t = TSCClock::rdtsc();
            (void)t;
        }
    }

    std::cout << "\nRDTSC is typically 2-5x faster than std::chrono" << std::endl;
}

// Measure function execution time
template<typename Func>
void measureLatency(const std::string& name, Func&& func, int iterations = 10000) {
    LatencyStats stats;

    // Warmup
    for (int i = 0; i < 1000; ++i) {
        func();
    }

    // Measure
    for (int i = 0; i < iterations; ++i) {
        auto start = TSCClock::rdtsc();
        func();
        auto end = TSCClock::rdtsc();

        double latency_ns = TSCClock::elapsedNanoseconds(start, end);
        stats.addSample(latency_ns);
    }

    std::cout << "\n--- " << name << " ---";
    stats.calculate();
}

// Simulate order processing
struct Order {
    uint64_t id;
    double price;
    int quantity;
};

void processOrder(const Order& order) {
    volatile double result = order.price * order.quantity;
    (void)result;
}

// Demonstrate timestamp generation
void demonstrateTimestamps() {
    std::cout << "\n=== Timestamp Generation ===" << std::endl;

    // Get current time in various formats
    auto now = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();

    std::cout << "std::chrono timestamp (ns since epoch): " << now_ns << std::endl;

    uint64_t tsc = TSCClock::rdtsc();
    std::cout << "RDTSC timestamp: " << tsc << std::endl;
    std::cout << "RDTSC in nanoseconds: " << std::fixed << std::setprecision(0)
              << TSCClock::toNanoseconds(tsc) << std::endl;

    // Convert to time_t for human-readable
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::cout << "Human readable: " << std::ctime(&now_time_t);
}

// Measure clock resolution
void measureClockResolution() {
    std::cout << "\n=== Clock Resolution ===" << std::endl;

    // Measure minimum measurable time difference
    std::vector<long long> diffs;
    for (int i = 0; i < 10000; ++i) {
        auto t1 = std::chrono::high_resolution_clock::now();
        auto t2 = std::chrono::high_resolution_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        if (diff > 0) {
            diffs.push_back(diff);
        }
    }

    if (!diffs.empty()) {
        std::sort(diffs.begin(), diffs.end());
        std::cout << "std::chrono::high_resolution_clock minimum resolution: "
                  << diffs.front() << " ns" << std::endl;
    }

    // RDTSC resolution
    std::vector<uint64_t> tsc_diffs;
    for (int i = 0; i < 10000; ++i) {
        auto t1 = TSCClock::rdtsc();
        auto t2 = TSCClock::rdtsc();
        if (t2 > t1) {
            tsc_diffs.push_back(t2 - t1);
        }
    }

    if (!tsc_diffs.empty()) {
        std::sort(tsc_diffs.begin(), tsc_diffs.end());
        std::cout << "RDTSC minimum ticks: " << tsc_diffs.front() << " cycles" << std::endl;
        std::cout << "RDTSC resolution: " << std::fixed << std::setprecision(2)
                  << TSCClock::toNanoseconds(tsc_diffs.front()) << " ns" << std::endl;
    }
}

int main() {
    std::cout << "=== High-Resolution Timing for HFT ===" << std::endl;

    // Calibrate TSC
    std::cout << "\n--- Calibrating TSC ---" << std::endl;
    TSCClock::calibrate();

    // Demonstrate timestamps
    demonstrateTimestamps();

    // Measure clock resolution
    measureClockResolution();

    // Benchmark timing methods
    benchmarkTimingMethods();

    // Measure simple operations
    measureLatency("Simple multiplication", []() {
        volatile double x = 150.25 * 100;
        (void)x;
    });

    measureLatency("Order processing", []() {
        Order order{1001, 150.25, 100};
        processOrder(order);
    });

    // Measure context switch time
    std::cout << "\n--- Context Switch Measurement ---" << std::endl;
    {
        Timer timer("Context switch (yield)");
        for (int i = 0; i < 1000; ++i) {
            std::this_thread::yield();
        }
    }
    std::cout << "Context switch is VERY expensive (1000s of ns)" << std::endl;

    // Best practices
    std::cout << "\n=== Best Practices for HFT Timing ===" << std::endl;
    std::cout << "1. Use RDTSC for ultra-low latency timestamps" << std::endl;
    std::cout << "2. Calibrate TSC frequency at startup" << std::endl;
    std::cout << "3. Use std::chrono for wall-clock time" << std::endl;
    std::cout << "4. Avoid system calls in hot paths" << std::endl;
    std::cout << "5. Measure with release builds (-O3)" << std::endl;
    std::cout << "6. Report percentiles (99th, 99.9th), not just average" << std::endl;
    std::cout << "7. Warm up before measuring" << std::endl;
    std::cout << "8. Pin threads to specific cores" << std::endl;

    std::cout << "\n=== RDTSC Caveats ===" << std::endl;
    std::cout << "1. May not be synchronized across cores (use invariant TSC)" << std::endl;
    std::cout << "2. Can be affected by CPU frequency scaling" << std::endl;
    std::cout << "3. Not suitable for absolute wall-clock time" << std::endl;
    std::cout << "4. Instruction reordering can affect accuracy" << std::endl;
    std::cout << "5. Use RDTSCP or fences if needed" << std::endl;

    std::cout << "\n=== Typical HFT Latencies ===" << std::endl;
    std::cout << "L1 cache access:      ~1 ns" << std::endl;
    std::cout << "L2 cache access:      ~3 ns" << std::endl;
    std::cout << "L3 cache access:      ~10-20 ns" << std::endl;
    std::cout << "RAM access:           ~100 ns" << std::endl;
    std::cout << "Context switch:       ~1-10 μs" << std::endl;
    std::cout << "System call:          ~100-500 ns" << std::endl;
    std::cout << "Network (datacenter): ~10-100 μs" << std::endl;
    std::cout << "Disk I/O:             ~1-10 ms" << std::endl;

    return 0;
}

/*
 * Key Takeaways:
 * 1. RDTSC is fastest but has caveats
 * 2. std::chrono is portable and safe
 * 3. Always measure with release builds
 * 4. Report percentiles, not just averages
 * 5. Context switches are expensive (avoid!)
 *
 * HFT Timing Requirements:
 * - Nanosecond precision for tick-to-trade
 * - Synchronized clocks across machines (PTP)
 * - Minimal overhead for timestamping
 * - Accurate latency measurement
 *
 * Compilation:
 * g++ -std=c++17 -O3 -march=native 01_high_resolution_timing.cpp -o timing
 *
 * For even better accuracy:
 * - Use RDTSCP (serializing RDTSC)
 * - Pin process to specific CPU core
 * - Disable frequency scaling
 * - Use isolcpus to dedicate cores
 */

#pragma once

#include <chrono>
#include <cstdint>

namespace hft {
namespace core {

/**
 * High-resolution timer for latency measurement
 * Uses TSC (Time Stamp Counter) for ultra-low overhead
 */
class Timer {
public:
    using clock_type = std::chrono::high_resolution_clock;
    using time_point = clock_type::time_point;
    using nanoseconds = std::chrono::nanoseconds;
    using microseconds = std::chrono::microseconds;

    Timer() : start_time_(now()) {}

    static time_point now() {
        return clock_type::now();
    }

    static uint64_t rdtsc() {
        #if defined(__x86_64__) || defined(__i386__)
        uint32_t lo, hi;
        __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
        return (static_cast<uint64_t>(hi) << 32) | lo;
        #else
        return std::chrono::high_resolution_clock::now().time_since_epoch().count();
        #endif
    }

    void reset() {
        start_time_ = now();
    }

    template<typename Duration = nanoseconds>
    int64_t elapsed() const {
        return std::chrono::duration_cast<Duration>(now() - start_time_).count();
    }

    int64_t elapsed_ns() const {
        return elapsed<nanoseconds>();
    }

    int64_t elapsed_us() const {
        return elapsed<microseconds>();
    }

    static uint64_t timestamp_ns() {
        return std::chrono::duration_cast<nanoseconds>(
            clock_type::now().time_since_epoch()).count();
    }

private:
    time_point start_time_;
};

/**
 * RAII-style latency measurement
 */
class ScopedTimer {
public:
    ScopedTimer(int64_t& result) : result_(result), timer_() {}

    ~ScopedTimer() {
        result_ = timer_.elapsed_ns();
    }

private:
    int64_t& result_;
    Timer timer_;
};

} // namespace core
} // namespace hft

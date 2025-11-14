#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <vector>

namespace hft {
namespace metrics {

/**
 * Latency statistics
 */
struct LatencyStats {
    int64_t min_ns = 0;
    int64_t max_ns = 0;
    int64_t mean_ns = 0;
    int64_t median_ns = 0;
    int64_t p95_ns = 0;
    int64_t p99_ns = 0;
    uint64_t count = 0;
};

/**
 * Latency Tracker
 * Tracks latency measurements with percentile calculations
 */
template<size_t MaxSamples = 10000>
class LatencyTracker {
public:
    LatencyTracker() : count_(0) {}

    void record(int64_t latency_ns) {
        if (count_ < MaxSamples) {
            samples_[count_++] = latency_ns;
        } else {
            // Ring buffer behavior
            samples_[count_ % MaxSamples] = latency_ns;
            count_++;
        }
    }

    LatencyStats getStats() const {
        if (count_ == 0) {
            return LatencyStats{};
        }

        size_t actual_count = std::min(count_, MaxSamples);
        std::vector<int64_t> sorted_samples(samples_.begin(),
                                            samples_.begin() + actual_count);
        std::sort(sorted_samples.begin(), sorted_samples.end());

        LatencyStats stats;
        stats.count = count_;
        stats.min_ns = sorted_samples.front();
        stats.max_ns = sorted_samples.back();

        // Calculate mean
        int64_t sum = 0;
        for (size_t i = 0; i < actual_count; ++i) {
            sum += sorted_samples[i];
        }
        stats.mean_ns = sum / actual_count;

        // Calculate percentiles
        stats.median_ns = sorted_samples[actual_count / 2];
        stats.p95_ns = sorted_samples[static_cast<size_t>(actual_count * 0.95)];
        stats.p99_ns = sorted_samples[static_cast<size_t>(actual_count * 0.99)];

        return stats;
    }

    void reset() {
        count_ = 0;
    }

private:
    std::array<int64_t, MaxSamples> samples_;
    size_t count_;
};

} // namespace metrics
} // namespace hft

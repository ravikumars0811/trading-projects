#pragma once

#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace hft {
namespace optimization {

/**
 * @brief CPU and thread optimization utilities for HFT systems
 *
 * Provides functionality for:
 * - Thread pinning to specific CPUs
 * - Real-time scheduling (SCHED_FIFO)
 * - CPU isolation verification
 * - Cache line alignment utilities
 */
class CPUOptimizer {
public:
    /**
     * @brief Pin current thread to specific CPU core
     * @param cpu_id CPU core ID (0-indexed)
     * @return true if successful
     */
    static bool pinCurrentThread(int cpu_id) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id, &cpuset);

        pthread_t current_thread = pthread_self();
        int result = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);

        if (result != 0) {
            std::cerr << "Failed to pin thread to CPU " << cpu_id
                     << ": " << strerror(result) << std::endl;
            return false;
        }

        std::cout << "Thread pinned to CPU " << cpu_id << std::endl;
        return true;
    }

    /**
     * @brief Set real-time priority for current thread
     * @param priority Priority level (1-99, higher = more priority)
     * @return true if successful
     */
    static bool setRealtimePriority(int priority = 99) {
        struct sched_param param;
        param.sched_priority = priority;

        pthread_t current_thread = pthread_self();
        int result = pthread_setschedparam(current_thread, SCHED_FIFO, &param);

        if (result != 0) {
            std::cerr << "Failed to set real-time priority: " << strerror(result)
                     << std::endl;
            std::cerr << "Try running with sudo or setting CAP_SYS_NICE capability"
                     << std::endl;
            return false;
        }

        std::cout << "Real-time priority set to " << priority << std::endl;
        return true;
    }

    /**
     * @brief Pin thread and set real-time priority (combined)
     * @param cpu_id CPU core ID
     * @param priority Real-time priority (1-99)
     * @return true if both operations successful
     */
    static bool optimizeThread(int cpu_id, int priority = 99) {
        bool pinned = pinCurrentThread(cpu_id);
        bool rt_set = setRealtimePriority(priority);
        return pinned && rt_set;
    }

    /**
     * @brief Get current CPU core that thread is running on
     * @return CPU core ID
     */
    static int getCurrentCPU() {
        return sched_getcpu();
    }

    /**
     * @brief Verify CPU isolation configuration
     * @return true if CPU isolation is properly configured
     */
    static bool verifyCPUIsolation() {
        std::ifstream cmdline("/proc/cmdline");
        std::string line;
        std::getline(cmdline, line);

        bool has_isolcpus = line.find("isolcpus=") != std::string::npos;
        bool has_nohz_full = line.find("nohz_full=") != std::string::npos;
        bool has_rcu_nocbs = line.find("rcu_nocbs=") != std::string::npos;

        std::cout << "CPU Isolation Status:" << std::endl;
        std::cout << "  isolcpus: " << (has_isolcpus ? "YES" : "NO") << std::endl;
        std::cout << "  nohz_full: " << (has_nohz_full ? "YES" : "NO") << std::endl;
        std::cout << "  rcu_nocbs: " << (has_rcu_nocbs ? "YES" : "NO") << std::endl;

        return has_isolcpus && has_nohz_full && has_rcu_nocbs;
    }

    /**
     * @brief Check and display CPU frequency governor
     */
    static void checkCPUGovernor() {
        std::ifstream governor("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
        std::string current_governor;
        std::getline(governor, current_governor);

        std::cout << "CPU Governor: " << current_governor << std::endl;
        if (current_governor != "performance") {
            std::cout << "WARNING: CPU governor is not set to 'performance'" << std::endl;
            std::cout << "Run: echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor"
                     << std::endl;
        }
    }

    /**
     * @brief Disable address space layout randomization for deterministic performance
     */
    static void disableASLR() {
        // Note: This typically requires root privileges
        // Can also be done via: echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
        std::cout << "To disable ASLR, run:" << std::endl;
        std::cout << "  echo 0 | sudo tee /proc/sys/kernel/randomize_va_space" << std::endl;
    }

    /**
     * @brief Print comprehensive optimization status
     */
    static void printOptimizationStatus() {
        std::cout << "\n=== HFT System Optimization Status ===" << std::endl;
        std::cout << "Current CPU: " << getCurrentCPU() << std::endl;
        std::cout << "Total CPUs: " << sysconf(_SC_NPROCESSORS_ONLN) << std::endl;
        checkCPUGovernor();
        verifyCPUIsolation();
        std::cout << "====================================\n" << std::endl;
    }
};

/**
 * @brief RAII wrapper for thread optimization
 *
 * Automatically pins thread and sets priority on construction
 */
class ScopedThreadOptimizer {
private:
    int original_cpu_;
    bool success_;

public:
    ScopedThreadOptimizer(int cpu_id, int priority = 99)
        : original_cpu_(CPUOptimizer::getCurrentCPU())
        , success_(CPUOptimizer::optimizeThread(cpu_id, priority)) {
    }

    ~ScopedThreadOptimizer() {
        if (success_) {
            // Optionally restore original CPU affinity
            // CPUOptimizer::pinCurrentThread(original_cpu_);
        }
    }

    bool isOptimized() const { return success_; }
};

/**
 * @brief Cache line size detection and alignment helpers
 */
class CacheOptimizer {
public:
    static constexpr size_t CACHE_LINE_SIZE = 64;  // x86-64 typical

    /**
     * @brief Get L1 cache line size from system
     */
    static size_t getL1CacheLineSize() {
        std::ifstream cache_info("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size");
        size_t size;
        cache_info >> size;
        return size;
    }

    /**
     * @brief Prefetch data into cache
     * @param addr Address to prefetch
     * @param locality 0=non-temporal, 3=high temporal locality
     */
    static void prefetch(const void* addr, int locality = 3) {
        __builtin_prefetch(addr, 0, locality);
    }

    /**
     * @brief Prefetch for write
     */
    static void prefetchWrite(void* addr, int locality = 3) {
        __builtin_prefetch(addr, 1, locality);
    }
};

} // namespace optimization
} // namespace hft

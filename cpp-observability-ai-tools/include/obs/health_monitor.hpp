#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <chrono>
#include <mutex>
#include <nlohmann/json.hpp>

namespace obs {

enum class HealthStatus {
    HEALTHY,
    DEGRADED,
    UNHEALTHY,
    UNKNOWN
};

struct HealthCheck {
    std::string name;
    std::string component;
    HealthStatus status;
    std::string message;
    std::chrono::system_clock::time_point last_check;
    std::chrono::milliseconds check_duration;
    std::unordered_map<std::string, std::string> metadata;
};

using HealthCheckFunction = std::function<HealthCheck()>;

class HealthMonitor {
public:
    static HealthMonitor& instance();

    // Register health checks
    void register_check(const std::string& name,
                       const std::string& component,
                       HealthCheckFunction check_fn,
                       std::chrono::seconds interval = std::chrono::seconds(30));

    // Manual check execution
    HealthCheck execute_check(const std::string& name);
    std::vector<HealthCheck> execute_all_checks();

    // Query operations
    HealthStatus get_overall_status() const;
    std::vector<HealthCheck> get_all_checks() const;
    std::vector<HealthCheck> get_checks_by_component(const std::string& component) const;
    std::vector<HealthCheck> get_unhealthy_checks() const;

    // Export
    nlohmann::json export_json() const;

    // Background monitoring
    void start_monitoring();
    void stop_monitoring();

private:
    HealthMonitor() = default;
    ~HealthMonitor();
    HealthMonitor(const HealthMonitor&) = delete;
    HealthMonitor& operator=(const HealthMonitor&) = delete;

    struct RegisteredCheck {
        std::string name;
        std::string component;
        HealthCheckFunction function;
        std::chrono::seconds interval;
        std::chrono::steady_clock::time_point last_run;
    };

    mutable std::mutex mutex_;
    std::vector<RegisteredCheck> registered_checks_;
    std::unordered_map<std::string, HealthCheck> latest_results_;
    std::atomic<bool> monitoring_active_{false};
    std::unique_ptr<std::thread> monitoring_thread_;

    void monitoring_loop();
};

// Helper macros for common health checks
#define HEALTH_CHECK_CPU() obs::HealthMonitor::instance().register_check(\
    "cpu_usage", "system", []() -> obs::HealthCheck { \
        /* CPU check implementation */ \
        return {}; \
    })

} // namespace obs

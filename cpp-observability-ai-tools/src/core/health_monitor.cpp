#include "obs/health_monitor.hpp"
#include <algorithm>

namespace obs {

HealthMonitor& HealthMonitor::instance() {
    static HealthMonitor instance;
    return instance;
}

HealthMonitor::~HealthMonitor() {
    stop_monitoring();
}

void HealthMonitor::register_check(const std::string& name,
                                   const std::string& component,
                                   HealthCheckFunction check_fn,
                                   std::chrono::seconds interval) {
    std::lock_guard<std::mutex> lock(mutex_);

    RegisteredCheck check;
    check.name = name;
    check.component = component;
    check.function = std::move(check_fn);
    check.interval = interval;
    check.last_run = std::chrono::steady_clock::now();

    registered_checks_.push_back(std::move(check));
}

HealthCheck HealthMonitor::execute_check(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = std::find_if(registered_checks_.begin(), registered_checks_.end(),
                          [&name](const RegisteredCheck& c) { return c.name == name; });

    if (it != registered_checks_.end()) {
        auto start = std::chrono::steady_clock::now();
        HealthCheck result = it->function();
        auto end = std::chrono::steady_clock::now();

        result.check_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        result.last_check = std::chrono::system_clock::now();

        latest_results_[name] = result;
        it->last_run = std::chrono::steady_clock::now();

        return result;
    }

    HealthCheck error_result;
    error_result.name = name;
    error_result.status = HealthStatus::UNKNOWN;
    error_result.message = "Check not found";
    return error_result;
}

std::vector<HealthCheck> HealthMonitor::execute_all_checks() {
    std::vector<HealthCheck> results;

    std::unique_lock<std::mutex> lock(mutex_);
    auto checks_copy = registered_checks_;
    lock.unlock();

    for (const auto& check : checks_copy) {
        results.push_back(execute_check(check.name));
    }

    return results;
}

HealthStatus HealthMonitor::get_overall_status() const {
    std::lock_guard<std::mutex> lock(mutex_);

    if (latest_results_.empty()) {
        return HealthStatus::UNKNOWN;
    }

    bool has_unhealthy = false;
    bool has_degraded = false;

    for (const auto& [name, check] : latest_results_) {
        if (check.status == HealthStatus::UNHEALTHY) {
            has_unhealthy = true;
        } else if (check.status == HealthStatus::DEGRADED) {
            has_degraded = true;
        }
    }

    if (has_unhealthy) return HealthStatus::UNHEALTHY;
    if (has_degraded) return HealthStatus::DEGRADED;
    return HealthStatus::HEALTHY;
}

std::vector<HealthCheck> HealthMonitor::get_all_checks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<HealthCheck> checks;

    for (const auto& [name, check] : latest_results_) {
        checks.push_back(check);
    }

    return checks;
}

std::vector<HealthCheck> HealthMonitor::get_checks_by_component(const std::string& component) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<HealthCheck> checks;

    for (const auto& [name, check] : latest_results_) {
        if (check.component == component) {
            checks.push_back(check);
        }
    }

    return checks;
}

std::vector<HealthCheck> HealthMonitor::get_unhealthy_checks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<HealthCheck> checks;

    for (const auto& [name, check] : latest_results_) {
        if (check.status == HealthStatus::UNHEALTHY || check.status == HealthStatus::DEGRADED) {
            checks.push_back(check);
        }
    }

    return checks;
}

nlohmann::json HealthMonitor::export_json() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json result;

    result["overall_status"] = static_cast<int>(get_overall_status());
    result["checks"] = nlohmann::json::array();

    for (const auto& [name, check] : latest_results_) {
        nlohmann::json check_json;
        check_json["name"] = check.name;
        check_json["component"] = check.component;
        check_json["status"] = static_cast<int>(check.status);
        check_json["message"] = check.message;
        check_json["check_duration_ms"] = check.check_duration.count();
        check_json["metadata"] = check.metadata;

        result["checks"].push_back(check_json);
    }

    return result;
}

void HealthMonitor::start_monitoring() {
    if (monitoring_active_.exchange(true)) {
        return; // Already monitoring
    }

    monitoring_thread_ = std::make_unique<std::thread>([this]() {
        monitoring_loop();
    });
}

void HealthMonitor::stop_monitoring() {
    if (!monitoring_active_.exchange(false)) {
        return; // Not monitoring
    }

    if (monitoring_thread_ && monitoring_thread_->joinable()) {
        monitoring_thread_->join();
    }
}

void HealthMonitor::monitoring_loop() {
    while (monitoring_active_.load()) {
        auto now = std::chrono::steady_clock::now();

        std::unique_lock<std::mutex> lock(mutex_);
        auto checks_copy = registered_checks_;
        lock.unlock();

        for (auto& check : checks_copy) {
            if (now - check.last_run >= check.interval) {
                execute_check(check.name);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

} // namespace obs

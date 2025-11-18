#include "obs/health_monitor.hpp"
#include <cassert>
#include <iostream>

void test_health_check() {
    std::cout << "Testing health check registration... ";

    auto& health = obs::HealthMonitor::instance();

    health.register_check("test_check", "test_component", []() -> obs::HealthCheck {
        obs::HealthCheck check;
        check.name = "test_check";
        check.component = "test_component";
        check.status = obs::HealthStatus::HEALTHY;
        check.message = "All systems operational";
        return check;
    });

    auto result = health.execute_check("test_check");
    assert(result.name == "test_check");
    assert(result.component == "test_component");
    assert(result.status == obs::HealthStatus::HEALTHY);

    std::cout << "PASSED\n";
}

void test_overall_status() {
    std::cout << "Testing overall health status... ";

    auto& health = obs::HealthMonitor::instance();

    health.register_check("healthy_check", "system", []() -> obs::HealthCheck {
        obs::HealthCheck check;
        check.name = "healthy_check";
        check.component = "system";
        check.status = obs::HealthStatus::HEALTHY;
        check.message = "OK";
        return check;
    });

    health.execute_check("healthy_check");

    auto status = health.get_overall_status();
    assert(status == obs::HealthStatus::HEALTHY || status == obs::HealthStatus::UNKNOWN);

    std::cout << "PASSED\n";
}

void test_export() {
    std::cout << "Testing health export... ";

    auto& health = obs::HealthMonitor::instance();
    auto json = health.export_json();

    assert(json.contains("overall_status"));
    assert(json.contains("checks"));
    assert(json["checks"].is_array());

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "Running HealthMonitor Tests\n";
    std::cout << "============================\n";

    test_health_check();
    test_overall_status();
    test_export();

    std::cout << "\nAll tests passed!\n";
    return 0;
}

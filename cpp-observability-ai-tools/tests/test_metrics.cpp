#include "obs/metrics_collector.hpp"
#include <cassert>
#include <iostream>

void test_counter() {
    std::cout << "Testing counter metrics... ";

    auto& metrics = obs::MetricsCollector::instance();
    metrics.reset();

    metrics.increment("test_counter", 1.0);
    metrics.increment("test_counter", 2.0);

    auto all_metrics = metrics.get_metrics_by_name("test_counter");
    assert(all_metrics.size() == 2);
    assert(all_metrics[0].value == 1.0);
    assert(all_metrics[1].value == 2.0);

    std::cout << "PASSED\n";
}

void test_gauge() {
    std::cout << "Testing gauge metrics... ";

    auto& metrics = obs::MetricsCollector::instance();
    metrics.reset();

    metrics.set_gauge("test_gauge", 42.0);
    metrics.set_gauge("test_gauge", 100.0);

    auto all_metrics = metrics.get_metrics_by_name("test_gauge");
    assert(all_metrics.size() == 2);
    assert(all_metrics[1].value == 100.0);

    std::cout << "PASSED\n";
}

void test_histogram() {
    std::cout << "Testing histogram metrics... ";

    auto& metrics = obs::MetricsCollector::instance();
    metrics.reset();

    metrics.record_histogram("test_histogram", 10.5);
    metrics.record_histogram("test_histogram", 20.5);
    metrics.record_histogram("test_histogram", 30.5);

    auto all_metrics = metrics.get_metrics_by_name("test_histogram");
    assert(all_metrics.size() == 3);

    std::cout << "PASSED\n";
}

void test_timer() {
    std::cout << "Testing timer metrics... ";

    auto& metrics = obs::MetricsCollector::instance();
    metrics.reset();

    metrics.start_timer("test_timer");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    metrics.stop_timer("test_timer");

    auto all_metrics = metrics.get_metrics_by_name("test_timer");
    assert(all_metrics.size() == 1);
    assert(all_metrics[0].value >= 10.0);

    std::cout << "PASSED\n";
}

void test_export() {
    std::cout << "Testing metrics export... ";

    auto& metrics = obs::MetricsCollector::instance();
    metrics.reset();

    metrics.increment("export_test", 1.0);
    metrics.set_gauge("export_gauge", 50.0);

    auto json = metrics.export_json();
    assert(json.is_array());
    assert(json.size() == 2);

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "Running MetricsCollector Tests\n";
    std::cout << "==============================\n";

    test_counter();
    test_gauge();
    test_histogram();
    test_timer();
    test_export();

    std::cout << "\nAll tests passed!\n";
    return 0;
}

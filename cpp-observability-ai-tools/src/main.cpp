#include "obs/metrics_collector.hpp"
#include "obs/health_monitor.hpp"
#include "obs/tracer.hpp"
#include "obs/logger.hpp"
#include "obs/alert_manager.hpp"
#include "obs/mcp_server.hpp"
#include "obs/llm_client.hpp"
#include "obs/ai_analyzer.hpp"
#include "obs/workflow_engine.hpp"
#include "obs/visualizer.hpp"
#include "obs/async_executor.hpp"

#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <fstream>

std::atomic<bool> running{true};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutting down gracefully...\n";
        running = false;
    }
}

void setup_health_checks() {
    auto& health = obs::HealthMonitor::instance();

    // CPU health check
    health.register_check("cpu_usage", "system", []() -> obs::HealthCheck {
        obs::HealthCheck check;
        check.name = "cpu_usage";
        check.component = "system";

        // Simulate CPU check (in production, read from /proc/stat)
        double cpu_usage = 45.5; // Placeholder

        if (cpu_usage < 70.0) {
            check.status = obs::HealthStatus::HEALTHY;
            check.message = "CPU usage is normal: " + std::to_string(cpu_usage) + "%";
        } else if (cpu_usage < 90.0) {
            check.status = obs::HealthStatus::DEGRADED;
            check.message = "CPU usage is elevated: " + std::to_string(cpu_usage) + "%";
        } else {
            check.status = obs::HealthStatus::UNHEALTHY;
            check.message = "CPU usage is critical: " + std::to_string(cpu_usage) + "%";
        }

        return check;
    }, std::chrono::seconds(30));

    // Memory health check
    health.register_check("memory_usage", "system", []() -> obs::HealthCheck {
        obs::HealthCheck check;
        check.name = "memory_usage";
        check.component = "system";

        // Simulate memory check
        double mem_usage = 62.3; // Placeholder

        if (mem_usage < 80.0) {
            check.status = obs::HealthStatus::HEALTHY;
            check.message = "Memory usage is normal: " + std::to_string(mem_usage) + "%";
        } else if (mem_usage < 95.0) {
            check.status = obs::HealthStatus::DEGRADED;
            check.message = "Memory usage is elevated: " + std::to_string(mem_usage) + "%";
        } else {
            check.status = obs::HealthStatus::UNHEALTHY;
            check.message = "Memory usage is critical: " + std::to_string(mem_usage) + "%";
        }

        return check;
    }, std::chrono::seconds(30));

    // Disk health check
    health.register_check("disk_usage", "storage", []() -> obs::HealthCheck {
        obs::HealthCheck check;
        check.name = "disk_usage";
        check.component = "storage";

        // Simulate disk check
        double disk_usage = 55.0; // Placeholder

        if (disk_usage < 85.0) {
            check.status = obs::HealthStatus::HEALTHY;
            check.message = "Disk usage is normal: " + std::to_string(disk_usage) + "%";
        } else if (disk_usage < 95.0) {
            check.status = obs::HealthStatus::DEGRADED;
            check.message = "Disk usage is elevated: " + std::to_string(disk_usage) + "%";
        } else {
            check.status = obs::HealthStatus::UNHEALTHY;
            check.message = "Disk usage is critical: " + std::to_string(disk_usage) + "%";
        }

        return check;
    }, std::chrono::seconds(60));

    health.start_monitoring();
}

void setup_alert_rules() {
    auto& alert_mgr = obs::AlertManager::instance();

    // Register alert handler
    alert_mgr.register_handler("console", [](const obs::Alert& alert) {
        LOG_WARN("ALERT: " + alert.name + " - " + alert.description);
    });

    // CPU alert rule
    alert_mgr.register_alert_rule(
        "high_cpu_usage",
        []() -> bool {
            // In production, check actual CPU metrics
            return false; // Placeholder
        },
        "CPU usage exceeded 80%",
        obs::AlertSeverity::WARNING,
        std::chrono::seconds(60)
    );

    alert_mgr.start_rule_evaluation();
}

void simulate_metrics() {
    auto& metrics = obs::MetricsCollector::instance();

    while (running) {
        // Simulate various metrics
        metrics.increment("requests_total", 1.0, {{"endpoint", "/api/data"}, {"method", "GET"}});
        metrics.set_gauge("active_connections", 42.0);
        metrics.record_histogram("request_duration_ms", 123.5);

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void generate_dashboard() {
    auto& metrics = obs::MetricsCollector::instance();
    auto& health = obs::HealthMonitor::instance();
    auto& alerts = obs::AlertManager::instance();
    auto& viz = obs::analytics::Visualizer::instance();

    std::string dashboard_html = viz.generate_html_dashboard(
        metrics.export_json(),
        health.export_json(),
        alerts.export_json()
    );

    std::ofstream dashboard_file("dashboard.html");
    dashboard_file << dashboard_html;
    dashboard_file.close();

    LOG_INFO("Dashboard generated: dashboard.html");
}

int main(int argc, char** argv) {
    // Setup signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Configure logging
    obs::Logger::instance().set_level(obs::LogLevel::INFO);
    obs::Logger::instance().enable_console(true);
    obs::Logger::instance().set_output_file("observability.log");

    LOG_INFO("Starting Observability & AI Tools Server");
    LOG_INFO("========================================");

    // Setup observability components
    setup_health_checks();
    setup_alert_rules();

    // Start MCP server
    auto& mcp = obs::mcp::MCPServer::instance();
    mcp.start(8080);

    LOG_INFO("MCP Server started on port 8080");
    LOG_INFO("Available MCP tools:");

    for (const auto& tool : mcp.list_tools()) {
        LOG_INFO("  - " + tool.name + ": " + tool.description);
    }

    // Start metrics simulation
    std::thread metrics_thread(simulate_metrics);

    // Example: Create an automation workflow
    auto& workflow_engine = obs::automation::WorkflowEngine::instance();
    std::string workflow_id = workflow_engine.create_workflow("Health Check Workflow", false);

    workflow_engine.add_task_to_workflow(workflow_id, "Check System Health",
        "Verify all health checks are passing", []() -> bool {
            auto health_checks = obs::HealthMonitor::instance().execute_all_checks();
            return obs::HealthMonitor::instance().get_overall_status() == obs::HealthStatus::HEALTHY;
        });

    workflow_engine.add_task_to_workflow(workflow_id, "Generate Report",
        "Create observability report", []() -> bool {
            LOG_INFO("Generating observability report...");
            return true;
        });

    // Main loop
    LOG_INFO("\nServer is running. Press Ctrl+C to stop.\n");

    int dashboard_counter = 0;
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Generate dashboard every 30 seconds
        if (++dashboard_counter >= 30) {
            generate_dashboard();
            dashboard_counter = 0;
        }
    }

    // Cleanup
    LOG_INFO("\nShutting down components...");

    mcp.stop();
    obs::HealthMonitor::instance().stop_monitoring();
    obs::AlertManager::instance().stop_rule_evaluation();

    if (metrics_thread.joinable()) {
        metrics_thread.join();
    }

    // Final dashboard
    generate_dashboard();

    // Print final statistics
    LOG_INFO("\nFinal Statistics:");
    auto stats = obs::MetricsCollector::instance().get_statistics();
    LOG_INFO("  Total metrics collected: " + std::to_string(stats.total_metrics));
    LOG_INFO("  Unique metric names: " + std::to_string(stats.unique_metric_names));
    LOG_INFO("  Collection rate: " + std::to_string(stats.collection_rate_per_sec) + " metrics/sec");

    LOG_INFO("\nServer stopped successfully");

    return 0;
}

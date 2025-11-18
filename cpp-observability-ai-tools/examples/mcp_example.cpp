#include "obs/mcp_server.hpp"
#include "obs/metrics_collector.hpp"
#include "obs/health_monitor.hpp"
#include "obs/logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    auto& logger = obs::Logger::instance();
    logger.set_level(obs::LogLevel::INFO);

    LOG_INFO("Model Context Protocol (MCP) Example");
    LOG_INFO("=====================================\n");

    // Setup some sample data
    auto& metrics = obs::MetricsCollector::instance();
    metrics.increment("api_requests", 100);
    metrics.set_gauge("active_users", 42);
    metrics.record_histogram("response_time_ms", 123.5);

    auto& health = obs::HealthMonitor::instance();
    health.register_check("api_service", "backend", []() -> obs::HealthCheck {
        obs::HealthCheck check;
        check.name = "api_service";
        check.component = "backend";
        check.status = obs::HealthStatus::HEALTHY;
        check.message = "API service is running normally";
        return check;
    });

    health.execute_all_checks();

    // Start MCP server
    auto& mcp = obs::mcp::MCPServer::instance();

    LOG_INFO("Starting MCP server on port 8080...");
    mcp.start(8080);

    // List available tools
    LOG_INFO("\nAvailable MCP Tools:");
    for (const auto& tool : mcp.list_tools()) {
        LOG_INFO("  - " + tool.name);
        LOG_INFO("    Description: " + tool.description);
    }

    // Test tool execution
    LOG_INFO("\nTesting MCP Tools:");

    // Test health check tool
    LOG_INFO("\n1. Testing get_health_status tool:");
    auto health_response = mcp.execute_tool("get_health_status", {});
    if (health_response.success) {
        LOG_INFO("   Result: " + health_response.result.dump(2));
    } else {
        LOG_INFO("   Error: " + health_response.error);
    }

    // Test metrics query tool
    LOG_INFO("\n2. Testing query_metrics tool:");
    auto metrics_response = mcp.execute_tool("query_metrics", {});
    if (metrics_response.success) {
        LOG_INFO("   Retrieved " + std::to_string(metrics_response.result.size()) + " metrics");
    } else {
        LOG_INFO("   Error: " + metrics_response.error);
    }

    // Register a custom MCP tool
    LOG_INFO("\n3. Registering custom MCP tool:");
    obs::mcp::MCPTool custom_tool;
    custom_tool.name = "get_system_info";
    custom_tool.description = "Get system information";
    custom_tool.input_schema = {
        {"type", "object"},
        {"properties", {}}
    };
    custom_tool.handler = [](const nlohmann::json& params) -> obs::mcp::MCPResponse {
        obs::mcp::MCPResponse response;
        response.success = true;
        response.result = {
            {"hostname", "observability-server"},
            {"version", "1.0.0"},
            {"uptime_seconds", 3600}
        };
        return response;
    };

    mcp.register_tool(custom_tool);
    LOG_INFO("   Registered: " + custom_tool.name);

    // Test custom tool
    auto custom_response = mcp.execute_tool("get_system_info", {});
    if (custom_response.success) {
        LOG_INFO("   Result: " + custom_response.result.dump(2));
    }

    // Display tools manifest
    LOG_INFO("\n4. MCP Tools Manifest:");
    auto manifest = mcp.get_tools_manifest();
    std::cout << manifest.dump(2) << std::endl;

    LOG_INFO("\nMCP server is running. Press Enter to stop...");
    std::cin.get();

    mcp.stop();
    LOG_INFO("MCP server stopped");

    return 0;
}

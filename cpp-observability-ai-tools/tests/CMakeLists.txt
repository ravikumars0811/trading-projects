# Tests CMakeLists.txt

# Simple test framework (header-only)
add_executable(test_metrics test_metrics.cpp)
target_link_libraries(test_metrics PRIVATE obs_core)

add_executable(test_health test_health.cpp)
target_link_libraries(test_health PRIVATE obs_core)

add_executable(test_mcp test_mcp.cpp)
target_link_libraries(test_mcp PRIVATE obs_mcp)

add_executable(test_automation test_automation.cpp)
target_link_libraries(test_automation PRIVATE obs_automation)

# Add tests
add_test(NAME MetricsCollectorTest COMMAND test_metrics)
add_test(NAME HealthMonitorTest COMMAND test_health)
add_test(NAME MCPServerTest COMMAND test_mcp)
add_test(NAME AutomationTest COMMAND test_automation)

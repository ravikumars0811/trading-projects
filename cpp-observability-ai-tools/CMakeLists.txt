cmake_minimum_required(VERSION 3.20)
project(ObservabilityAITools VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -O3 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# Check for optional packages
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, will use bundled version")
    include(FetchContent)
    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIR}
)

# Core observability library
add_library(obs_core STATIC
    src/core/metrics_collector.cpp
    src/core/health_monitor.cpp
    src/core/tracer.cpp
    src/core/logger.cpp
    src/core/alert_manager.cpp
)

target_link_libraries(obs_core
    PUBLIC
        Threads::Threads
        nlohmann_json::nlohmann_json
)

# MCP (Model Context Protocol) library
add_library(obs_mcp STATIC
    src/mcp/mcp_server.cpp
    src/mcp/mcp_tool.cpp
    src/mcp/health_tool.cpp
    src/mcp/metrics_tool.cpp
)

target_link_libraries(obs_mcp
    PUBLIC
        obs_core
        nlohmann_json::nlohmann_json
)

# AI integration library
add_library(obs_ai STATIC
    src/ai/llm_client.cpp
    src/ai/ai_analyzer.cpp
    src/ai/insight_generator.cpp
)

target_link_libraries(obs_ai
    PUBLIC
        obs_core
        CURL::libcurl
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
)

# Automation library
add_library(obs_automation STATIC
    src/automation/workflow_engine.cpp
    src/automation/rule_engine.cpp
    src/automation/task_scheduler.cpp
)

target_link_libraries(obs_automation
    PUBLIC
        obs_core
        obs_ai
        Threads::Threads
)

# Analytics library
add_library(obs_analytics STATIC
    src/analytics/data_aggregator.cpp
    src/analytics/visualizer.cpp
    src/analytics/time_series.cpp
)

target_link_libraries(obs_analytics
    PUBLIC
        obs_core
        nlohmann_json::nlohmann_json
)

# Utilities library
add_library(obs_utils STATIC
    src/utils/async_executor.cpp
    src/utils/thread_pool.cpp
    src/utils/config_loader.cpp
)

target_link_libraries(obs_utils
    PUBLIC
        Threads::Threads
        nlohmann_json::nlohmann_json
)

# Main executable - Observability Server
add_executable(obs_server
    src/main.cpp
)

target_link_libraries(obs_server
    PRIVATE
        obs_core
        obs_mcp
        obs_ai
        obs_automation
        obs_analytics
        obs_utils
)

# Example executables
add_executable(example_metrics examples/metrics_example.cpp)
target_link_libraries(example_metrics PRIVATE obs_core obs_utils)

add_executable(example_mcp examples/mcp_example.cpp)
target_link_libraries(example_mcp PRIVATE obs_mcp obs_utils)

add_executable(example_ai examples/ai_example.cpp)
target_link_libraries(example_ai PRIVATE obs_ai obs_utils)

add_executable(example_automation examples/automation_example.cpp)
target_link_libraries(example_automation PRIVATE obs_automation obs_utils)

# Testing
enable_testing()
add_subdirectory(tests)

# Installation
install(TARGETS obs_server DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY config/ DESTINATION etc/obs)

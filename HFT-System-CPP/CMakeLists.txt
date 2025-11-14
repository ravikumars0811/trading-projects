cmake_minimum_required(VERSION 3.15)
project(HFTSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler optimizations for low latency
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -ffast-math")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
endif()

# Additional warnings and optimization flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(CORE_SOURCES
    src/core/memory_pool.cpp
    src/core/timer.cpp
    src/core/config.cpp
    src/core/logger.cpp
)

set(MARKET_DATA_SOURCES
    src/market_data/market_data_handler.cpp
    src/market_data/order_book.cpp
)

set(OMS_SOURCES
    src/oms/order_manager.cpp
    src/oms/position_manager.cpp
)

set(STRATEGY_SOURCES
    src/strategy/strategy_base.cpp
    src/strategy/market_making_strategy.cpp
    src/strategy/stat_arb_strategy.cpp
)

set(GATEWAY_SOURCES
    src/gateway/exchange_gateway.cpp
    src/gateway/fix_protocol.cpp
)

set(RISK_SOURCES
    src/risk/risk_manager.cpp
)

set(METRICS_SOURCES
    src/metrics/performance_monitor.cpp
    src/metrics/latency_tracker.cpp
)

# Main executable
add_executable(hft_system
    src/main.cpp
    ${CORE_SOURCES}
    ${MARKET_DATA_SOURCES}
    ${OMS_SOURCES}
    ${STRATEGY_SOURCES}
    ${GATEWAY_SOURCES}
    ${RISK_SOURCES}
    ${METRICS_SOURCES}
)

target_link_libraries(hft_system
    Threads::Threads
)

# Installation
install(TARGETS hft_system DESTINATION bin)
install(DIRECTORY config/ DESTINATION etc/hft_system)

# Testing
enable_testing()
add_subdirectory(tests)

cmake_minimum_required(VERSION 3.15)
project(AlgorithmicTradingSystem VERSION 1.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for performance
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -march=native")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /O2")
endif()

# Find required packages
find_package(Threads REQUIRED)

# Optional: Find Boost for WebSocket support
find_package(Boost COMPONENTS system thread QUIET)

# Optional: Find PostgreSQL
find_package(PostgreSQL QUIET)

# Optional: Find Redis
find_package(hiredis QUIET)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/market_data/market_data_handler.cpp
    src/order_management/order_manager.cpp
    src/strategy/strategy.cpp
    src/risk/risk_manager.cpp
)

# Create library
add_library(trading_system STATIC ${SOURCES})
target_link_libraries(trading_system PRIVATE Threads::Threads)

# Optional libraries
if(Boost_FOUND)
    target_include_directories(trading_system PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(trading_system PRIVATE ${Boost_LIBRARIES})
    target_compile_definitions(trading_system PRIVATE HAS_BOOST)
endif()

if(PostgreSQL_FOUND)
    target_include_directories(trading_system PRIVATE ${PostgreSQL_INCLUDE_DIRS})
    target_link_libraries(trading_system PRIVATE ${PostgreSQL_LIBRARIES})
    target_compile_definitions(trading_system PRIVATE HAS_POSTGRESQL)
endif()

# Main executable
add_executable(trading_system_main src/main.cpp)
target_link_libraries(trading_system_main PRIVATE trading_system)

# Backtester executable
add_executable(backtester src/backtesting/backtester_main.cpp)
target_link_libraries(backtester PRIVATE trading_system)

# Tests (optional)
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install targets
install(TARGETS trading_system_main backtester
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  C++ Standard:      ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler:          ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Boost Found:       ${Boost_FOUND}")
message(STATUS "  PostgreSQL Found:  ${PostgreSQL_FOUND}")
message(STATUS "  Build Tests:       ${BUILD_TESTS}")
message(STATUS "")

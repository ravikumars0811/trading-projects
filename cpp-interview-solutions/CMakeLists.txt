cmake_minimum_required(VERSION 3.15)
project(CppInterviewSolutions CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find threads package (required for multithreading examples)
find_package(Threads REQUIRED)

# ============================================================================
# SECTION 1: Advanced C++ Concepts
# ============================================================================

add_executable(move_semantics 01-advanced-cpp/move_semantics.cpp)
add_executable(smart_pointers 01-advanced-cpp/smart_pointers.cpp)
add_executable(templates_metaprogramming 01-advanced-cpp/templates_metaprogramming.cpp)

# ============================================================================
# SECTION 2: HFT Data Structures
# ============================================================================

add_executable(lock_free_queue 02-hft-datastructures/lock_free_queue.cpp)
target_link_libraries(lock_free_queue Threads::Threads)

add_executable(order_book 02-hft-datastructures/order_book.cpp)

# ============================================================================
# SECTION 3: Algorithms
# ============================================================================

add_executable(trading_algorithms 03-algorithms/trading_algorithms.cpp)

# ============================================================================
# SECTION 4: Design Patterns
# ============================================================================

add_executable(trading_patterns 04-design-patterns/trading_patterns.cpp)

# ============================================================================
# SECTION 5: Concurrency
# ============================================================================

add_executable(thread_pool 05-concurrency/thread_pool.cpp)
target_link_libraries(thread_pool Threads::Threads)

# ============================================================================
# SECTION 7: Low-Latency Optimizations
# ============================================================================

add_executable(optimization_techniques 07-low-latency/optimization_techniques.cpp)
target_link_libraries(optimization_techniques Threads::Threads)
# Enable AVX for SIMD examples (if supported)
target_compile_options(optimization_techniques PRIVATE -mavx -mfma)

# ============================================================================
# SECTION 8: Trading Problems
# ============================================================================

add_executable(backtesting_engine 08-trading-problems/backtesting_engine.cpp)

# ============================================================================
# Installation
# ============================================================================

install(TARGETS
    move_semantics
    smart_pointers
    templates_metaprogramming
    lock_free_queue
    order_book
    trading_algorithms
    trading_patterns
    thread_pool
    optimization_techniques
    backtesting_engine
    RUNTIME DESTINATION bin
)

# ============================================================================
# Custom targets
# ============================================================================

# Run all examples
add_custom_target(run_all
    COMMAND echo "Running all examples..."
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/move_semantics
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/smart_pointers
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/templates_metaprogramming
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lock_free_queue
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/order_book
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trading_algorithms
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trading_patterns
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thread_pool
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/optimization_techniques
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/backtesting_engine
    DEPENDS
        move_semantics
        smart_pointers
        templates_metaprogramming
        lock_free_queue
        order_book
        trading_algorithms
        trading_patterns
        thread_pool
        optimization_techniques
        backtesting_engine
)

# Print build information
message(STATUS "")
message(STATUS "========================================")
message(STATUS "C++ Interview Solutions Build Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
endif()
message(STATUS "========================================")
message(STATUS "")

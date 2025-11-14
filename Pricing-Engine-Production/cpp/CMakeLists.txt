cmake_minimum_required(VERSION 3.15)
project(PricingEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for optimization and warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/option_pricer.cpp
    src/fixed_income_pricer.cpp
)

# Create static library
add_library(pricing_engine_static STATIC ${SOURCES})
target_include_directories(pricing_engine_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Create shared library
add_library(pricing_engine SHARED ${SOURCES})
target_include_directories(pricing_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link math library
target_link_libraries(pricing_engine_static m)
target_link_libraries(pricing_engine m)

# Python bindings using pybind11
find_package(pybind11 CONFIG)
if(pybind11_FOUND)
    pybind11_add_module(pricing_engine_py src/python_bindings.cpp ${SOURCES})
    target_include_directories(pricing_engine_py PRIVATE include)
    target_link_libraries(pricing_engine_py PRIVATE m)

    # Set output directory for Python module
    set_target_properties(pricing_engine_py PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../python/src
    )
endif()

# Testing
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest)
    if(GTest_FOUND)
        add_subdirectory(tests)
    endif()
endif()

# Installation
install(TARGETS pricing_engine pricing_engine_static
    EXPORT PricingEngineTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Export configuration
install(EXPORT PricingEngineTargets
    FILE PricingEngineConfig.cmake
    NAMESPACE PricingEngine::
    DESTINATION lib/cmake/PricingEngine
)

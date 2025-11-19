cmake_minimum_required(VERSION 3.10)
project(DPDK_Performance_Learning C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")

# Find required packages
find_package(Threads REQUIRED)

# Check for NUMA library
find_library(NUMA_LIBRARY numa)
if(NUMA_LIBRARY)
    message(STATUS "Found NUMA library: ${NUMA_LIBRARY}")
    set(HAVE_NUMA TRUE)
else()
    message(WARNING "NUMA library not found - some examples may not build")
    set(HAVE_NUMA FALSE)
endif()

# Check for DPDK
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(DPDK libdpdk)
    if(DPDK_FOUND)
        message(STATUS "Found DPDK: ${DPDK_VERSION}")
        set(HAVE_DPDK TRUE)
    else()
        message(WARNING "DPDK not found - DPDK examples will not be built")
        message(WARNING "See 01-dpdk/docs/setup.md for installation instructions")
        set(HAVE_DPDK FALSE)
    endif()
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/common/include)

#
# Performance Optimization Examples
#
add_executable(cache_friendly
    02-performance-optimization/cache/cache_friendly.c
)
target_link_libraries(cache_friendly m)

#
# System Programming Examples
#
if(HAVE_NUMA)
    add_executable(thread_affinity
        03-system-programming/threads/thread_affinity.c
    )
    target_link_libraries(thread_affinity Threads::Threads ${NUMA_LIBRARY})
endif()

#
# Memory Management Examples
#
add_executable(arena_allocator
    04-memory-management/allocators/arena_allocator.c
)
target_link_libraries(arena_allocator m)

#
# CPU Optimization Examples
#
add_executable(simd_basics
    05-cpu-optimization/simd/simd_basics.c
)
# Enable SIMD optimizations
target_compile_options(simd_basics PRIVATE -march=native -mavx2)
target_link_libraries(simd_basics m)

#
# Distributed Systems Examples
#
add_executable(tcp_server_epoll
    06-distributed-systems/networking/tcp_server_epoll.c
)

add_executable(consistent_hash
    06-distributed-systems/load-balancing/consistent_hash.c
)
target_link_libraries(consistent_hash m)

#
# DPDK Examples (only if DPDK is available)
#
if(HAVE_DPDK)
    # Basic EAL Init
    add_executable(basic_eal_init
        01-dpdk/examples/basic_eal_init.c
    )
    target_include_directories(basic_eal_init PRIVATE ${DPDK_INCLUDE_DIRS})
    target_link_libraries(basic_eal_init ${DPDK_LIBRARIES})

    # Mempool Example
    add_executable(mempool_example
        01-dpdk/examples/mempool_example.c
    )
    target_include_directories(mempool_example PRIVATE ${DPDK_INCLUDE_DIRS})
    target_link_libraries(mempool_example ${DPDK_LIBRARIES})

    # Ring Example
    add_executable(ring_example
        01-dpdk/examples/ring_example.c
    )
    target_include_directories(ring_example PRIVATE ${DPDK_INCLUDE_DIRS})
    target_link_libraries(ring_example ${DPDK_LIBRARIES})
endif()

#
# Install targets
#
install(TARGETS cache_friendly arena_allocator simd_basics
                tcp_server_epoll consistent_hash
        RUNTIME DESTINATION bin)

if(HAVE_NUMA)
    install(TARGETS thread_affinity RUNTIME DESTINATION bin)
endif()

if(HAVE_DPDK)
    install(TARGETS basic_eal_init mempool_example ring_example
            RUNTIME DESTINATION bin)
endif()

#
# Print build summary
#
message(STATUS "")
message(STATUS "=================================")
message(STATUS "Build Configuration Summary")
message(STATUS "=================================")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "NUMA support: ${HAVE_NUMA}")
message(STATUS "DPDK support: ${HAVE_DPDK}")
message(STATUS "=================================")
message(STATUS "")
message(STATUS "Build commands:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make")
message(STATUS "")
message(STATUS "Run examples:")
message(STATUS "  ./cache_friendly")
message(STATUS "  ./simd_basics")
message(STATUS "  ./tcp_server_epoll 8080")
if(HAVE_DPDK)
    message(STATUS "  sudo ./basic_eal_init -l 0-3 -n 4")
endif()
message(STATUS "=================================")

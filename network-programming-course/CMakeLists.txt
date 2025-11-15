cmake_minimum_required(VERSION 3.10)
project(NetworkProgrammingCourse CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -march=native")

# Module 2: Socket Programming Examples
add_executable(tcp_server module-02-socket-programming/examples/tcp_server.cpp)
add_executable(tcp_client module-02-socket-programming/examples/tcp_client.cpp)
add_executable(udp_server module-02-socket-programming/examples/udp_server.cpp)
add_executable(udp_client module-02-socket-programming/examples/udp_client.cpp)
add_executable(optimized_tcp_server module-02-socket-programming/examples/optimized_tcp_server.cpp)

# Module 3: Advanced I/O Examples
add_executable(epoll_server module-03-advanced-io/examples/epoll_server.cpp)

# Find liburing for io_uring examples (optional)
find_library(URING_LIB uring)
if(URING_LIB)
    message(STATUS "Found liburing: ${URING_LIB}")
    # Uncomment when io_uring example is added
    # add_executable(io_uring_server module-03-advanced-io/examples/io_uring_server.cpp)
    # target_link_libraries(io_uring_server ${URING_LIB})
else()
    message(STATUS "liburing not found - io_uring examples will be skipped")
endif()

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")

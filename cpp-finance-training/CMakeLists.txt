cmake_minimum_required(VERSION 3.20)
project(CPP_Finance_Training VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
endif()

# Enable threading
find_package(Threads REQUIRED)

# Function to add example executable
function(add_example module_name file_name)
    get_filename_component(target_name ${file_name} NAME_WE)
    set(target_name "${module_name}_${target_name}")
    add_executable(${target_name} ${module_name}/${file_name})
    target_link_libraries(${target_name} PRIVATE Threads::Threads)
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${module_name}"
    )
endfunction()

# Module 1: Basics
add_example(01-basics 01_variables_datatypes.cpp)
add_example(01-basics 02_control_flow.cpp)
add_example(01-basics 03_functions.cpp)
add_example(01-basics 04_arrays_strings.cpp)
add_example(01-basics 05_pointers_references.cpp)
add_example(01-basics 06_dynamic_memory.cpp)
add_example(01-basics 07_io_operations.cpp)

# Module 2: Intermediate
add_example(02-intermediate 01_classes_objects.cpp)
add_example(02-intermediate 02_inheritance_polymorphism.cpp)
add_example(02-intermediate 04_stl_containers.cpp)

# Module 3: Advanced
add_example(03-advanced 01_move_semantics.cpp)

# Module 4: Modern C++
add_example(04-modern-cpp 01_cpp11_features.cpp)

# Module 5: System Programming
add_example(05-system-programming 01_high_resolution_timing.cpp)

# Module 7: HFT Specific
add_example(07-hft-specific 01_order_book.cpp)
add_example(07-hft-specific 02_lockfree_queue.cpp)

# Print build information
message(STATUS "")
message(STATUS "=== C++ Finance Training Build Configuration ===")
message(STATUS "Build type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard:     C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler flags:   ${CMAKE_CXX_FLAGS}")
if(CMAKE_BUILD_TYPE MATCHES "Release")
    message(STATUS "Release flags:    ${CMAKE_CXX_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE MATCHES "Debug")
    message(STATUS "Debug flags:      ${CMAKE_CXX_FLAGS_DEBUG}")
endif()
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/bin")
message(STATUS "================================================")
message(STATUS "")

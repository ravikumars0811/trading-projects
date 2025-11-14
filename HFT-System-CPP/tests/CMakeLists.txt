cmake_minimum_required(VERSION 3.15)

# Test executables
add_executable(test_order_book test_order_book.cpp
    ../src/market_data/order_book.cpp
    ../src/core/timer.cpp)
target_link_libraries(test_order_book ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_lock_free_queue test_lock_free_queue.cpp
    ../src/core/timer.cpp)
target_link_libraries(test_lock_free_queue ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_memory_pool test_memory_pool.cpp
    ../src/core/timer.cpp)
target_link_libraries(test_memory_pool ${CMAKE_THREAD_LIBS_INIT})

# Add tests
add_test(NAME OrderBookTest COMMAND test_order_book)
add_test(NAME LockFreeQueueTest COMMAND test_lock_free_queue)
add_test(NAME MemoryPoolTest COMMAND test_memory_pool)

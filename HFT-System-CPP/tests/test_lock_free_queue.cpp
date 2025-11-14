#include "../include/core/lock_free_queue.hpp"
#include "../include/core/timer.hpp"
#include <iostream>
#include <thread>
#include <cassert>

using namespace hft;

void testBasicOperations() {
    std::cout << "Testing basic queue operations...\n";

    core::LockFreeQueue<int, 1024> queue;

    assert(queue.empty());
    std::cout << "✓ Queue starts empty\n";

    // Push items
    for (int i = 0; i < 10; ++i) {
        assert(queue.push(i));
    }

    assert(!queue.empty());
    assert(queue.size() == 10);
    std::cout << "✓ Push operations work correctly\n";

    // Pop items
    for (int i = 0; i < 10; ++i) {
        auto item = queue.pop();
        assert(item.has_value());
        assert(item.value() == i);
    }

    assert(queue.empty());
    std::cout << "✓ Pop operations work correctly\n";
}

void testThreadSafety() {
    std::cout << "\nTesting thread safety...\n";

    core::LockFreeQueue<int, 65536> queue;
    const int num_items = 10000;

    std::atomic<int> items_received{0};

    // Producer thread
    std::thread producer([&queue, num_items]() {
        for (int i = 0; i < num_items; ++i) {
            while (!queue.push(i)) {
                std::this_thread::yield();
            }
        }
    });

    // Consumer thread
    std::thread consumer([&queue, &items_received, num_items]() {
        int expected = 0;
        while (expected < num_items) {
            auto item = queue.pop();
            if (item.has_value()) {
                assert(item.value() == expected);
                expected++;
                items_received++;
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    assert(items_received == num_items);
    std::cout << "✓ Thread safety verified\n";
}

void testPerformance() {
    std::cout << "\nTesting queue performance...\n";

    core::LockFreeQueue<int, 65536> queue;
    const int num_items = 100000;

    core::Timer timer;

    // Push
    for (int i = 0; i < num_items; ++i) {
        queue.push(i);
    }

    int64_t push_time = timer.elapsed_us();

    timer.reset();

    // Pop
    for (int i = 0; i < num_items; ++i) {
        queue.pop();
    }

    int64_t pop_time = timer.elapsed_us();

    std::cout << "  Push " << num_items << " items: " << push_time << " us\n";
    std::cout << "  Pop " << num_items << " items: " << pop_time << " us\n";
    std::cout << "  Avg push latency: " << (double)push_time / num_items << " us\n";
    std::cout << "  Avg pop latency: " << (double)pop_time / num_items << " us\n";
    std::cout << "✓ Performance test completed\n";
}

int main() {
    std::cout << "=== Lock-Free Queue Tests ===\n\n";

    try {
        testBasicOperations();
        testThreadSafety();
        testPerformance();

        std::cout << "\n✅ All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << "\n";
        return 1;
    }
}

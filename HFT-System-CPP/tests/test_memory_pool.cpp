#include "../include/core/memory_pool.hpp"
#include "../include/core/timer.hpp"
#include <iostream>
#include <cassert>
#include <vector>

using namespace hft;

struct TestObject {
    int value;
    double data;
    char buffer[64];

    TestObject(int v = 0) : value(v), data(v * 1.5) {}
};

void testBasicAllocation() {
    std::cout << "Testing basic allocation...\n";

    core::MemoryPool<TestObject> pool;

    // Allocate objects
    auto* obj1 = pool.allocate(42);
    assert(obj1 != nullptr);
    assert(obj1->value == 42);
    std::cout << "✓ Single allocation works\n";

    auto* obj2 = pool.allocate(100);
    assert(obj2 != nullptr);
    assert(obj2->value == 100);
    assert(obj1 != obj2);
    std::cout << "✓ Multiple allocations work\n";

    // Deallocate
    pool.deallocate(obj1);
    pool.deallocate(obj2);
    std::cout << "✓ Deallocation works\n";
}

void testReuse() {
    std::cout << "\nTesting memory reuse...\n";

    core::MemoryPool<TestObject> pool;

    auto* obj1 = pool.allocate(1);
    void* addr1 = obj1;
    pool.deallocate(obj1);

    auto* obj2 = pool.allocate(2);
    void* addr2 = obj2;

    // Should reuse the same memory
    assert(addr1 == addr2);
    std::cout << "✓ Memory reuse works correctly\n";

    pool.deallocate(obj2);
}

void testPerformance() {
    std::cout << "\nTesting allocation performance...\n";

    core::MemoryPool<TestObject> pool;
    const int num_allocs = 10000;

    core::Timer timer;

    std::vector<TestObject*> objects;
    objects.reserve(num_allocs);

    // Allocate
    for (int i = 0; i < num_allocs; ++i) {
        objects.push_back(pool.allocate(i));
    }

    int64_t alloc_time = timer.elapsed_us();

    timer.reset();

    // Deallocate
    for (auto* obj : objects) {
        pool.deallocate(obj);
    }

    int64_t dealloc_time = timer.elapsed_us();

    std::cout << "  Allocate " << num_allocs << " objects: " << alloc_time << " us\n";
    std::cout << "  Deallocate " << num_allocs << " objects: " << dealloc_time << " us\n";
    std::cout << "  Avg allocation latency: " << (double)alloc_time / num_allocs << " us\n";
    std::cout << "✓ Performance test completed\n";

    // Compare with standard new/delete
    timer.reset();
    std::vector<TestObject*> std_objects;
    for (int i = 0; i < num_allocs; ++i) {
        std_objects.push_back(new TestObject(i));
    }
    int64_t std_alloc_time = timer.elapsed_us();

    timer.reset();
    for (auto* obj : std_objects) {
        delete obj;
    }
    int64_t std_dealloc_time = timer.elapsed_us();

    std::cout << "\n  Standard new/delete comparison:\n";
    std::cout << "  Allocate: " << std_alloc_time << " us\n";
    std::cout << "  Deallocate: " << std_dealloc_time << " us\n";
    std::cout << "  Speedup: " << (double)std_alloc_time / alloc_time << "x\n";
}

int main() {
    std::cout << "=== Memory Pool Tests ===\n\n";

    try {
        testBasicAllocation();
        testReuse();
        testPerformance();

        std::cout << "\n✅ All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << "\n";
        return 1;
    }
}

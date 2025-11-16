# HFT Server & Pricing Engine Performance Optimization Guide

## Table of Contents
1. [System-Level Linux Optimizations](#1-system-level-linux-optimizations)
2. [CPU & Cache Optimization](#2-cpu--cache-optimization)
3. [Memory Optimization](#3-memory-optimization)
4. [Network Optimization](#4-network-optimization)
5. [Lock-Free Programming Patterns](#5-lock-free-programming-patterns)
6. [SIMD Vectorization](#6-simd-vectorization)
7. [I/O Optimization](#7-io-optimization)
8. [Python Performance Optimization](#8-python-performance-optimization)
9. [Profiling & Debugging Tools](#9-profiling--debugging-tools)
10. [Real-World Optimization Examples](#10-real-world-optimization-examples)

---

## 1. System-Level Linux Optimizations

### 1.1 CPU Isolation & IRQ Affinity

**Problem**: OS interrupts and context switches cause latency spikes.

**Solution**: Isolate CPUs for HFT workloads and bind interrupts to specific cores.

```bash
# /etc/default/grub - Isolate CPUs 2-7 for HFT
GRUB_CMDLINE_LINUX="isolcpus=2-7 nohz_full=2-7 rcu_nocbs=2-7"
sudo update-grub
sudo reboot

# Move all IRQs to CPU 0-1 (housekeeping cores)
#!/bin/bash
for irq in /proc/irq/*/smp_affinity; do
    echo "03" > $irq  # Binary: 0000 0011 = CPUs 0,1
done

# Bind HFT process to isolated cores
taskset -c 2-7 ./hft_server

# Verify CPU affinity
ps -eo pid,psr,comm | grep hft_server
```

**C++ Thread Pinning**:
```cpp
#include <pthread.h>
#include <sched.h>

class CPUAffinity {
public:
    static bool pinThread(pthread_t thread, int cpu_id) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id, &cpuset);

        int result = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
        if (result != 0) {
            std::cerr << "Failed to pin thread to CPU " << cpu_id << std::endl;
            return false;
        }

        // Set real-time priority
        struct sched_param param;
        param.sched_priority = 99; // Max priority
        result = pthread_setschedparam(thread, SCHED_FIFO, &param);

        return result == 0;
    }

    static int getCurrentCPU() {
        return sched_getcpu();
    }
};

// Usage in trading thread
void* tradingThreadFunc(void* arg) {
    pthread_t current = pthread_self();
    CPUAffinity::pinThread(current, 2); // Pin to CPU 2

    std::cout << "Trading thread on CPU: " << CPUAffinity::getCurrentCPU() << std::endl;

    while (running) {
        // Critical trading logic
    }
    return nullptr;
}
```

### 1.2 Disable CPU Frequency Scaling

**Problem**: CPU frequency changes introduce latency jitter.

```bash
# Set CPU governor to performance mode
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    echo "performance" > $cpu
done

# Disable Turbo Boost (reduces jitter)
echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo

# Verify
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

### 1.3 Huge Pages Configuration

**Problem**: TLB (Translation Lookaside Buffer) misses slow memory access.

**Solution**: Use huge pages (2MB/1GB) for large memory regions.

```bash
# Configure transparent huge pages
echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo always > /sys/kernel/mm/transparent_hugepage/defrag

# For explicit huge pages (2MB)
echo 1024 > /proc/sys/vm/nr_hugepages  # 2GB worth

# For 1GB huge pages (better for very large datasets)
echo 4 > /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages

# Verify
cat /proc/meminfo | grep Huge
```

**C++ Huge Pages Allocation**:
```cpp
#include <sys/mman.h>
#include <cstddef>

template<typename T>
class HugePageAllocator {
private:
    static constexpr size_t HUGE_PAGE_SIZE = 2 * 1024 * 1024; // 2MB

public:
    static T* allocate(size_t n) {
        size_t bytes = n * sizeof(T);

        // Align to huge page boundary
        size_t aligned_bytes = ((bytes + HUGE_PAGE_SIZE - 1) / HUGE_PAGE_SIZE) * HUGE_PAGE_SIZE;

        void* ptr = mmap(nullptr, aligned_bytes,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                        -1, 0);

        if (ptr == MAP_FAILED) {
            // Fallback to regular allocation
            ptr = aligned_alloc(64, bytes);
        }

        return static_cast<T*>(ptr);
    }

    static void deallocate(T* ptr, size_t n) {
        size_t bytes = n * sizeof(T);
        size_t aligned_bytes = ((bytes + HUGE_PAGE_SIZE - 1) / HUGE_PAGE_SIZE) * HUGE_PAGE_SIZE;
        munmap(ptr, aligned_bytes);
    }
};

// Usage for order book
struct OrderBookData {
    std::array<PriceLevel, 10000> bids;
    std::array<PriceLevel, 10000> asks;
};

// Allocate on huge pages
OrderBookData* orderBook = HugePageAllocator<OrderBookData>::allocate(1);
```

### 1.4 NUMA Optimization

**Problem**: Non-uniform memory access causes latency variance.

```bash
# Check NUMA topology
numactl --hardware

# Run HFT server on specific NUMA node
numactl --cpunodebind=0 --membind=0 ./hft_server

# Interleave memory across nodes (for large datasets)
numactl --interleave=all ./pricing_engine
```

**C++ NUMA-Aware Allocation**:
```cpp
#include <numa.h>
#include <numaif.h>

class NUMAAllocator {
public:
    static void* allocateOnNode(size_t size, int node) {
        if (numa_available() == -1) {
            return malloc(size);
        }

        void* ptr = numa_alloc_onnode(size, node);
        if (!ptr) {
            throw std::bad_alloc();
        }
        return ptr;
    }

    static void bindToNode(int node) {
        struct bitmask* mask = numa_allocate_nodemask();
        numa_bitmask_setbit(mask, node);
        numa_bind(mask);
        numa_free_nodemask(mask);
    }

    static int getCurrentNode() {
        return numa_node_of_cpu(sched_getcpu());
    }
};

// Usage
void initTradingEngine() {
    // Bind memory allocations to NUMA node 0
    NUMAAllocator::bindToNode(0);

    // Allocate order book on same node
    void* orderBookMem = NUMAAllocator::allocateOnNode(sizeof(OrderBook), 0);
    OrderBook* book = new (orderBookMem) OrderBook();
}
```

---

## 2. CPU & Cache Optimization

### 2.1 Cache Line Alignment & False Sharing Prevention

**Problem**: False sharing causes cache coherency traffic between cores.

**Solution**: Align data structures to cache line boundaries (64 bytes on x86-64).

```cpp
// Bad: False sharing between threads
struct BadCounters {
    std::atomic<uint64_t> counter1;  // Same cache line
    std::atomic<uint64_t> counter2;  // Same cache line
};

// Good: Cache-line aligned
struct alignas(64) GoodCounters {
    std::atomic<uint64_t> counter1;
    char padding1[56];  // 64 - 8 = 56 bytes padding

    alignas(64) std::atomic<uint64_t> counter2;
    char padding2[56];
};

// Better: Use C++17 hardware_destructive_interference_size
#include <new>

struct OptimalCounters {
    alignas(std::hardware_destructive_interference_size)
    std::atomic<uint64_t> counter1;

    alignas(std::hardware_destructive_interference_size)
    std::atomic<uint64_t> counter2;
};
```

**Real-world Example: Lock-Free Queue**:
```cpp
template<typename T, size_t Size>
class CacheAlignedQueue {
private:
    static constexpr size_t CACHE_LINE = 64;

    // Producer writes to head, consumer reads tail
    // Separate cache lines prevent false sharing
    alignas(CACHE_LINE) std::atomic<size_t> head_{0};
    char pad1_[CACHE_LINE - sizeof(std::atomic<size_t>)];

    alignas(CACHE_LINE) std::atomic<size_t> tail_{0};
    char pad2_[CACHE_LINE - sizeof(std::atomic<size_t>)];

    alignas(CACHE_LINE) std::array<T, Size> buffer_;

public:
    bool push(const T& item) {
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t next_head = (current_head + 1) % Size;

        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false; // Queue full
        }

        buffer_[current_head] = item;
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);

        if (current_tail == head_.load(std::memory_order_acquire)) {
            return false; // Queue empty
        }

        item = buffer_[current_tail];
        tail_.store((current_tail + 1) % Size, std::memory_order_release);
        return true;
    }
};
```

### 2.2 Data Structure Layout Optimization

**Problem**: Poor data locality causes cache misses.

**Solution**: Structure of Arrays (SoA) instead of Array of Structures (AoS).

```cpp
// Bad: Array of Structures (AoS) - poor cache utilization
struct Order {
    uint64_t order_id;
    uint32_t price;
    uint32_t quantity;
    uint64_t timestamp;
    char symbol[8];
};

std::vector<Order> orders;

// Iterating through prices causes loading unnecessary data
for (const auto& order : orders) {
    process(order.price);  // Loads entire struct, wastes cache
}

// Good: Structure of Arrays (SoA) - excellent cache utilization
struct OrderBook_SoA {
    std::vector<uint64_t> order_ids;
    std::vector<uint32_t> prices;      // Contiguous in memory
    std::vector<uint32_t> quantities;  // Contiguous in memory
    std::vector<uint64_t> timestamps;
    std::vector<std::array<char, 8>> symbols;

    void addOrder(uint64_t id, uint32_t price, uint32_t qty,
                  uint64_t ts, const char* sym) {
        order_ids.push_back(id);
        prices.push_back(price);
        quantities.push_back(qty);
        timestamps.push_back(ts);
        symbols.push_back(*reinterpret_cast<const std::array<char, 8>*>(sym));
    }
};

// Now iterating is cache-friendly
OrderBook_SoA book;
for (uint32_t price : book.prices) {
    process(price);  // Sequential memory access, great cache utilization
}
```

### 2.3 Prefetching

**Problem**: Memory latency stalls the pipeline.

**Solution**: Manual prefetching to hide latency.

```cpp
#include <xmmintrin.h>  // For _mm_prefetch

class OrderBookWithPrefetch {
private:
    static constexpr int PREFETCH_DISTANCE = 8;
    std::array<PriceLevel*, 10000> levels;

public:
    void processOrders(const std::vector<int>& indices) {
        for (size_t i = 0; i < indices.size(); ++i) {
            // Prefetch future data while processing current
            if (i + PREFETCH_DISTANCE < indices.size()) {
                _mm_prefetch(reinterpret_cast<const char*>(
                    levels[indices[i + PREFETCH_DISTANCE]]), _MM_HINT_T0);
            }

            // Process current level
            PriceLevel* level = levels[indices[i]];
            processLevel(level);
        }
    }

    void processLevel(PriceLevel* level) {
        // Business logic
    }
};

// Software prefetching for linked list traversal
void traverseOrderList(Order* head) {
    Order* current = head;
    Order* next = current ? current->next : nullptr;

    while (current) {
        if (next) {
            // Prefetch next node while processing current
            __builtin_prefetch(next, 0, 3);  // Read, high temporal locality
            if (next->next) {
                __builtin_prefetch(next->next, 0, 2);  // Medium locality
            }
        }

        processOrder(current);
        current = next;
        next = current ? current->next : nullptr;
    }
}
```

---

## 3. Memory Optimization

### 3.1 Custom Allocators with Memory Pools

**Problem**: `malloc/free` and `new/delete` are too slow for HFT.

**Solution**: Pre-allocated memory pools with O(1) allocation.

```cpp
template<typename T, size_t BlockSize = 4096>
class FastMemoryPool {
private:
    union Node {
        T data;
        Node* next;
    };

    struct alignas(64) Block {
        std::array<Node, BlockSize> nodes;
        Block* next_block;
    };

    Node* free_list_ = nullptr;
    Block* block_head_ = nullptr;
    std::atomic<size_t> allocation_count_{0};
    std::atomic<size_t> deallocation_count_{0};

    void allocateBlock() {
        Block* new_block = static_cast<Block*>(
            std::aligned_alloc(64, sizeof(Block)));

        // Link all nodes in free list
        for (size_t i = 0; i < BlockSize - 1; ++i) {
            new_block->nodes[i].next = &new_block->nodes[i + 1];
        }
        new_block->nodes[BlockSize - 1].next = free_list_;

        free_list_ = &new_block->nodes[0];
        new_block->next_block = block_head_;
        block_head_ = new_block;
    }

public:
    FastMemoryPool() {
        allocateBlock();
    }

    ~FastMemoryPool() {
        Block* current = block_head_;
        while (current) {
            Block* next = current->next_block;
            std::free(current);
            current = next;
        }
    }

    T* allocate() {
        if (!free_list_) {
            allocateBlock();
        }

        Node* node = free_list_;
        free_list_ = node->next;
        allocation_count_.fetch_add(1, std::memory_order_relaxed);

        return &node->data;
    }

    void deallocate(T* ptr) {
        Node* node = reinterpret_cast<Node*>(ptr);
        node->next = free_list_;
        free_list_ = node;
        deallocation_count_.fetch_add(1, std::memory_order_relaxed);
    }

    // Statistics
    size_t getAllocationCount() const {
        return allocation_count_.load(std::memory_order_relaxed);
    }

    size_t getDeallocationCount() const {
        return deallocation_count_.load(std::memory_order_relaxed);
    }
};

// Usage
struct Order {
    uint64_t id;
    uint32_t price;
    uint32_t quantity;
};

FastMemoryPool<Order> order_pool;

// Allocation is O(1)
Order* order = order_pool.allocate();
new (order) Order{123, 10050, 100};  // Placement new

// Deallocation is O(1)
order->~Order();
order_pool.deallocate(order);
```

### 3.2 Stack Allocation for Hot Path

**Problem**: Heap allocation in critical path.

**Solution**: Use stack-allocated arrays or `alloca` for small, temporary data.

```cpp
// Bad: Heap allocation in hot path
std::vector<double> calculateVWAP(const std::vector<Trade>& trades) {
    std::vector<double> vwaps;  // Heap allocation
    vwaps.reserve(trades.size());
    // ...
    return vwaps;
}

// Good: Stack allocation for small arrays
std::array<double, 100> calculateVWAP_Fast(const Trade* trades, size_t count) {
    std::array<double, 100> vwaps;  // Stack allocation
    // ...
    return vwaps;
}

// Better: Use VLAs (Variable Length Arrays) via alloca (GCC/Clang)
void calculateVWAP_VLA(const Trade* trades, size_t count, double* out) {
    // Allocate on stack (fast, but risky for large sizes)
    double* temp = static_cast<double*>(alloca(count * sizeof(double)));

    // Calculation
    for (size_t i = 0; i < count; ++i) {
        temp[i] = trades[i].price * trades[i].volume;
    }

    // Copy result
    std::memcpy(out, temp, count * sizeof(double));
}

// Best: Small buffer optimization (SBO)
template<typename T, size_t N>
class SmallVector {
private:
    size_t size_ = 0;
    size_t capacity_ = N;
    T* data_;
    alignas(T) char buffer_[N * sizeof(T)];

public:
    SmallVector() : data_(reinterpret_cast<T*>(buffer_)) {}

    ~SmallVector() {
        if (data_ != reinterpret_cast<T*>(buffer_)) {
            delete[] data_;
        }
    }

    void push_back(const T& value) {
        if (size_ == capacity_) {
            reserve(capacity_ * 2);
        }
        data_[size_++] = value;
    }

    void reserve(size_t new_cap) {
        if (new_cap <= capacity_) return;

        T* new_data = new T[new_cap];
        std::memcpy(new_data, data_, size_ * sizeof(T));

        if (data_ != reinterpret_cast<T*>(buffer_)) {
            delete[] data_;
        }

        data_ = new_data;
        capacity_ = new_cap;
    }

    T& operator[](size_t i) { return data_[i]; }
    size_t size() const { return size_; }
};

// Up to 100 elements: no heap allocation
SmallVector<double, 100> vwaps;
```

### 3.3 Memory-Mapped Files for Large Datasets

**Problem**: Loading large market data files is slow.

**Solution**: Use `mmap` for zero-copy file access.

```cpp
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

class MemoryMappedFile {
private:
    void* data_ = nullptr;
    size_t size_ = 0;
    int fd_ = -1;

public:
    MemoryMappedFile(const char* filename) {
        fd_ = open(filename, O_RDONLY);
        if (fd_ == -1) {
            throw std::runtime_error("Cannot open file");
        }

        struct stat sb;
        if (fstat(fd_, &sb) == -1) {
            close(fd_);
            throw std::runtime_error("Cannot stat file");
        }

        size_ = sb.st_size;

        data_ = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0);
        if (data_ == MAP_FAILED) {
            close(fd_);
            throw std::runtime_error("Cannot mmap file");
        }

        // Advise kernel about access pattern
        madvise(data_, size_, MADV_SEQUENTIAL | MADV_WILLNEED);
    }

    ~MemoryMappedFile() {
        if (data_ != nullptr && data_ != MAP_FAILED) {
            munmap(data_, size_);
        }
        if (fd_ != -1) {
            close(fd_);
        }
    }

    const void* data() const { return data_; }
    size_t size() const { return size_; }

    // Parse tick data efficiently
    template<typename TickHandler>
    void parseTicks(TickHandler handler) {
        const char* ptr = static_cast<const char*>(data_);
        const char* end = ptr + size_;

        while (ptr < end) {
            // Parse tick (assuming fixed-size binary format)
            const Tick* tick = reinterpret_cast<const Tick*>(ptr);
            handler(*tick);
            ptr += sizeof(Tick);
        }
    }
};

// Usage
struct Tick {
    uint64_t timestamp;
    uint32_t price;
    uint32_t volume;
};

MemoryMappedFile file("market_data.bin");
file.parseTicks([](const Tick& tick) {
    processMarketData(tick);
});
```

---

## 4. Network Optimization

### 4.1 Kernel Bypass with AF_XDP or DPDK

**Problem**: Kernel network stack adds latency.

**Solution**: User-space networking with kernel bypass.

**AF_XDP Example (Modern Linux Kernel)**:
```cpp
#include <linux/if_xdp.h>
#include <bpf/xsk.h>

class XDPSocket {
private:
    struct xsk_socket_info {
        struct xsk_ring_cons rx;
        struct xsk_ring_prod tx;
        struct xsk_umem_info* umem;
        struct xsk_socket* xsk;
        uint32_t outstanding_tx;
    };

    xsk_socket_info* xsk_info_ = nullptr;

public:
    void receivePackets() {
        uint32_t idx_rx = 0, idx_fq = 0;
        unsigned int rcvd = xsk_ring_cons__peek(&xsk_info_->rx,
                                                  BATCH_SIZE, &idx_rx);

        if (rcvd == 0) return;

        for (unsigned int i = 0; i < rcvd; i++) {
            const struct xdp_desc* desc =
                xsk_ring_cons__rx_desc(&xsk_info_->rx, idx_rx++);

            uint64_t addr = desc->addr;
            uint32_t len = desc->len;

            // Process packet at addr
            processMarketDataPacket(
                xsk_umem__get_data(xsk_info_->umem->buffer, addr), len);
        }

        xsk_ring_cons__release(&xsk_info_->rx, rcvd);
    }

    void processMarketDataPacket(const uint8_t* data, uint32_t len) {
        // Parse FIX message or binary protocol
        // Ultra-low latency: bypasses kernel network stack
    }
};
```

### 4.2 TCP Optimization

**Problem**: Default TCP settings are tuned for throughput, not latency.

```bash
# Disable Nagle's algorithm (TCP_NODELAY)
# Reduce buffering delay

# Kernel tuning
sysctl -w net.ipv4.tcp_low_latency=1
sysctl -w net.ipv4.tcp_timestamps=0  # Reduce overhead
sysctl -w net.ipv4.tcp_sack=0        # Disable selective ACK

# Increase buffer sizes
sysctl -w net.core.rmem_max=134217728  # 128MB
sysctl -w net.core.wmem_max=134217728
sysctl -w net.ipv4.tcp_rmem="4096 87380 134217728"
sysctl -w net.ipv4.tcp_wmem="4096 65536 134217728"

# Reduce TIME_WAIT
sysctl -w net.ipv4.tcp_fin_timeout=15
sysctl -w net.ipv4.tcp_tw_reuse=1
```

**C++ TCP Socket Optimization**:
```cpp
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

class LowLatencyTCP {
public:
    static void optimizeSocket(int sockfd) {
        // Disable Nagle's algorithm
        int flag = 1;
        setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

        // Disable delayed ACK
        setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag));

        // Set low latency mode
        int low_latency = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_PRIORITY, &low_latency, sizeof(low_latency));

        // Increase socket buffers
        int bufsize = 16 * 1024 * 1024;  // 16MB
        setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));

        // Set busy poll (poll NIC without interrupt)
        unsigned long busy_poll_us = 50;  // 50 microseconds
        setsockopt(sockfd, SOL_SOCKET, SO_BUSY_POLL, &busy_poll_us, sizeof(busy_poll_us));
    }

    static ssize_t sendWithRetry(int sockfd, const void* buf, size_t len) {
        ssize_t sent = 0;
        while (sent < static_cast<ssize_t>(len)) {
            ssize_t n = send(sockfd,
                            static_cast<const char*>(buf) + sent,
                            len - sent, MSG_NOSIGNAL);
            if (n < 0) {
                if (errno == EINTR || errno == EAGAIN) {
                    continue;
                }
                return -1;
            }
            sent += n;
        }
        return sent;
    }
};
```

### 4.3 UDP Multicast for Market Data

**Problem**: TCP overhead too high for market data feeds.

**Solution**: Use UDP multicast with sequence number checking.

```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class UDPMulticastReceiver {
private:
    int sockfd_;
    uint64_t expected_seqnum_ = 0;
    uint64_t gaps_detected_ = 0;

public:
    UDPMulticastReceiver(const char* group_ip, uint16_t port) {
        sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd_ < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        // Allow multiple sockets to bind to same port
        int reuse = 1;
        setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        // Bind to port
        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sockfd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            throw std::runtime_error("Bind failed");
        }

        // Join multicast group
        struct ip_mreq mreq = {};
        mreq.imr_multiaddr.s_addr = inet_addr(group_ip);
        mreq.imr_interface.s_addr = INADDR_ANY;

        setsockopt(sockfd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

        // Increase receive buffer
        int bufsize = 32 * 1024 * 1024;  // 32MB
        setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
    }

    struct MarketDataPacket {
        uint64_t seqnum;
        uint64_t timestamp;
        char symbol[8];
        uint32_t price;
        uint32_t volume;
    } __attribute__((packed));

    void receiveLoop() {
        MarketDataPacket packet;

        while (true) {
            ssize_t n = recv(sockfd_, &packet, sizeof(packet), 0);
            if (n != sizeof(packet)) {
                continue;
            }

            // Check for gaps (packet loss)
            if (packet.seqnum != expected_seqnum_) {
                gaps_detected_++;
                // Request retransmission or log gap
                handleGap(expected_seqnum_, packet.seqnum);
            }

            expected_seqnum_ = packet.seqnum + 1;

            // Process market data
            processMarketData(packet);
        }
    }

    void handleGap(uint64_t expected, uint64_t received) {
        // Log missing packets and request retransmission
        LOG_WARNING("Gap detected: expected=" << expected
                   << " received=" << received
                   << " gap=" << (received - expected));
    }

    void processMarketData(const MarketDataPacket& packet) {
        // Feed into order book
    }

    uint64_t getGapsDetected() const { return gaps_detected_; }
};
```

---

## 5. Lock-Free Programming Patterns

### 5.1 MPMC (Multi-Producer Multi-Consumer) Queue

**Problem**: Standard queues use locks, causing contention.

**Solution**: Lock-free MPMC queue using CAS operations.

```cpp
#include <atomic>
#include <array>

template<typename T, size_t Size>
class MPMCQueue {
private:
    struct Node {
        std::atomic<uint64_t> sequence;
        T data;
    };

    static constexpr size_t CACHE_LINE = 64;

    alignas(CACHE_LINE) std::atomic<uint64_t> enqueue_pos_{0};
    alignas(CACHE_LINE) std::atomic<uint64_t> dequeue_pos_{0};
    alignas(CACHE_LINE) std::array<Node, Size> buffer_;

public:
    MPMCQueue() {
        for (size_t i = 0; i < Size; ++i) {
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    bool enqueue(const T& data) {
        uint64_t pos = enqueue_pos_.load(std::memory_order_relaxed);

        while (true) {
            Node& node = buffer_[pos % Size];
            uint64_t seq = node.sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);

            if (diff == 0) {
                // Slot available, try to claim it
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1,
                    std::memory_order_relaxed)) {
                    node.data = data;
                    node.sequence.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                // Queue full
                return false;
            } else {
                // Another thread claimed this slot, try next
                pos = enqueue_pos_.load(std::memory_order_relaxed);
            }
        }
    }

    bool dequeue(T& data) {
        uint64_t pos = dequeue_pos_.load(std::memory_order_relaxed);

        while (true) {
            Node& node = buffer_[pos % Size];
            uint64_t seq = node.sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);

            if (diff == 0) {
                // Data available, try to claim it
                if (dequeue_pos_.compare_exchange_weak(pos, pos + 1,
                    std::memory_order_relaxed)) {
                    data = node.data;
                    node.sequence.store(pos + Size, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                // Queue empty
                return false;
            } else {
                // Another thread claimed this slot, try next
                pos = dequeue_pos_.load(std::memory_order_relaxed);
            }
        }
    }
};

// Usage: Multiple producers and consumers
MPMCQueue<Order, 65536> order_queue;

// Producer thread
void producerThread() {
    Order order{/* ... */};
    while (!order_queue.enqueue(order)) {
        // Spin or backoff
    }
}

// Consumer thread
void consumerThread() {
    Order order;
    while (!order_queue.dequeue(order)) {
        // Spin or backoff
    }
    processOrder(order);
}
```

### 5.2 Atomic Operations & Memory Ordering

**Understanding Memory Ordering**:
```cpp
#include <atomic>

class MemoryOrderingExample {
private:
    std::atomic<int> data_{0};
    std::atomic<bool> ready_{false};

public:
    // Producer
    void publish(int value) {
        data_.store(value, std::memory_order_relaxed);  // Can be reordered
        ready_.store(true, std::memory_order_release);  // Synchronizes with acquire
        // All writes before release are visible to acquire
    }

    // Consumer
    int consume() {
        while (!ready_.load(std::memory_order_acquire)) {
            // Wait
        }
        // All writes before release are now visible
        return data_.load(std::memory_order_relaxed);
    }
};

// Fastest: Relaxed ordering (no synchronization)
std::atomic<uint64_t> counter{0};
void incrementFast() {
    counter.fetch_add(1, std::memory_order_relaxed);
}

// Medium: Acquire-Release (synchronizes specific pairs)
std::atomic<Order*> order_ptr{nullptr};
void publishOrder(Order* order) {
    order_ptr.store(order, std::memory_order_release);
}
Order* getOrder() {
    return order_ptr.load(std::memory_order_acquire);
}

// Slowest: Sequential consistency (total ordering)
std::atomic<int> seq_counter{0};
void incrementSlow() {
    seq_counter.fetch_add(1);  // Default: memory_order_seq_cst
}
```

### 5.3 Lock-Free Stack

```cpp
template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        Node* next;
    };

    std::atomic<Node*> head_{nullptr};

public:
    void push(const T& data) {
        Node* new_node = new Node{data, nullptr};
        new_node->next = head_.load(std::memory_order_relaxed);

        // CAS loop: retry until successful
        while (!head_.compare_exchange_weak(
            new_node->next, new_node,
            std::memory_order_release,
            std::memory_order_relaxed)) {
            // CAS failed, new_node->next updated to current head
            // Retry with updated head
        }
    }

    bool pop(T& result) {
        Node* old_head = head_.load(std::memory_order_relaxed);

        while (old_head) {
            // Try to update head to old_head->next
            if (head_.compare_exchange_weak(
                old_head, old_head->next,
                std::memory_order_acquire,
                std::memory_order_relaxed)) {
                result = old_head->data;
                delete old_head;  // Warning: ABA problem possible
                return true;
            }
            // CAS failed, old_head updated to current head
            // Retry with updated head
        }

        return false;  // Stack empty
    }
};
```

**Note**: The above has ABA problem. Use hazard pointers or epoch-based reclamation in production.

---

## 6. SIMD Vectorization

### 6.1 Auto-Vectorization with Compiler Hints

**Problem**: Scalar operations are slow for bulk calculations.

**Solution**: Use SIMD instructions (AVX2/AVX-512) for parallel processing.

```cpp
#include <immintrin.h>  // AVX2 intrinsics
#include <cstdint>

// Calculate VWAP (Volume Weighted Average Price) for 1000 trades
// Scalar version
double calculateVWAP_Scalar(const uint32_t* prices, const uint32_t* volumes, size_t n) {
    uint64_t total_value = 0;
    uint64_t total_volume = 0;

    for (size_t i = 0; i < n; ++i) {
        total_value += static_cast<uint64_t>(prices[i]) * volumes[i];
        total_volume += volumes[i];
    }

    return static_cast<double>(total_value) / total_volume;
}

// AVX2 version (8x uint32_t per instruction)
double calculateVWAP_AVX2(const uint32_t* prices, const uint32_t* volumes, size_t n) {
    __m256i total_value_vec = _mm256_setzero_si256();
    __m256i total_volume_vec = _mm256_setzero_si256();

    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        // Load 8 prices and 8 volumes
        __m256i price_vec = _mm256_loadu_si256((__m256i*)&prices[i]);
        __m256i volume_vec = _mm256_loadu_si256((__m256i*)&volumes[i]);

        // Multiply prices * volumes (32-bit multiplication)
        __m256i value_vec = _mm256_mullo_epi32(price_vec, volume_vec);

        // Accumulate
        total_value_vec = _mm256_add_epi32(total_value_vec, value_vec);
        total_volume_vec = _mm256_add_epi32(total_volume_vec, volume_vec);
    }

    // Horizontal sum of vectors
    uint32_t total_value_array[8];
    uint32_t total_volume_array[8];
    _mm256_storeu_si256((__m256i*)total_value_array, total_value_vec);
    _mm256_storeu_si256((__m256i*)total_volume_array, total_volume_vec);

    uint64_t total_value = 0;
    uint64_t total_volume = 0;
    for (int j = 0; j < 8; ++j) {
        total_value += total_value_array[j];
        total_volume += total_volume_array[j];
    }

    // Handle remaining elements
    for (; i < n; ++i) {
        total_value += static_cast<uint64_t>(prices[i]) * volumes[i];
        total_volume += volumes[i];
    }

    return static_cast<double>(total_value) / total_volume;
}

// Compiler auto-vectorization (requires -O3 -march=native -ftree-vectorize)
double calculateVWAP_Auto(const uint32_t* __restrict__ prices,
                          const uint32_t* __restrict__ volumes,
                          size_t n) {
    uint64_t total_value = 0;
    uint64_t total_volume = 0;

    #pragma GCC ivdep  // Ignore vector dependencies
    for (size_t i = 0; i < n; ++i) {
        total_value += static_cast<uint64_t>(prices[i]) * volumes[i];
        total_volume += volumes[i];
    }

    return static_cast<double>(total_value) / total_volume;
}
```

### 6.2 Vectorized Option Pricing (Black-Scholes)

```cpp
#include <immintrin.h>
#include <cmath>

// Vectorized Black-Scholes for 8 options simultaneously
void blackScholes_AVX2(
    const float* S,     // Stock prices (8 values)
    const float* K,     // Strike prices (8 values)
    const float* T,     // Time to maturity (8 values)
    const float* r,     // Risk-free rate (8 values)
    const float* sigma, // Volatility (8 values)
    float* call_prices  // Output: call prices (8 values)
) {
    __m256 S_vec = _mm256_loadu_ps(S);
    __m256 K_vec = _mm256_loadu_ps(K);
    __m256 T_vec = _mm256_loadu_ps(T);
    __m256 r_vec = _mm256_loadu_ps(r);
    __m256 sigma_vec = _mm256_loadu_ps(sigma);

    // d1 = (ln(S/K) + (r + sigma²/2) * T) / (sigma * sqrt(T))
    __m256 ln_S_K = _mm256_log_ps(_mm256_div_ps(S_vec, K_vec));

    __m256 sigma_sq = _mm256_mul_ps(sigma_vec, sigma_vec);
    __m256 half = _mm256_set1_ps(0.5f);
    __m256 r_plus_half_sigma_sq = _mm256_add_ps(r_vec, _mm256_mul_ps(sigma_sq, half));

    __m256 sqrt_T = _mm256_sqrt_ps(T_vec);
    __m256 sigma_sqrt_T = _mm256_mul_ps(sigma_vec, sqrt_T);

    __m256 numerator = _mm256_add_ps(ln_S_K, _mm256_mul_ps(r_plus_half_sigma_sq, T_vec));
    __m256 d1 = _mm256_div_ps(numerator, sigma_sqrt_T);

    // d2 = d1 - sigma * sqrt(T)
    __m256 d2 = _mm256_sub_ps(d1, sigma_sqrt_T);

    // N(d1) and N(d2) - cumulative normal distribution
    __m256 Nd1 = cdf_norm_avx2(d1);
    __m256 Nd2 = cdf_norm_avx2(d2);

    // Call price = S * N(d1) - K * exp(-r*T) * N(d2)
    __m256 neg_rT = _mm256_mul_ps(_mm256_set1_ps(-1.0f), _mm256_mul_ps(r_vec, T_vec));
    __m256 exp_neg_rT = _mm256_exp_ps(neg_rT);

    __m256 term1 = _mm256_mul_ps(S_vec, Nd1);
    __m256 term2 = _mm256_mul_ps(_mm256_mul_ps(K_vec, exp_neg_rT), Nd2);

    __m256 call_price_vec = _mm256_sub_ps(term1, term2);

    _mm256_storeu_ps(call_prices, call_price_vec);
}

// Approximate CDF of standard normal distribution (vectorized)
__m256 cdf_norm_avx2(__m256 x) {
    // Abramowitz and Stegun approximation
    const __m256 a1 = _mm256_set1_ps(0.254829592f);
    const __m256 a2 = _mm256_set1_ps(-0.284496736f);
    const __m256 a3 = _mm256_set1_ps(1.421413741f);
    const __m256 a4 = _mm256_set1_ps(-1.453152027f);
    const __m256 a5 = _mm256_set1_ps(1.061405429f);
    const __m256 p = _mm256_set1_ps(0.3275911f);

    __m256 sign = _mm256_and_ps(x, _mm256_set1_ps(-0.0f));  // Extract sign bit
    __m256 abs_x = _mm256_andnot_ps(_mm256_set1_ps(-0.0f), x);  // Absolute value

    __m256 t = _mm256_div_ps(_mm256_set1_ps(1.0f),
                             _mm256_add_ps(_mm256_set1_ps(1.0f),
                                          _mm256_mul_ps(p, abs_x)));

    __m256 y = _mm256_sub_ps(_mm256_set1_ps(1.0f),
                             _mm256_mul_ps(
                                 _mm256_mul_ps(
                                     _mm256_add_ps(
                                         _mm256_mul_ps(
                                             _mm256_add_ps(
                                                 _mm256_mul_ps(
                                                     _mm256_add_ps(
                                                         _mm256_mul_ps(
                                                             _mm256_add_ps(
                                                                 _mm256_mul_ps(a5, t), a4), t), a3), t), a2), t), a1), t),
                                 _mm256_exp_ps(_mm256_mul_ps(_mm256_set1_ps(-0.5f),
                                                             _mm256_mul_ps(x, x)))));

    // Apply sign
    __m256 result = _mm256_blendv_ps(y, _mm256_sub_ps(_mm256_set1_ps(1.0f), y), sign);
    return result;
}
```

---

## 7. I/O Optimization

### 7.1 io_uring for Async I/O (Modern Linux)

**Problem**: Traditional I/O blocks threads.

**Solution**: Use io_uring for true async I/O with kernel bypass.

```cpp
#include <liburing.h>
#include <fcntl.h>

class IOUringReader {
private:
    struct io_uring ring_;
    static constexpr int QUEUE_DEPTH = 256;

public:
    IOUringReader() {
        io_uring_queue_init(QUEUE_DEPTH, &ring_, 0);
    }

    ~IOUringReader() {
        io_uring_queue_exit(&ring_);
    }

    void submitReadRequest(int fd, void* buffer, size_t size, off_t offset, void* user_data) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
        if (!sqe) {
            throw std::runtime_error("SQ full");
        }

        io_uring_prep_read(sqe, fd, buffer, size, offset);
        io_uring_sqe_set_data(sqe, user_data);
        io_uring_submit(&ring_);
    }

    void processCompletions() {
        struct io_uring_cqe* cqe;
        unsigned head;
        unsigned count = 0;

        io_uring_for_each_cqe(&ring_, head, cqe) {
            void* user_data = io_uring_cqe_get_data(cqe);
            int result = cqe->res;

            if (result < 0) {
                std::cerr << "I/O error: " << strerror(-result) << std::endl;
            } else {
                handleCompletion(user_data, result);
            }

            count++;
        }

        io_uring_cq_advance(&ring_, count);
    }

    void handleCompletion(void* user_data, int bytes_read) {
        // Process read data
    }
};

// Usage: Read market data file with io_uring
struct ReadContext {
    char buffer[4096];
    int file_offset;
};

void readMarketDataAsync() {
    IOUringReader reader;
    int fd = open("market_data.bin", O_RDONLY | O_DIRECT);

    std::vector<ReadContext> contexts(10);

    // Submit 10 read requests
    for (int i = 0; i < 10; ++i) {
        contexts[i].file_offset = i * 4096;
        reader.submitReadRequest(fd, contexts[i].buffer, 4096,
                                contexts[i].file_offset, &contexts[i]);
    }

    // Process completions
    reader.processCompletions();
}
```

### 7.2 Direct I/O (O_DIRECT)

**Problem**: Page cache adds latency for real-time data.

**Solution**: Bypass page cache with direct I/O.

```cpp
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

class DirectIOReader {
private:
    int fd_;
    static constexpr size_t ALIGNMENT = 4096;  // Sector size

    void* alignedAlloc(size_t size) {
        void* ptr;
        if (posix_memalign(&ptr, ALIGNMENT, size) != 0) {
            throw std::bad_alloc();
        }
        return ptr;
    }

public:
    DirectIOReader(const char* filename) {
        fd_ = open(filename, O_RDONLY | O_DIRECT);
        if (fd_ == -1) {
            throw std::runtime_error("Cannot open file with O_DIRECT");
        }
    }

    ~DirectIOReader() {
        if (fd_ != -1) close(fd_);
    }

    ssize_t read(void* buffer, size_t size, off_t offset) {
        // buffer must be aligned to sector boundary
        // size must be multiple of sector size
        // offset must be multiple of sector size
        return pread(fd_, buffer, size, offset);
    }

    void readTickData() {
        size_t buffer_size = 1024 * 1024;  // 1MB, sector-aligned
        void* buffer = alignedAlloc(buffer_size);

        off_t offset = 0;
        ssize_t bytes_read;

        while ((bytes_read = read(buffer, buffer_size, offset)) > 0) {
            processTickBuffer(buffer, bytes_read);
            offset += bytes_read;
        }

        free(buffer);
    }

    void processTickBuffer(void* buffer, size_t size) {
        // Parse ticks
    }
};
```

---

## 8. Python Performance Optimization

### 8.1 NumPy Vectorization

**Problem**: Python loops are extremely slow.

**Solution**: Use NumPy for vectorized operations (C-speed).

```python
import numpy as np
import time

# BAD: Pure Python loop
def calculate_returns_slow(prices):
    returns = []
    for i in range(1, len(prices)):
        ret = (prices[i] - prices[i-1]) / prices[i-1]
        returns.append(ret)
    return returns

# GOOD: NumPy vectorization
def calculate_returns_fast(prices):
    return np.diff(prices) / prices[:-1]

# Benchmark
prices = np.random.rand(1000000) * 100

start = time.perf_counter()
returns_slow = calculate_returns_slow(prices.tolist())
time_slow = time.perf_counter() - start

start = time.perf_counter()
returns_fast = calculate_returns_fast(prices)
time_fast = time.perf_counter() - start

print(f"Slow: {time_slow:.4f}s")  # ~0.5s
print(f"Fast: {time_fast:.4f}s")  # ~0.002s
print(f"Speedup: {time_slow/time_fast:.1f}x")  # ~250x faster
```

### 8.2 Numba JIT Compilation

**Problem**: Complex calculations can't be vectorized.

**Solution**: Use Numba to compile Python to machine code.

```python
import numba
from numba import jit, njit, prange
import numpy as np

# Monte Carlo option pricing
@njit(parallel=True, fastmath=True)
def monte_carlo_option_numba(S0, K, T, r, sigma, num_sims):
    """
    S0: Initial stock price
    K: Strike price
    T: Time to maturity
    r: Risk-free rate
    sigma: Volatility
    num_sims: Number of simulations
    """
    payoffs = np.zeros(num_sims)
    dt = T / 252  # Daily steps

    for i in prange(num_sims):  # Parallel loop
        S = S0
        for t in range(252):
            z = np.random.standard_normal()
            S = S * np.exp((r - 0.5 * sigma**2) * dt + sigma * np.sqrt(dt) * z)

        payoffs[i] = max(S - K, 0)  # Call option payoff

    price = np.exp(-r * T) * np.mean(payoffs)
    return price

# Benchmark
result = monte_carlo_option_numba(100, 105, 1.0, 0.05, 0.2, 1000000)
print(f"Option price: {result:.4f}")

# ~100x faster than pure Python
# Uses all CPU cores with prange
```

### 8.3 Cython for C-Speed Python

**Problem**: Critical path needs C performance.

**Solution**: Use Cython to compile to C.

**pricing_engine.pyx**:
```cython
# cython: language_level=3
# cython: boundscheck=False
# cython: wraparound=False
# cython: cdivision=True

import numpy as np
cimport numpy as cnp
from libc.math cimport exp, sqrt, log
cimport cython

@cython.boundscheck(False)
@cython.wraparound(False)
def black_scholes_cython(double[:] S, double[:] K, double[:] T,
                          double[:] r, double[:] sigma):
    """Vectorized Black-Scholes in Cython"""
    cdef int n = S.shape[0]
    cdef cnp.ndarray[cnp.float64_t, ndim=1] call_prices = np.zeros(n, dtype=np.float64)

    cdef int i
    cdef double d1, d2, Nd1, Nd2
    cdef double sqrt_T, sigma_sqrt_T

    for i in range(n):
        sqrt_T = sqrt(T[i])
        sigma_sqrt_T = sigma[i] * sqrt_T

        d1 = (log(S[i] / K[i]) + (r[i] + 0.5 * sigma[i]**2) * T[i]) / sigma_sqrt_T
        d2 = d1 - sigma_sqrt_T

        Nd1 = norm_cdf(d1)
        Nd2 = norm_cdf(d2)

        call_prices[i] = S[i] * Nd1 - K[i] * exp(-r[i] * T[i]) * Nd2

    return call_prices

cdef double norm_cdf(double x) nogil:
    """Cumulative distribution function for standard normal"""
    return 0.5 * (1.0 + erf(x / sqrt(2.0)))

from libc.math cimport erf
```

**setup.py**:
```python
from setuptools import setup
from Cython.Build import cythonize
import numpy

setup(
    ext_modules=cythonize("pricing_engine.pyx",
                          compiler_directives={'language_level': "3"}),
    include_dirs=[numpy.get_include()]
)
```

Build: `python setup.py build_ext --inplace`

### 8.4 PyPy for JIT Optimization

**Problem**: CPython interpreter overhead.

**Solution**: Use PyPy for automatic JIT compilation.

```bash
# Install PyPy
wget https://downloads.python.org/pypy/pypy3.10-v7.3.12-linux64.tar.bz2
tar xf pypy3.10-v7.3.12-linux64.tar.bz2

# Run with PyPy (3-5x faster for numerical code)
./pypy3.10-v7.3.12-linux64/bin/pypy3 trading_strategy.py
```

### 8.5 Memory-Efficient Data Structures

```python
import pandas as pd
import numpy as np

# BAD: Default dtypes waste memory
df = pd.read_csv('market_data.csv')
print(df.memory_usage(deep=True).sum() / 1024**2, "MB")  # e.g., 500 MB

# GOOD: Optimize dtypes
df = pd.read_csv('market_data.csv', dtype={
    'timestamp': 'int64',
    'price': 'float32',  # Instead of float64
    'volume': 'int32',   # Instead of int64
    'symbol': 'category'  # Instead of object (string)
})
print(df.memory_usage(deep=True).sum() / 1024**2, "MB")  # e.g., 150 MB

# Convert existing DataFrame
def optimize_dataframe(df):
    for col in df.columns:
        col_type = df[col].dtype

        if col_type == 'float64':
            df[col] = df[col].astype('float32')
        elif col_type == 'int64':
            df[col] = df[col].astype('int32')
        elif col_type == 'object':
            df[col] = df[col].astype('category')

    return df

df_optimized = optimize_dataframe(df)
```

### 8.6 Multiprocessing for CPU-Bound Tasks

```python
from multiprocessing import Pool, cpu_count
import numpy as np

def calculate_strategy_signals(params):
    """CPU-intensive strategy calculation"""
    lookback, threshold = params
    # Expensive calculation
    signals = complex_calculation(lookback, threshold)
    return signals

# BAD: Sequential
results = []
for params in parameter_combinations:
    result = calculate_strategy_signals(params)
    results.append(result)

# GOOD: Parallel
if __name__ == '__main__':
    with Pool(cpu_count()) as pool:
        results = pool.map(calculate_strategy_signals, parameter_combinations)

    # ~8x faster on 8-core machine
```

---

## 9. Profiling & Debugging Tools

### 9.1 Performance Profiling

**Linux perf**:
```bash
# Record performance data
perf record -g ./hft_server

# Analyze with flamegraph
perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg

# Find cache misses
perf stat -e cache-references,cache-misses,cycles,instructions ./hft_server

# Monitor specific functions
perf record -e cycles -g --call-graph dwarf ./hft_server
perf report
```

**Valgrind Cachegrind**:
```bash
# Cache profiling
valgrind --tool=cachegrind --cache-sim=yes ./hft_server

# Annotate source code
cg_annotate cachegrind.out.<pid>

# Visualize with KCachegrind
kcachegrind cachegrind.out.<pid>
```

**Intel VTune**:
```bash
# Hotspot analysis
vtune -collect hotspots -result-dir vtune_results ./hft_server

# Memory access analysis
vtune -collect memory-access -result-dir vtune_results ./hft_server

# Microarchitecture analysis
vtune -collect uarch-exploration -result-dir vtune_results ./hft_server
```

### 9.2 Latency Measurement

**TSC-Based Timer (Nanosecond Precision)**:
```cpp
#include <x86intrin.h>

class TSCTimer {
private:
    uint64_t start_tsc_;
    static double tsc_freq_ghz_;

public:
    static void calibrate() {
        // Measure TSC frequency
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t tsc_start = __rdtsc();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        uint64_t tsc_end = __rdtsc();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start).count();

        tsc_freq_ghz_ = static_cast<double>(tsc_end - tsc_start) / duration_ns;
    }

    void start() {
        _mm_lfence();  // Serialize instruction stream
        start_tsc_ = __rdtsc();
        _mm_lfence();
    }

    uint64_t elapsed_ns() {
        _mm_lfence();
        uint64_t end_tsc = __rdtsc();
        _mm_lfence();

        return static_cast<uint64_t>((end_tsc - start_tsc_) / tsc_freq_ghz_);
    }
};

double TSCTimer::tsc_freq_ghz_ = 0.0;

// Usage
TSCTimer::calibrate();

TSCTimer timer;
timer.start();
processOrder(order);
uint64_t latency_ns = timer.elapsed_ns();

std::cout << "Order processing latency: " << latency_ns << " ns" << std::endl;
```

### 9.3 Python Profiling

```python
import cProfile
import pstats
from line_profiler import LineProfiler

# Function profiling
def trading_strategy():
    # Complex logic
    pass

# Profile with cProfile
profiler = cProfile.Profile()
profiler.enable()
trading_strategy()
profiler.disable()

stats = pstats.Stats(profiler)
stats.sort_stats('cumulative')
stats.print_stats(20)  # Top 20 functions

# Line-by-line profiling
lp = LineProfiler()
lp.add_function(trading_strategy)
lp.enable()
trading_strategy()
lp.disable()
lp.print_stats()

# Memory profiling
from memory_profiler import profile

@profile
def memory_intensive_function():
    data = [i**2 for i in range(1000000)]
    return data
```

---

## 10. Real-World Optimization Examples

### 10.1 Complete Order Book Implementation

```cpp
template<typename PriceType, typename QtyType, size_t MaxLevels = 10000>
class OptimizedOrderBook {
private:
    struct Order {
        uint64_t order_id;
        QtyType quantity;
        Order* next;
        Order* prev;
    };

    struct PriceLevel {
        PriceType price;
        QtyType total_quantity;
        Order* head;
        Order* tail;
        size_t order_count;
    };

    // Cache-aligned arrays for bids and asks
    alignas(64) std::array<PriceLevel, MaxLevels> bids_;
    alignas(64) std::array<PriceLevel, MaxLevels> asks_;

    // Memory pool for orders
    FastMemoryPool<Order, 65536> order_pool_;

    // Price to level index mapping
    std::unordered_map<PriceType, size_t> bid_price_to_level_;
    std::unordered_map<PriceType, size_t> ask_price_to_level_;

    size_t best_bid_level_ = 0;
    size_t best_ask_level_ = 0;

public:
    // Add limit order - O(1) amortized
    void addOrder(uint64_t order_id, bool is_buy, PriceType price, QtyType qty) {
        Order* order = order_pool_.allocate();
        new (order) Order{order_id, qty, nullptr, nullptr};

        if (is_buy) {
            size_t level = getBidLevel(price);
            addOrderToLevel(bids_[level], order, price);
            if (price > bids_[best_bid_level_].price) {
                best_bid_level_ = level;
            }
        } else {
            size_t level = getAskLevel(price);
            addOrderToLevel(asks_[level], order, price);
            if (price < asks_[best_ask_level_].price ||
                asks_[best_ask_level_].order_count == 0) {
                best_ask_level_ = level;
            }
        }
    }

    // Cancel order - O(1)
    void cancelOrder(uint64_t order_id, bool is_buy, PriceType price) {
        // Implementation details...
    }

    // Get best bid/ask - O(1)
    std::pair<PriceType, QtyType> getBestBid() const {
        const PriceLevel& level = bids_[best_bid_level_];
        return {level.price, level.total_quantity};
    }

    std::pair<PriceType, QtyType> getBestAsk() const {
        const PriceLevel& level = asks_[best_ask_level_];
        return {level.price, level.total_quantity};
    }

private:
    size_t getBidLevel(PriceType price) {
        auto it = bid_price_to_level_.find(price);
        if (it != bid_price_to_level_.end()) {
            return it->second;
        }

        size_t level = bid_price_to_level_.size();
        bid_price_to_level_[price] = level;
        return level;
    }

    size_t getAskLevel(PriceType price) {
        auto it = ask_price_to_level_.find(price);
        if (it != ask_price_to_level_.end()) {
            return it->second;
        }

        size_t level = ask_price_to_level_.size();
        ask_price_to_level_[price] = level;
        return level;
    }

    void addOrderToLevel(PriceLevel& level, Order* order, PriceType price) {
        if (level.head == nullptr) {
            level.price = price;
            level.head = level.tail = order;
        } else {
            level.tail->next = order;
            order->prev = level.tail;
            level.tail = order;
        }

        level.total_quantity += order->quantity;
        level.order_count++;
    }
};
```

### 10.2 Complete Market Data Handler

```cpp
class MarketDataHandler {
private:
    MPMCQueue<MarketDataMessage, 1048576> input_queue_;
    OptimizedOrderBook<uint32_t, uint32_t> order_book_;

    std::atomic<bool> running_{true};
    std::thread processing_thread_;

    // Statistics
    alignas(64) std::atomic<uint64_t> messages_processed_{0};
    alignas(64) std::atomic<uint64_t> total_latency_ns_{0};

public:
    MarketDataHandler() {
        processing_thread_ = std::thread(&MarketDataHandler::processingLoop, this);

        // Pin to isolated CPU
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(2, &cpuset);
        pthread_setaffinity_np(processing_thread_.native_handle(),
                              sizeof(cpu_set_t), &cpuset);

        // Set real-time priority
        struct sched_param param;
        param.sched_priority = 99;
        pthread_setschedparam(processing_thread_.native_handle(),
                             SCHED_FIFO, &param);
    }

    void processingLoop() {
        MarketDataMessage msg;
        TSCTimer timer;

        while (running_.load(std::memory_order_relaxed)) {
            if (input_queue_.dequeue(msg)) {
                timer.start();

                processMessage(msg);

                uint64_t latency = timer.elapsed_ns();
                messages_processed_.fetch_add(1, std::memory_order_relaxed);
                total_latency_ns_.fetch_add(latency, std::memory_order_relaxed);
            }
        }
    }

    void processMessage(const MarketDataMessage& msg) {
        switch (msg.type) {
            case MessageType::ADD_ORDER:
                order_book_.addOrder(msg.order_id, msg.is_buy, msg.price, msg.quantity);
                break;
            case MessageType::CANCEL_ORDER:
                order_book_.cancelOrder(msg.order_id, msg.is_buy, msg.price);
                break;
            case MessageType::TRADE:
                handleTrade(msg);
                break;
        }
    }

    void printStatistics() {
        uint64_t count = messages_processed_.load(std::memory_order_relaxed);
        uint64_t total_latency = total_latency_ns_.load(std::memory_order_relaxed);

        std::cout << "Messages processed: " << count << std::endl;
        std::cout << "Average latency: " << (total_latency / count) << " ns" << std::endl;
    }
};
```

---

## Summary: Quick Reference

### Linux System Tuning Checklist
- [ ] CPU isolation (`isolcpus`, `nohz_full`)
- [ ] CPU governor set to `performance`
- [ ] Disable Turbo Boost (reduce jitter)
- [ ] Configure huge pages (2MB/1GB)
- [ ] NUMA binding
- [ ] IRQ affinity to housekeeping cores
- [ ] Disable swap
- [ ] Network buffer tuning
- [ ] TCP_NODELAY and TCP_QUICKACK
- [ ] Enable io_uring

### C++ Optimization Checklist
- [ ] Compiler flags: `-O3 -march=native -flto -ffast-math`
- [ ] Cache line alignment (64 bytes)
- [ ] Lock-free data structures
- [ ] Memory pools (no malloc/free in hot path)
- [ ] SIMD vectorization (AVX2/AVX-512)
- [ ] Thread pinning and real-time priority
- [ ] Prefetching for linked structures
- [ ] Structure of Arrays (SoA) layout
- [ ] TSC-based timing
- [ ] Huge pages for large allocations

### Python Optimization Checklist
- [ ] NumPy vectorization
- [ ] Numba JIT compilation
- [ ] Cython for critical paths
- [ ] Optimize DataFrame dtypes
- [ ] Multiprocessing for parallelism
- [ ] PyPy for numerical code
- [ ] Avoid Python loops
- [ ] Use memory-mapped files

### Profiling Tools
- **Linux perf**: CPU profiling, cache analysis
- **Valgrind**: Memory leaks, cache misses
- **Intel VTune**: Microarchitecture analysis
- **cProfile/line_profiler**: Python profiling
- **TSC timer**: Nanosecond latency measurement

---

**Performance Targets for HFT:**
- Order book operations: < 200 ns
- Market data processing: < 500 ns
- End-to-end order latency: < 1 μs
- 99th percentile jitter: < 10 μs

This guide covers the essential techniques to achieve these targets.

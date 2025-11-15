# Module 7: Low-Latency Application Design

## Overview

This final module brings together all concepts to design and build ultra-low latency network applications, with real-world examples from high-frequency trading, gaming, and real-time systems.

## Table of Contents

1. [Understanding Latency](#understanding-latency)
2. [Latency Measurement](#latency-measurement)
3. [Lock-Free Programming](#lock-free-programming)
4. [Cache Optimization](#cache-optimization)
5. [Jitter Reduction](#jitter-reduction)
6. [Hardware Timestamping](#hardware-timestamping)
7. [Real-World Case Studies](#real-world-case-studies)
8. [Complete Low-Latency Application](#complete-low-latency-application)

---

## Understanding Latency

### Latency Budget

```
Total Latency = Network + OS + Application + Processing

Example: High-Frequency Trading
Target: <10 μs one-way

┌─────────────────────────────────────────────┐
│ Component              │ Latency  │ Budget │
├────────────────────────┼──────────┼────────┤
│ Network (switch)       │ 0.5 μs   │  5%    │
│ NIC (kernel bypass)    │ 0.5 μs   │  5%    │
│ Application processing │ 2.0 μs   │ 20%    │
│ Message encoding       │ 1.0 μs   │ 10%    │
│ Network transmission   │ 4.0 μs   │ 40%    │
│ Jitter buffer          │ 2.0 μs   │ 20%    │
├────────────────────────┼──────────┼────────┤
│ Total                  │ 10.0 μs  │ 100%   │
└─────────────────────────────────────────────┘

Every microsecond counts!
```

### Latency Numbers Every Programmer Should Know

```
Operation                          Time (ns)   Time (μs)
──────────────────────────────────────────────────────
L1 cache reference                     1 ns
Branch mispredict                      3 ns
L2 cache reference                     4 ns
Mutex lock/unlock                     25 ns
L3 cache reference                    40 ns
Main memory reference                100 ns      0.1 μs
──────────────────────────────────────────────────────
Kernel syscall                     1,000 ns      1.0 μs
Context switch                     3,000 ns      3.0 μs
──────────────────────────────────────────────────────
1 Gbps network: 1 KB            10,000 ns     10.0 μs
SSD random read                100,000 ns    100.0 μs
──────────────────────────────────────────────────────
Send packet: CA→Netherlands  150,000,000 ns 150,000 μs (150 ms)

Key Insight:
- Memory access: 100 ns
- Syscall: 1,000 ns (10x slower!)
- Context switch: 3,000 ns (30x slower!)

Optimization Goal: Stay in userspace, avoid syscalls!
```

---

## Latency Measurement

### High-Resolution Timing

```cpp
#include <chrono>
#include <iostream>

class LatencyTimer {
private:
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    time_point start_time;

public:
    void start() {
        start_time = clock::now();
    }

    uint64_t elapsed_ns() const {
        auto end = clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start_time).count();
    }

    double elapsed_us() const {
        return elapsed_ns() / 1000.0;
    }
};

// Usage
LatencyTimer timer;
timer.start();
// ... operation ...
std::cout << "Latency: " << timer.elapsed_us() << " μs\n";
```

### Hardware Timestamps (RDTSC)

```cpp
#include <x86intrin.h>

// Read CPU timestamp counter (fastest!)
inline uint64_t rdtsc() {
    return __rdtsc();
}

// Serializing version (prevents reordering)
inline uint64_t rdtscp() {
    unsigned int aux;
    return __rdtscp(&aux);
}

// Convert cycles to nanoseconds
double cycles_to_ns(uint64_t cycles, double cpu_ghz) {
    return cycles / cpu_ghz;
}

// Usage
uint64_t start = rdtsc();
// ... operation ...
uint64_t end = rdtsc();
uint64_t cycles = end - start;

// Assuming 3.0 GHz CPU
double ns = cycles_to_ns(cycles, 3.0);
std::cout << "Latency: " << ns << " ns (" << cycles << " cycles)\n";

/*
 * RDTSC advantages:
 * - Extremely fast (~20 cycles)
 * - Sub-nanosecond resolution
 * - No syscall overhead
 *
 * Cautions:
 * - Can be affected by frequency scaling
 * - Disable turbo boost for consistency
 * - Use rdtscp() to prevent reordering
 */
```

### Percentile Measurement

```cpp
#include <vector>
#include <algorithm>
#include <cmath>

class LatencyStats {
private:
    std::vector<double> samples;

public:
    void record(double latency_us) {
        samples.push_back(latency_us);
    }

    void print_percentiles() {
        if (samples.empty()) return;

        std::sort(samples.begin(), samples.end());
        size_t n = samples.size();

        auto percentile = [&](double p) {
            size_t idx = static_cast<size_t>(std::ceil(n * p / 100.0)) - 1;
            return samples[std::min(idx, n - 1)];
        };

        std::cout << "Latency Statistics (μs):\n";
        std::cout << "  Count: " << n << "\n";
        std::cout << "  Min:   " << samples[0] << "\n";
        std::cout << "  p50:   " << percentile(50) << "\n";
        std::cout << "  p90:   " << percentile(90) << "\n";
        std::cout << "  p95:   " << percentile(95) << "\n";
        std::cout << "  p99:   " << percentile(99) << "\n";
        std::cout << "  p99.9: " << percentile(99.9) << "\n";
        std::cout << "  p99.99:" << percentile(99.99) << "\n";
        std::cout << "  Max:   " << samples[n - 1] << "\n";

        // Calculate standard deviation
        double mean = 0;
        for (double s : samples) mean += s;
        mean /= n;

        double variance = 0;
        for (double s : samples) {
            variance += (s - mean) * (s - mean);
        }
        variance /= n;

        std::cout << "  Mean:  " << mean << "\n";
        std::cout << "  StdDev:" << std::sqrt(variance) << "\n";
    }
};

/*
 * Why percentiles matter:
 * - p99: 99 out of 100 requests
 * - p99.9: 999 out of 1000 requests
 * - Tail latency kills user experience
 *
 * For HFT:
 * - p99.99 or better required
 * - One slow packet = missed opportunity
 */
```

---

## Lock-Free Programming

### Why Lock-Free?

```
With Locks:
Thread A: lock() → critical section → unlock()
Thread B:        wait (blocked)       lock() → ...

Latency: ~1-10 μs (lock contention)
Jitter: High (unpredictable waits)


Lock-Free:
Thread A: atomic_compare_exchange()
Thread B: atomic_compare_exchange()

Latency: ~10-100 ns
Jitter: Low (no blocking)
```

### Lock-Free Queue (SPSC)

```cpp
#include <atomic>
#include <array>

// Single Producer Single Consumer Lock-Free Queue
template<typename T, size_t Size>
class SPSCQueue {
private:
    std::array<T, Size> buffer;
    alignas(64) std::atomic<size_t> write_pos{0};  // Separate cache lines
    alignas(64) std::atomic<size_t> read_pos{0};

public:
    // Producer: Try to enqueue
    bool try_push(const T& item) {
        size_t w = write_pos.load(std::memory_order_relaxed);
        size_t next_w = (w + 1) % Size;

        // Check if queue is full
        if (next_w == read_pos.load(std::memory_order_acquire)) {
            return false;  // Queue full
        }

        buffer[w] = item;
        write_pos.store(next_w, std::memory_order_release);
        return true;
    }

    // Consumer: Try to dequeue
    bool try_pop(T& item) {
        size_t r = read_pos.load(std::memory_order_relaxed);

        // Check if queue is empty
        if (r == write_pos.load(std::memory_order_acquire)) {
            return false;  // Queue empty
        }

        item = buffer[r];
        read_pos.store((r + 1) % Size, std::memory_order_release);
        return true;
    }
};

// Usage
SPSCQueue<int, 1024> queue;

// Producer thread
queue.try_push(42);

// Consumer thread
int value;
if (queue.try_pop(value)) {
    // Process value
}

/*
 * Performance:
 * - 10-20 ns per operation
 * - No locks, no syscalls
 * - Cache-line aligned (false sharing prevention)
 *
 * Memory ordering:
 * - relaxed: No synchronization (fast!)
 * - acquire/release: Synchronize between threads
 * - seq_cst: Full fence (slowest)
 */
```

### Lock-Free vs Lock-Based Performance

```
Benchmark: 1M operations

Lock-based (std::mutex):
- Latency: 1,200 ns per operation
- Total time: 1.2 seconds
- Jitter: High (50-5000 ns)

Lock-free (atomic):
- Latency: 15 ns per operation
- Total time: 15 milliseconds
- Jitter: Low (10-30 ns)

Speedup: 80x faster!
```

---

## Cache Optimization

### Cache Line Awareness

```cpp
// Cache line size: typically 64 bytes
constexpr size_t CACHE_LINE_SIZE = 64;

// BAD: False sharing
struct BadCounter {
    std::atomic<int> counter_a;  // 4 bytes
    std::atomic<int> counter_b;  // 4 bytes (same cache line!)
};
// Thread A writes counter_a → invalidates cache line
// Thread B writes counter_b → cache line bouncing!

// GOOD: Cache line aligned
struct alignas(CACHE_LINE_SIZE) GoodCounter {
    std::atomic<int> counter_a;
    char pad1[CACHE_LINE_SIZE - sizeof(std::atomic<int>)];
};

struct alignas(CACHE_LINE_SIZE) GoodCounter2 {
    std::atomic<int> counter_b;
    char pad2[CACHE_LINE_SIZE - sizeof(std::atomic<int>)];
};

/*
 * Or use C++17:
 */
#ifdef __cpp_lib_hardware_interference_size
constexpr size_t CACHE_LINE = std::hardware_destructive_interference_size;
#else
constexpr size_t CACHE_LINE = 64;
#endif

struct alignas(CACHE_LINE) AlignedData {
    std::atomic<int> value;
};
```

### Data Structure Layout

```cpp
// BAD: Poor cache locality
struct BadPacket {
    uint32_t seq_num;      // 4 bytes
    char* payload;         // 8 bytes (pointer to heap)
    uint32_t payload_len;  // 4 bytes
    double timestamp;      // 8 bytes
};

// Access pattern:
// 1. Read seq_num (cache miss)
// 2. Read payload (cache miss - different memory!)
// 3. Read payload_len (cache hit)
// 4. Read timestamp (cache hit)

// GOOD: Inline data for cache locality
struct GoodPacket {
    uint32_t seq_num;      // 4 bytes
    uint32_t payload_len;  // 4 bytes
    double timestamp;      // 8 bytes
    char payload[1400];    // Inline! (MTU size)
};

// Access pattern:
// 1. Read entire struct (few cache misses)
// 2. All fields in same cache lines
// 3. Sequential access (prefetcher friendly)

/*
 * Performance impact:
 * - Bad: ~200 ns (multiple cache misses)
 * - Good: ~20 ns (sequential access)
 * - 10x faster!
 */
```

### Prefetching

```cpp
// Manual prefetching
void process_packets(const Packet* packets, size_t count) {
    for (size_t i = 0; i < count; i++) {
        // Prefetch next packet while processing current
        if (i + 1 < count) {
            __builtin_prefetch(&packets[i + 1], 0, 3);
            // Parameters:
            // - Address to prefetch
            // - 0 = read, 1 = write
            // - 3 = temporal locality (0-3)
        }

        // Process current packet
        process(packets[i]);
    }
}

/*
 * Prefetch strategies:
 * - T0: No temporal locality (streaming data)
 * - T1: Low temporal locality
 * - T2: Moderate temporal locality
 * - T3: High temporal locality (keep in cache)
 */
```

---

## Jitter Reduction

### Sources of Jitter

```
1. Operating System
   - Context switches
   - Interrupts
   - Process scheduling

2. Hardware
   - CPU frequency scaling
   - Power management
   - Thermal throttling

3. Network
   - Queue depth
   - Switch buffering
   - Retransmissions

4. Application
   - Garbage collection
   - Memory allocation
   - Lock contention
```

### System Configuration

```bash
#!/bin/bash
# Low-latency system configuration

# 1. Isolate CPU cores
# Edit /etc/default/grub:
GRUB_CMDLINE_LINUX="isolcpus=2,3,4,5 nohz_full=2,3,4,5 rcu_nocbs=2,3,4,5"
# Update grub and reboot

# 2. Disable CPU frequency scaling
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    echo performance > $cpu
done

# 3. Disable turbo boost (for consistent latency)
echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo

# 4. Set IRQ affinity (pin NIC interrupts to specific cores)
echo 2 > /proc/irq/45/smp_affinity_list  # Pin to core 2

# 5. Disable transparent huge pages
echo never > /sys/kernel/mm/transparent_hugepage/enabled

# 6. Increase priority of network processing
echo -1 > /proc/sys/kernel/sched_rt_runtime_us  # Allow 100% RT

# 7. Disable swap
swapoff -a

# 8. Set process priority
chrt -f 99 ./my_low_latency_app  # Real-time FIFO, priority 99
```

### Application-Level Jitter Reduction

```cpp
#include <sched.h>
#include <sys/mman.h>
#include <pthread.h>

// Set up low-latency environment
void setup_low_latency() {
    // 1. Lock memory (prevent swapping)
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        perror("mlockall");
    }

    // 2. Set real-time scheduling
    struct sched_param param;
    param.sched_priority = 99;  // Highest priority
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        perror("sched_setscheduler");
    }

    // 3. Pin to specific CPU
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(2, &cpuset);  // Pin to core 2
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
    }
}

// Pre-fault memory to avoid page faults
void prefault_memory(void* addr, size_t size) {
    volatile char* ptr = static_cast<char*>(addr);
    for (size_t i = 0; i < size; i += 4096) {
        ptr[i] = 0;  // Touch each page
    }
}

// Usage
int main() {
    setup_low_latency();

    // Pre-allocate and prefault all memory
    const size_t BUFFER_SIZE = 1024 * 1024;  // 1 MB
    void* buffer = malloc(BUFFER_SIZE);
    prefault_memory(buffer, BUFFER_SIZE);

    // Your low-latency code here
    // No page faults, no context switches!

    return 0;
}
```

---

## Hardware Timestamping

### Software vs Hardware Timestamping

```
Software Timestamping:
Application → Kernel → NIC Driver → NIC → Wire
    ↑ timestamp here
Latency variance: ±100 μs (jitter from kernel)


Hardware Timestamping:
Application → Kernel → NIC Driver → NIC → Wire
                                      ↑ timestamp here
Latency variance: ±10 ns (PTP clock on NIC)

100x more accurate!
```

### Hardware Timestamp Implementation

```cpp
#include <linux/net_tstamp.h>
#include <linux/errqueue.h>

// Enable hardware TX timestamping
void enable_hw_timestamps(int sockfd) {
    int flags = SOF_TIMESTAMPING_TX_HARDWARE |
                SOF_TIMESTAMPING_RX_HARDWARE |
                SOF_TIMESTAMPING_RAW_HARDWARE;

    if (setsockopt(sockfd, SOL_SOCKET, SO_TIMESTAMPING,
                   &flags, sizeof(flags)) < 0) {
        perror("setsockopt SO_TIMESTAMPING");
    }
}

// Get TX timestamp
struct timespec get_tx_timestamp(int sockfd) {
    struct msghdr msg = {};
    struct iovec iov = {};
    char control[512];

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);

    // Receive from error queue
    if (recvmsg(sockfd, &msg, MSG_ERRQUEUE) < 0) {
        perror("recvmsg MSG_ERRQUEUE");
        return {};
    }

    // Parse timestamp
    for (struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
         cmsg != nullptr;
         cmsg = CMSG_NXTHDR(&msg, cmsg)) {

        if (cmsg->cmsg_level == SOL_SOCKET &&
            cmsg->cmsg_type == SCM_TIMESTAMPING) {

            struct timespec* ts = (struct timespec*)CMSG_DATA(cmsg);
            return ts[2];  // Hardware timestamp
        }
    }

    return {};
}

// Usage
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
enable_hw_timestamps(sockfd);

// Send packet
char data[] = "test";
send(sockfd, data, sizeof(data), 0);

// Get hardware timestamp
struct timespec tx_ts = get_tx_timestamp(sockfd);
printf("TX timestamp: %ld.%09ld\n", tx_ts.tv_sec, tx_ts.tv_nsec);

/*
 * Requires:
 * - Hardware support (Intel I350, X710, etc.)
 * - PTP-capable NIC
 * - Kernel support
 *
 * Benefits:
 * - Nanosecond accuracy
 * - No kernel jitter
 * - IEEE 1588 PTP synchronization
 */
```

---

## Real-World Case Studies

### Case Study 1: High-Frequency Trading

```
Requirements:
- Latency: <5 μs one-way
- Jitter: <100 ns
- Message rate: 1M/sec
- Reliability: 99.9999%

Architecture:
┌────────────────────────────────────┐
│ Market Data Feed (Multicast)       │
└────────┬───────────────────────────┘
         │ DPDK (kernel bypass)
         ▼
┌────────────────────────────────────┐
│ Strategy Engine                    │
│ - Lock-free queues                 │
│ - CPU pinning (core 2)             │
│ - No syscalls in hot path          │
│ - Pre-allocated memory             │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Order Gateway                      │
│ - Binary protocol                  │
│ - RDMA to exchange                 │
│ - Hardware timestamps              │
└────────────────────────────────────┘

Optimizations Applied:
1. Kernel bypass (DPDK)              → -20 μs
2. Lock-free data structures         → -2 μs
3. CPU pinning + isolated cores      → -1 μs
4. Pre-allocated memory              → -500 ns
5. Custom binary protocol            → -300 ns
6. Hardware timestamping             → Better accuracy

Result: 3.5 μs average latency
```

### Case Study 2: Online Gaming

```
Requirements:
- Latency: <50 ms (playable)
- Latency: <20 ms (competitive)
- Players: 100-1000 per server
- Update rate: 60 Hz (16.6 ms)

Architecture:
┌────────────────────────────────────┐
│ Game Client                        │
│ - UDP (unreliable but fast)        │
│ - Client-side prediction           │
│ - Lag compensation                 │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Game Server                        │
│ - epoll for 1000+ connections      │
│ - State synchronization            │
│ - Anti-cheat                       │
│ - Snapshot interpolation           │
└────────────────────────────────────┘

Optimizations:
1. UDP instead of TCP                → Lower latency
2. Reliable UDP for critical data    → Selective reliability
3. Delta compression                 → Reduce bandwidth
4. Interest management               → Only relevant updates
5. GeoDNS routing                    → Nearest server

Protocols Used:
- ENet (reliable UDP)
- Protocol Buffers (serialization)
- Client-side prediction
```

### Case Study 3: Live Video Streaming

```
Requirements:
- Latency: <3 seconds (acceptable)
- Latency: <500 ms (low-latency)
- Latency: <100 ms (interactive)
- Viewers: 1M+ concurrent

Architecture (Low-Latency):
┌────────────────────────────────────┐
│ Encoder (OBS, FFmpeg)              │
│ - H.264/H.265                      │
│ - 1080p60                          │
└────────┬───────────────────────────┘
         │ RTMP/SRT
         ▼
┌────────────────────────────────────┐
│ Ingest Server                      │
│ - Receive stream                   │
│ - Transcode (1080p, 720p, 480p)    │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Edge Servers (CDN)                 │
│ - CloudFront, Akamai               │
│ - WebRTC / HLS / DASH              │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Players (Web, Mobile)              │
└────────────────────────────────────┘

Protocol Comparison:
HLS (HTTP Live Streaming):
- Latency: 6-30 seconds
- Compatibility: Excellent
- Scalability: Excellent

DASH (Dynamic Adaptive Streaming):
- Latency: 6-30 seconds
- Compatibility: Good
- Scalability: Excellent

WebRTC:
- Latency: <500 ms
- Compatibility: Good (browsers)
- Scalability: Moderate
- Use case: Interactive streaming

SRT (Secure Reliable Transport):
- Latency: <2 seconds
- Reliability: Excellent
- Use case: Live production
```

---

## Complete Low-Latency Application

### Example: Low-Latency Echo Server

```cpp
// See examples/low_latency_server.cpp

Key Features:
1. Kernel bypass (optional DPDK)
2. Lock-free queues
3. CPU pinning
4. Hardware timestamps
5. Zero-copy
6. Memory pools
7. Binary protocol
8. epoll edge-triggered

Measured Performance:
- Latency p50: 8 μs
- Latency p99: 15 μs
- Latency p99.9: 25 μs
- Throughput: 500K msg/sec (single core)
- Jitter: <2 μs
```

---

## Low-Latency Checklist

### System Configuration
- [ ] Isolate CPU cores
- [ ] Disable CPU frequency scaling
- [ ] Disable turbo boost
- [ ] Pin IRQs to specific cores
- [ ] Disable transparent huge pages
- [ ] Use huge pages for memory
- [ ] Disable swap
- [ ] Set real-time priority

### Network Configuration
- [ ] Enable enhanced networking (AWS ENA)
- [ ] Use jumbo frames (MTU 9000)
- [ ] Increase ring buffer sizes
- [ ] Enable RSS (Receive Side Scaling)
- [ ] Tune TCP parameters
- [ ] Use TCP_NODELAY
- [ ] Use TCP_QUICKACK
- [ ] Consider kernel bypass (DPDK)

### Application Design
- [ ] Pre-allocate memory
- [ ] Use memory pools
- [ ] Lock-free data structures
- [ ] Avoid dynamic allocation in hot path
- [ ] CPU pinning
- [ ] Cache-line alignment
- [ ] Avoid syscalls in critical path
- [ ] Use hardware timestamps

### Measurement
- [ ] Measure with RDTSC
- [ ] Track percentiles (p99, p99.9, p99.99)
- [ ] Monitor jitter
- [ ] Profile with perf
- [ ] Analyze with flame graphs

---

## Key Takeaways

1. **Every microsecond counts**
   - Budget latency carefully
   - Optimize critical path

2. **Avoid the kernel**
   - Syscalls are expensive (1 μs)
   - Consider kernel bypass

3. **Lock-free for low latency**
   - Locks add 1-10 μs
   - Atomic operations: 10-100 ns

4. **Cache is king**
   - L1: 1 ns, RAM: 100 ns
   - Cache misses are expensive

5. **Reduce jitter**
   - Isolate cores
   - Real-time priority
   - Lock memory

6. **Measure everything**
   - p99.9 and p99.99 matter
   - Tail latency kills performance

7. **Hardware helps**
   - Hardware timestamps
   - Enhanced networking
   - RDMA

---

## Final Project

Build a complete low-latency market data handler:

1. **Requirements**
   - Receive multicast market data
   - Process 1M messages/sec
   - Latency <10 μs (p99)
   - Generate orderbook

2. **Technologies**
   - DPDK or kernel bypass
   - Lock-free queues
   - Binary protocol
   - Hardware timestamps

3. **Deliverables**
   - Source code
   - Performance measurements
   - Architecture document
   - Latency analysis

---

## Congratulations!

You've completed the Network Programming Course!

You now have the knowledge to build:
- High-performance network applications
- Low-latency trading systems
- Scalable cloud services
- Real-time gaming servers
- Distributed systems

**Next Steps:**
- Build your own project
- Contribute to open source
- Profile and optimize
- Keep learning!

## Additional Resources

- "Systems Performance" by Brendan Gregg
- "Linux Kernel Development" by Robert Love
- "C++ Concurrency in Action" by Anthony Williams
- DPDK Documentation: https://doc.dpdk.org/
- Linux kernel networking documentation
- Intel optimization manual

---

**Thank you for taking this course!**

For questions and discussions, please open an issue on GitHub.

Happy coding! 🚀

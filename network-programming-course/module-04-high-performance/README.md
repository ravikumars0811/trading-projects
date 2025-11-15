# Module 4: High-Performance Networking Techniques

## Overview

This module covers advanced high-performance networking techniques including zero-copy, kernel bypass, RDMA, and hardware optimizations for ultra-low latency applications.

## Table of Contents

1. [Zero-Copy Techniques](#zero-copy-techniques)
2. [Memory Management](#memory-management)
3. [CPU Affinity and NUMA](#cpu-affinity-and-numa)
4. [Kernel Bypass](#kernel-bypass)
5. [DPDK (Data Plane Development Kit)](#dpdk)
6. [RDMA (Remote Direct Memory Access)](#rdma)
7. [Network Stack Optimization](#network-stack-optimization)

---

## Zero-Copy Techniques

### Traditional Copy Path

```
User Application
     │
     │ read()
     ▼
┌─────────────────┐
│  User Buffer    │  ← Copy #1 (kernel → user)
└─────────────────┘
     │
     │ write()/send()
     ▼
┌─────────────────┐
│ Kernel Buffer   │  ← Copy #2 (user → kernel)
└─────────────────┘
     │
     │ DMA
     ▼
┌─────────────────┐
│  NIC Buffer     │  ← Copy #3 (kernel → NIC)
└─────────────────┘

Total: 3 copies, 2 context switches
Each copy: ~100-500ns per KB
```

### Zero-Copy with sendfile()

```cpp
#include <sys/sendfile.h>

// Send file to socket without userspace copy
int filefd = open("data.bin", O_RDONLY);
int sockfd = /* ... */;

off_t offset = 0;
size_t count = file_size;

// Kernel directly transfers data from file cache to socket
ssize_t sent = sendfile(sockfd, filefd, &offset, count);

/*
 * Data path:
 * File → Kernel Buffer → NIC
 * (only 1 copy via DMA!)
 *
 * Performance gain: 2-4x faster than read+write
 */
```

### Zero-Copy with splice()

```cpp
#include <fcntl.h>

// Transfer data between file descriptors via kernel pipe
// No userspace copying!

int pipe_fd[2];
pipe(pipe_fd);

// File → Pipe
ssize_t bytes = splice(file_fd, nullptr,
                       pipe_fd[1], nullptr,
                       length,
                       SPLICE_F_MOVE);

// Pipe → Socket
splice(pipe_fd[0], nullptr,
       socket_fd, nullptr,
       bytes,
       SPLICE_F_MOVE);

/*
 * Benefits:
 * - Zero copy to userspace
 * - Can chain multiple operations
 * - Works with any file descriptor
 */
```

### Zero-Copy with MSG_ZEROCOPY (Linux 4.14+)

```cpp
#include <linux/errqueue.h>

// Enable zero-copy transmission
int opt = 1;
setsockopt(sockfd, SOL_SOCKET, SO_ZEROCOPY, &opt, sizeof(opt));

// Send with zero-copy flag
char buffer[8192];
ssize_t sent = send(sockfd, buffer, sizeof(buffer), MSG_ZEROCOPY);

/*
 * IMPORTANT: Buffer must remain valid until kernel confirms transmission!
 */

// Receive completion notification
struct msghdr msg = {};
struct cmsghdr *cmsg;
char control[128];

msg.msg_control = control;
msg.msg_controllen = sizeof(control);

recvmsg(sockfd, &msg, MSG_ERRQUEUE); // Get completion

// Now safe to reuse/free buffer

/*
 * Performance:
 * - Reduces CPU usage by 30-50%
 * - Best for large messages (>10KB)
 * - Small messages: overhead not worth it
 */
```

### mmap() for File I/O

```cpp
#include <sys/mman.h>

// Map file directly into memory
int fd = open("data.bin", O_RDONLY);
struct stat sb;
fstat(fd, &sb);

void *mapped = mmap(nullptr, sb.st_size,
                    PROT_READ, MAP_PRIVATE,
                    fd, 0);

// Access file data without read() calls
char *data = static_cast<char*>(mapped);
// Use data...

// Send mapped memory
send(sockfd, data, sb.st_size, 0);

munmap(mapped, sb.st_size);
close(fd);

/*
 * Benefits:
 * - Lazy loading (demand paging)
 * - Shared among processes
 * - Kernel manages memory
 *
 * Drawbacks:
 * - Page faults on first access
 * - Not suitable for real-time
 */
```

---

## Memory Management

### Huge Pages

Huge pages reduce TLB (Translation Lookaside Buffer) misses for better performance.

```cpp
#include <sys/mman.h>

// Allocate 2MB huge page
void *buffer = mmap(nullptr, 2 * 1024 * 1024,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                    -1, 0);

if (buffer == MAP_FAILED) {
    perror("mmap huge page");
}

/*
 * Benefits:
 * - Fewer TLB misses (1 entry for 2MB vs 512 entries for 4KB pages)
 * - Better cache performance
 * - Reduced page table overhead
 *
 * Setup (requires root):
 * echo 128 > /proc/sys/vm/nr_hugepages
 */
```

### Memory Pools

Pre-allocate buffers to avoid runtime allocation overhead.

```cpp
#include <vector>
#include <memory>
#include <array>

template<typename T, size_t PoolSize>
class MemoryPool {
private:
    std::array<T, PoolSize> pool;
    std::vector<T*> free_list;

public:
    MemoryPool() {
        for (auto& item : pool) {
            free_list.push_back(&item);
        }
    }

    T* allocate() {
        if (free_list.empty()) {
            return nullptr;
        }
        T* ptr = free_list.back();
        free_list.pop_back();
        return ptr;
    }

    void deallocate(T* ptr) {
        free_list.push_back(ptr);
    }
};

// Usage for network buffers
struct PacketBuffer {
    char data[8192];
    size_t length;
};

MemoryPool<PacketBuffer, 10000> buffer_pool;

// O(1) allocation, no syscall
PacketBuffer* buf = buffer_pool.allocate();
// ... use buffer ...
buffer_pool.deallocate(buf);

/*
 * Performance benefits:
 * - No malloc/free overhead
 * - Better cache locality
 * - Predictable latency
 * - No memory fragmentation
 */
```

### NUMA-Aware Allocation

```cpp
#include <numa.h>

// Check NUMA availability
if (numa_available() < 0) {
    std::cerr << "NUMA not available\n";
}

// Get number of NUMA nodes
int num_nodes = numa_num_configured_nodes();

// Allocate on specific NUMA node
int node = 0;
void *buffer = numa_alloc_onnode(1024 * 1024, node);

// Bind thread to NUMA node
numa_run_on_node(node);

numa_free(buffer, 1024 * 1024);

/*
 * Why NUMA matters:
 * - Remote memory access: ~100-300ns
 * - Local memory access: ~60ns
 * - 2-5x performance difference!
 *
 * Best practice:
 * - Allocate on same node as processing thread
 * - Pin threads to cores on specific node
 */
```

---

## CPU Affinity and NUMA

### CPU Affinity

```cpp
#include <pthread.h>
#include <sched.h>

// Pin thread to specific CPU core
void set_cpu_affinity(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t thread = pthread_self();
    int rc = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

    if (rc != 0) {
        std::cerr << "Error setting CPU affinity: " << rc << "\n";
    } else {
        std::cout << "Thread pinned to core " << core_id << "\n";
    }
}

// Usage
int main() {
    set_cpu_affinity(2); // Pin to core 2

    // Your high-performance network code here
    // Benefits:
    // - Better cache locality
    // - No context switch overhead
    // - Predictable performance
}
```

### Isolating CPU Cores

```bash
# Boot parameter (add to grub config)
isolcpus=2,3,4,5

# Or dynamically with cgroups
echo 2-5 > /sys/fs/cgroup/cpuset/isolated/cpuset.cpus

# Move other processes away
echo 0,1 > /sys/fs/cgroup/cpuset/system/cpuset.cpus
```

### Disabling CPU Frequency Scaling

```bash
# Set to performance mode (max frequency always)
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    echo performance > $cpu
done

# Disable turbo boost (for consistent latency)
echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo
```

### NUMA Topology Visualization

```
NUMA Node 0             NUMA Node 1
┌─────────────────┐     ┌─────────────────┐
│  CPU 0-7        │     │  CPU 8-15       │
│  Memory 32GB    │     │  Memory 32GB    │
│  NIC 0          │     │  NIC 1          │
└────────┬────────┘     └────────┬────────┘
         │                       │
         └───────────┬───────────┘
                     │
               Interconnect
              (QPI/UPI)

Access latencies:
- Local memory:  60ns
- Remote memory: 120-300ns

Best practice:
- Bind NIC IRQ to local CPUs
- Allocate memory on same node
- Pin application threads to same node
```

---

## Kernel Bypass

### Why Bypass the Kernel?

```
Traditional Network Path:
┌──────────────┐
│ Application  │
└──────┬───────┘
       │ syscall (1-2 μs)
┌──────▼───────┐
│ Kernel       │
│ - TCP/IP     │
│ - Netfilter  │
│ - Queueing   │
└──────┬───────┘
       │ interrupt
┌──────▼───────┐
│ NIC Driver   │
└──────┬───────┘
       │
┌──────▼───────┐
│ Hardware     │
└──────────────┘

Latency: 10-50 μs


Kernel Bypass Path:
┌──────────────┐
│ Application  │
│ (user space) │
└──────┬───────┘
       │ direct (100-500 ns)
┌──────▼───────┐
│ User-space   │
│ Driver       │
└──────┬───────┘
       │ DMA
┌──────▼───────┐
│ Hardware     │
└──────────────┘

Latency: 1-5 μs (10x faster!)
```

### Kernel Bypass Techniques

1. **Memory-mapped I/O**
2. **Polling instead of interrupts**
3. **Dedicated CPU cores**
4. **Userspace network stack**

---

## DPDK (Data Plane Development Kit)

### What is DPDK?

DPDK is a framework for fast packet processing, bypassing the kernel entirely.

**Key Features:**
- Kernel bypass via UIO/VFIO
- Poll Mode Drivers (PMD) - no interrupts!
- Zero-copy packet handling
- Multi-core scaling
- Huge page memory

### DPDK Architecture

```
┌─────────────────────────────────────────┐
│         Application                     │
│  ┌──────────┐    ┌──────────┐          │
│  │ Thread 0 │    │ Thread 1 │          │
│  └────┬─────┘    └────┬─────┘          │
│       │               │                 │
│  ┌────▼───────────────▼─────┐          │
│  │    DPDK Libraries         │          │
│  │  - rte_eal (Environment)  │          │
│  │  - rte_mbuf (Buffers)     │          │
│  │  - rte_ring (Queues)      │          │
│  │  - rte_mempool (Memory)   │          │
│  └────┬──────────────────────┘          │
│       │                                  │
│  ┌────▼──────────────────────┐          │
│  │   Poll Mode Drivers (PMD) │          │
│  │   - No interrupts          │          │
│  │   - Continuous polling     │          │
│  └────┬──────────────────────┘          │
└───────┼──────────────────────────────────┘
        │ UIO/VFIO (kernel bypass)
┌───────▼──────────────────────────────────┐
│         NIC Hardware                     │
│  ┌─────────┐      ┌─────────┐           │
│  │ RX Queue│      │ TX Queue│           │
│  └─────────┘      └─────────┘           │
└──────────────────────────────────────────┘
```

### Basic DPDK Example

```cpp
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_cycles.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250

static struct rte_mempool *mbuf_pool;

// Initialize DPDK
int main(int argc, char *argv[]) {
    // Initialize EAL (Environment Abstraction Layer)
    int ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");
    }

    // Create mbuf pool
    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL",
                                        NUM_MBUFS,
                                        MBUF_CACHE_SIZE,
                                        0,
                                        RTE_MBUF_DEFAULT_BUF_SIZE,
                                        rte_socket_id());

    // Configure port
    uint16_t port_id = 0;
    struct rte_eth_conf port_conf = {};

    ret = rte_eth_dev_configure(port_id, 1, 1, &port_conf);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Cannot configure port\n");
    }

    // Setup RX queue
    ret = rte_eth_rx_queue_setup(port_id, 0, RX_RING_SIZE,
                                  rte_eth_dev_socket_id(port_id),
                                  nullptr, mbuf_pool);

    // Setup TX queue
    ret = rte_eth_tx_queue_setup(port_id, 0, TX_RING_SIZE,
                                  rte_eth_dev_socket_id(port_id),
                                  nullptr);

    // Start device
    ret = rte_eth_dev_start(port_id);

    printf("DPDK initialized, entering main loop...\n");

    // Main packet processing loop
    struct rte_mbuf *bufs[32];
    while (1) {
        // Receive packets (polling!)
        uint16_t nb_rx = rte_eth_rx_burst(port_id, 0, bufs, 32);

        if (unlikely(nb_rx == 0)) {
            continue;
        }

        // Process packets
        for (uint16_t i = 0; i < nb_rx; i++) {
            struct rte_mbuf *m = bufs[i];

            // Access packet data (zero-copy!)
            char *data = rte_pktmbuf_mtod(m, char*);
            uint16_t len = rte_pktmbuf_data_len(m);

            // Process packet...

            // Swap MAC addresses for echo
            uint8_t tmp[6];
            memcpy(tmp, data, 6);
            memcpy(data, data + 6, 6);
            memcpy(data + 6, tmp, 6);
        }

        // Send packets back (zero-copy!)
        uint16_t nb_tx = rte_eth_tx_burst(port_id, 0, bufs, nb_rx);

        // Free unsent packets
        for (uint16_t i = nb_tx; i < nb_rx; i++) {
            rte_pktmbuf_free(bufs[i]);
        }
    }

    return 0;
}

/*
 * Performance:
 * - 10+ Gbps on single core
 * - Sub-microsecond latency
 * - Line-rate 40/100 Gbps possible
 *
 * Compile:
 * gcc -O3 dpdk_example.c -o dpdk_app \
 *     -I/usr/include/dpdk \
 *     -lrte_eal -lrte_ethdev -lrte_mbuf
 *
 * Run:
 * ./dpdk_app -l 0-3 -n 4 -- -p 0x1
 */
```

### DPDK Performance

```
Benchmark: 64-byte packets

Standard Linux kernel:
- ~1-2 Mpps (million packets/sec)
- CPU: 100% on 8 cores
- Latency: 20-50 μs

DPDK:
- ~14 Mpps on single core (10 Gbps line rate)
- ~60 Mpps on 4 cores (40 Gbps)
- Latency: 1-5 μs
- 10-20x improvement!
```

---

## RDMA (Remote Direct Memory Access)

### What is RDMA?

RDMA allows direct memory access between machines without CPU involvement.

```
Traditional Network:
CPU ─→ Memory ─→ NIC ─→ Network ─→ NIC ─→ Memory ─→ CPU
(CPU overhead for every packet)

RDMA:
Memory ←─ DMA ─→ NIC ─→ Network ─→ NIC ←─ DMA ─→ Memory
(Zero CPU overhead!)
```

### RDMA Benefits

- **Zero-copy**: Direct memory-to-memory transfer
- **Kernel bypass**: No OS involvement
- **CPU offload**: NIC handles protocol
- **Low latency**: <1 μs typical
- **High throughput**: 100+ Gbps

### RDMA Verbs API Example

```cpp
#include <infiniband/verbs.h>

// Open RDMA device
struct ibv_device **dev_list = ibv_get_device_list(nullptr);
struct ibv_context *ctx = ibv_open_device(dev_list[0]);

// Allocate protection domain
struct ibv_pd *pd = ibv_alloc_pd(ctx);

// Register memory region
char *buffer = new char[4096];
struct ibv_mr *mr = ibv_reg_mr(pd, buffer, 4096,
                               IBV_ACCESS_LOCAL_WRITE |
                               IBV_ACCESS_REMOTE_WRITE |
                               IBV_ACCESS_REMOTE_READ);

// Create completion queue
struct ibv_cq *cq = ibv_create_cq(ctx, 10, nullptr, nullptr, 0);

// Create queue pair
struct ibv_qp_init_attr qp_attr = {};
qp_attr.send_cq = cq;
qp_attr.recv_cq = cq;
qp_attr.qp_type = IBV_QPT_RC; // Reliable connection
qp_attr.cap.max_send_wr = 10;
qp_attr.cap.max_recv_wr = 10;
qp_attr.cap.max_send_sge = 1;
qp_attr.cap.max_recv_sge = 1;

struct ibv_qp *qp = ibv_create_qp(pd, &qp_attr);

// RDMA Write (one-sided operation)
struct ibv_sge sge = {};
sge.addr = (uintptr_t)buffer;
sge.length = 4096;
sge.lkey = mr->lkey;

struct ibv_send_wr wr = {};
wr.wr_id = 1;
wr.sg_list = &sge;
wr.num_sge = 1;
wr.opcode = IBV_WR_RDMA_WRITE;
wr.send_flags = IBV_SEND_SIGNALED;
wr.wr.rdma.remote_addr = remote_addr; // From peer
wr.wr.rdma.rkey = remote_key;         // From peer

struct ibv_send_wr *bad_wr;
ibv_post_send(qp, &wr, &bad_wr);

// Poll for completion
struct ibv_wc wc;
while (ibv_poll_cq(cq, 1, &wc) == 0);

printf("RDMA Write completed!\n");

/*
 * Performance:
 * - Latency: 0.5-2 μs (vs 10-20 μs TCP)
 * - CPU usage: Near zero
 * - Throughput: 100+ Gbps
 *
 * Use cases:
 * - High-frequency trading
 * - Distributed storage
 * - HPC applications
 * - Database replication
 */
```

### RDMA vs TCP Comparison

| Feature | TCP | RDMA |
|---------|-----|------|
| Latency | 10-20 μs | 0.5-2 μs |
| CPU Usage | High | Near zero |
| Throughput | 10-40 Gbps | 100+ Gbps |
| Reliability | Yes | Yes |
| Ordering | Yes | Yes |
| Kernel Bypass | No | Yes |
| Hardware | Standard NIC | RDMA-capable NIC |
| Cost | Low | Medium-High |

---

## Network Stack Optimization

### Interrupt Coalescing

```bash
# Reduce interrupt rate (trade latency for throughput)
ethtool -C eth0 rx-usecs 50 rx-frames 32

# For low latency (more interrupts)
ethtool -C eth0 rx-usecs 0 rx-frames 1

# Check current settings
ethtool -c eth0
```

### Ring Buffer Size

```bash
# Increase ring buffer to reduce packet drops
ethtool -G eth0 rx 4096 tx 4096

# Check current size
ethtool -g eth0
```

### RSS (Receive Side Scaling)

```bash
# Distribute incoming packets across multiple CPU cores
ethtool -X eth0 equal 4  # 4 queues

# Set custom hash key
ethtool -X eth0 hkey <key>

# Check RSS settings
ethtool -x eth0
```

### IRQ Affinity

```bash
# Pin NIC IRQs to specific CPUs
echo 2 > /proc/irq/45/smp_affinity_list  # Pin IRQ 45 to core 2

# Find NIC IRQs
grep eth0 /proc/interrupts
```

### Kernel Parameters

```bash
# Increase network buffers
sysctl -w net.core.rmem_max=134217728     # 128MB
sysctl -w net.core.wmem_max=134217728
sysctl -w net.core.rmem_default=33554432  # 32MB
sysctl -w net.core.wmem_default=33554432

# Increase connection backlog
sysctl -w net.core.somaxconn=4096
sysctl -w net.ipv4.tcp_max_syn_backlog=4096

# TCP tuning for high-throughput
sysctl -w net.ipv4.tcp_rmem='4096 87380 134217728'
sysctl -w net.ipv4.tcp_wmem='4096 65536 134217728'

# Enable TCP window scaling
sysctl -w net.ipv4.tcp_window_scaling=1

# Reduce TIME_WAIT recycling time
sysctl -w net.ipv4.tcp_fin_timeout=15

# Enable TCP timestamps
sysctl -w net.ipv4.tcp_timestamps=1
```

---

## Performance Monitoring

### Measuring Latency

```cpp
#include <chrono>
#include <vector>
#include <algorithm>

class LatencyMeasurement {
private:
    std::vector<uint64_t> latencies;

public:
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;

    time_point start_measurement() {
        return clock::now();
    }

    void record(time_point start) {
        auto end = clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start).count();
        latencies.push_back(ns);
    }

    void print_percentiles() {
        std::sort(latencies.begin(), latencies.end());
        size_t n = latencies.size();

        std::cout << "Latency percentiles (μs):\n";
        std::cout << "  p50:  " << latencies[n * 50 / 100] / 1000.0 << "\n";
        std::cout << "  p90:  " << latencies[n * 90 / 100] / 1000.0 << "\n";
        std::cout << "  p95:  " << latencies[n * 95 / 100] / 1000.0 << "\n";
        std::cout << "  p99:  " << latencies[n * 99 / 100] / 1000.0 << "\n";
        std::cout << "  p99.9:" << latencies[n * 999 / 1000] / 1000.0 << "\n";
        std::cout << "  max:  " << latencies.back() / 1000.0 << "\n";
    }
};

// Usage
LatencyMeasurement lat;
for (int i = 0; i < 100000; i++) {
    auto start = lat.start_measurement();
    // ... network operation ...
    lat.record(start);
}
lat.print_percentiles();
```

---

## Key Takeaways

1. **Zero-copy eliminates memory copies**
   - Use sendfile(), splice(), MSG_ZEROCOPY
   - Critical for high-throughput

2. **Memory pools prevent allocation overhead**
   - Pre-allocate buffers
   - O(1) allocation/deallocation

3. **CPU affinity improves cache locality**
   - Pin threads to specific cores
   - Isolate cores for critical paths

4. **NUMA awareness is critical**
   - Allocate on local node
   - 2-5x performance difference

5. **Kernel bypass for ultimate performance**
   - DPDK: 10-20x improvement
   - RDMA: <1 μs latency possible

6. **Tune the network stack**
   - Ring buffers, IRQ affinity
   - RSS for multi-core scaling

7. **Measure everything**
   - p50, p95, p99, p99.9 percentiles
   - Don't rely on averages

---

## Next Steps

Continue to [Module 5: Network Protocols and Optimization](../module-05-protocols/README.md) for protocol-specific optimizations.

## Additional Resources

- DPDK Documentation: https://doc.dpdk.org/
- RDMA Programming Guide: https://www.rdmamojo.com/
- "Systems Performance" by Brendan Gregg
- Linux kernel documentation on networking

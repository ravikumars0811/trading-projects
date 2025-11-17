# HFT Performance Profiling & Debugging Guide

## Quick Reference Commands

### Instant Performance Snapshot
```bash
# Quick CPU and latency check
perf stat -e cycles,instructions,cache-references,cache-misses,branch-misses ./hft_server

# Real-time latency monitoring
cyclictest -p 99 -t1 -n -i 1000 -l 100000

# Check for context switches (should be minimal)
pidstat -w -p $(pgrep hft_server) 1
```

---

## 1. CPU Profiling

### 1.1 Linux perf - The Swiss Army Knife

**Install:**
```bash
sudo apt-get install linux-tools-common linux-tools-generic linux-tools-$(uname -r)
```

**Basic CPU Profiling:**
```bash
# Record performance data
perf record -g --call-graph dwarf -F 999 ./hft_server

# Analyze results interactively
perf report

# Generate flamegraph (install flamegraph tools first)
perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg

# Find hotspots with annotation
perf annotate
```

**Cache Performance Analysis:**
```bash
# Detailed cache statistics
perf stat -e cache-references,cache-misses,L1-dcache-loads,L1-dcache-load-misses,\
L1-icache-load-misses,LLC-loads,LLC-load-misses,dTLB-loads,dTLB-load-misses \
./hft_server

# Example output interpretation:
#   L1-dcache-load-misses < 5% is excellent
#   LLC-load-misses < 1% is excellent
#   dTLB-load-misses < 0.1% is excellent
```

**Branch Prediction Analysis:**
```bash
# Check branch prediction efficiency
perf stat -e branches,branch-misses ./hft_server

# Branch miss rate should be < 2% for optimal performance
```

**CPU Cycle Analysis:**
```bash
# Instructions per cycle (IPC) - higher is better (>2.0 is good)
perf stat -e cycles,instructions ./hft_server

# Calculate IPC: instructions / cycles
```

### 1.2 Intel VTune Profiler

**Installation:**
```bash
# Download from Intel website
wget https://registrationcenter-download.intel.com/...
tar -xzf vtune_profiler.tar.gz
cd vtune_profiler
./install.sh
```

**Hotspot Analysis:**
```bash
vtune -collect hotspots -result-dir vtune_hotspots ./hft_server
vtune-gui vtune_hotspots  # Open GUI
```

**Microarchitecture Analysis:**
```bash
# Detailed CPU pipeline analysis
vtune -collect uarch-exploration -result-dir vtune_uarch ./hft_server

# Memory access patterns
vtune -collect memory-access -result-dir vtune_memory ./hft_server
```

**Key Metrics to Monitor:**
- **CPI (Cycles Per Instruction)**: Should be < 0.5 for HFT
- **Frontend Bound**: < 10% (instruction fetch bottleneck)
- **Backend Bound**: < 30% (execution bottleneck)
- **Memory Bound**: < 20% (memory access bottleneck)

### 1.3 Valgrind Callgrind

**Cache Profiling:**
```bash
# Detailed cache simulation
valgrind --tool=cachegrind --cache-sim=yes --branch-sim=yes ./hft_server

# Annotate source code with cache misses
cg_annotate cachegrind.out.12345 --auto=yes

# Visualize with KCachegrind
kcachegrind cachegrind.out.12345
```

---

## 2. Latency Measurement

### 2.1 Real-Time Latency Testing

**Cyclictest (Kernel Latency):**
```bash
# Install
sudo apt-get install rt-tests

# Test latency (run for 1 hour)
sudo cyclictest -p 99 -t1 -n -i 1000 -l 3600000 -h 100 -q

# Output interpretation:
#   Min: ~1-3 μs (good)
#   Avg: ~2-5 μs (good)
#   Max: <50 μs (excellent), <100 μs (good), >1000 μs (bad - needs tuning)
```

**Latency Histogram:**
```bash
# Generate histogram
sudo cyclictest -p 99 -t1 -n -i 1000 -l 100000 -h 200 > latency.txt

# Plot histogram (requires gnuplot)
grep -v "#" latency.txt | tr " " "," | gnuplot -e "
set terminal png size 1024,768;
set output 'latency_histogram.png';
set xlabel 'Latency (us)';
set ylabel 'Count';
plot '-' using 1:2 with lines title 'Latency Distribution'
"
```

### 2.2 Application-Level Latency Tracking

**Using TSC (Time Stamp Counter):**

**C++ Implementation:**
```cpp
#include <x86intrin.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

class LatencyTracker {
private:
    std::vector<uint64_t> latencies_;
    double tsc_freq_ghz_;

public:
    LatencyTracker(size_t capacity = 1000000) {
        latencies_.reserve(capacity);
        calibrateTSC();
    }

    void calibrateTSC() {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t tsc_start = __rdtsc();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        uint64_t tsc_end = __rdtsc();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start).count();

        tsc_freq_ghz_ = static_cast<double>(tsc_end - tsc_start) / duration_ns;
    }

    uint64_t now() {
        _mm_lfence();
        uint64_t tsc = __rdtsc();
        _mm_lfence();
        return tsc;
    }

    void recordLatency(uint64_t start_tsc, uint64_t end_tsc) {
        latencies_.push_back(end_tsc - start_tsc);
    }

    void printStatistics() {
        if (latencies_.empty()) return;

        std::sort(latencies_.begin(), latencies_.end());

        auto toNanos = [this](uint64_t tsc) {
            return static_cast<uint64_t>(tsc / tsc_freq_ghz_);
        };

        std::cout << "Latency Statistics (nanoseconds):" << std::endl;
        std::cout << "  Count: " << latencies_.size() << std::endl;
        std::cout << "  Min:   " << toNanos(latencies_.front()) << " ns" << std::endl;
        std::cout << "  Max:   " << toNanos(latencies_.back()) << " ns" << std::endl;

        size_t n = latencies_.size();
        std::cout << "  Mean:  " << toNanos(std::accumulate(latencies_.begin(),
            latencies_.end(), 0ULL) / n) << " ns" << std::endl;
        std::cout << "  P50:   " << toNanos(latencies_[n / 2]) << " ns" << std::endl;
        std::cout << "  P95:   " << toNanos(latencies_[n * 95 / 100]) << " ns" << std::endl;
        std::cout << "  P99:   " << toNanos(latencies_[n * 99 / 100]) << " ns" << std::endl;
        std::cout << "  P99.9: " << toNanos(latencies_[n * 999 / 1000]) << " ns" << std::endl;
    }

    void exportToFile(const std::string& filename) {
        std::ofstream out(filename);
        for (uint64_t lat : latencies_) {
            out << (lat / tsc_freq_ghz_) << "\n";
        }
    }
};

// Usage in order processing
LatencyTracker tracker;

void processOrder(Order& order) {
    uint64_t start = tracker.now();

    // Process order
    orderBook.addOrder(order);

    uint64_t end = tracker.now();
    tracker.recordLatency(start, end);
}

// After processing, print stats
tracker.printStatistics();
tracker.exportToFile("order_latencies.txt");
```

**Python Implementation (using ctypes):**
```python
import ctypes
import numpy as np
import time

# Load libc for clock_gettime
libc = ctypes.CDLL('libc.so.6', use_errno=True)

class Timespec(ctypes.Structure):
    _fields_ = [('tv_sec', ctypes.c_long), ('tv_nsec', ctypes.c_long)]

CLOCK_MONOTONIC_RAW = 4

def get_time_ns():
    """Get nanosecond timestamp"""
    ts = Timespec()
    libc.clock_gettime(CLOCK_MONOTONIC_RAW, ctypes.byref(ts))
    return ts.tv_sec * 1_000_000_000 + ts.tv_nsec

class LatencyTracker:
    def __init__(self, capacity=1000000):
        self.latencies = []

    def record(self, start_ns, end_ns):
        self.latencies.append(end_ns - start_ns)

    def print_statistics(self):
        if not self.latencies:
            return

        latencies = np.array(self.latencies)

        print("Latency Statistics (nanoseconds):")
        print(f"  Count: {len(latencies)}")
        print(f"  Min:   {np.min(latencies):,} ns")
        print(f"  Max:   {np.max(latencies):,} ns")
        print(f"  Mean:  {np.mean(latencies):,.0f} ns")
        print(f"  P50:   {np.percentile(latencies, 50):,.0f} ns")
        print(f"  P95:   {np.percentile(latencies, 95):,.0f} ns")
        print(f"  P99:   {np.percentile(latencies, 99):,.0f} ns")
        print(f"  P99.9: {np.percentile(latencies, 99.9):,.0f} ns")

# Usage
tracker = LatencyTracker()

start = get_time_ns()
process_order(order)  # Your function
end = get_time_ns()

tracker.record(start, end)
```

---

## 3. Memory Profiling

### 3.1 Valgrind Massif (Heap Profiling)

```bash
# Track heap allocations over time
valgrind --tool=massif --time-unit=ms ./hft_server

# Analyze results
ms_print massif.out.12345

# Visualize with massif-visualizer
massif-visualizer massif.out.12345
```

**Key Metrics:**
- **Peak Heap Usage**: Should be stable (no leaks)
- **Allocations**: Minimize in hot path (use memory pools)

### 3.2 Heaptrack

```bash
# Install
sudo apt-get install heaptrack heaptrack-gui

# Profile heap usage
heaptrack ./hft_server

# Analyze
heaptrack-gui heaptrack.hft_server.12345.gz
```

### 3.3 Checking for Memory Leaks

```bash
# Run with Valgrind memcheck
valgrind --leak-check=full --show-leak-kinds=all ./hft_server

# For production (lower overhead)
valgrind --leak-check=summary ./hft_server
```

---

## 4. Network Profiling

### 4.1 tcpdump Analysis

```bash
# Capture market data feed
sudo tcpdump -i eth0 -w market_data.pcap port 9001

# Analyze with timing
tcpdump -r market_data.pcap -tttt

# Count packets per second
tcpdump -r market_data.pcap | awk '{print $1}' | uniq -c
```

### 4.2 iperf3 - Network Throughput

```bash
# Server side
iperf3 -s

# Client side (test bandwidth)
iperf3 -c <server_ip> -t 30

# Test latency
iperf3 -c <server_ip> --udp -b 1M -l 64
```

### 4.3 NIC Statistics

```bash
# Check for drops and errors
ethtool -S eth0 | grep -E 'drop|error|miss'

# Network interrupt statistics
cat /proc/interrupts | grep eth0

# Network buffer statistics
netstat -s | grep -E 'overflow|drop'
```

---

## 5. Disk I/O Profiling

### 5.1 iostat

```bash
# Install
sudo apt-get install sysstat

# Monitor I/O in real-time
iostat -xz 1

# Key metrics:
#   %util: Should be <80% for good performance
#   await: Average I/O latency (ms) - lower is better
#   r_await/w_await: Read/write latency
```

### 5.2 iotop

```bash
# Install
sudo apt-get install iotop

# Monitor per-process I/O
sudo iotop -o  # Only show processes doing I/O
```

### 5.3 Disk Benchmark

```bash
# Test sequential read
sudo hdparm -t /dev/nvme0n1

# Test random read/write with fio
sudo fio --name=random-read --ioengine=libaio --iodepth=32 --rw=randread \
         --bs=4k --direct=1 --size=1G --numjobs=4 --runtime=60 --group_reporting
```

---

## 6. System-Wide Profiling

### 6.1 htop - Enhanced Process Viewer

```bash
# Install
sudo apt-get install htop

# Run and press F2 to configure:
#   - Show CPU affinity
#   - Show CPU frequency
#   - Color by CPU usage
htop
```

### 6.2 atop - Advanced System Monitor

```bash
# Install
sudo apt-get install atop

# Real-time monitoring
sudo atop 1

# Record for later analysis
sudo atop -w /var/log/atop/atop_$(date +%Y%m%d) 60
```

### 6.3 sar - System Activity Reporter

```bash
# CPU usage
sar -u 1 10

# Memory usage
sar -r 1 10

# Network statistics
sar -n DEV 1 10

# Context switches
sar -w 1 10
```

---

## 7. Debugging Techniques

### 7.1 GDB for Live Debugging

```bash
# Attach to running process
sudo gdb -p $(pgrep hft_server)

# Set breakpoint
(gdb) break order_book.cpp:42

# Print variables
(gdb) print current_order

# Print backtrace
(gdb) bt

# Continue execution
(gdb) continue

# Detach without killing
(gdb) detach
```

### 7.2 strace - System Call Tracing

```bash
# Trace system calls
sudo strace -p $(pgrep hft_server)

# Count syscalls (find bottlenecks)
sudo strace -c -p $(pgrep hft_server)

# Trace specific syscalls
sudo strace -e trace=open,read,write -p $(pgrep hft_server)

# Trace with timestamps
sudo strace -tt -T -p $(pgrep hft_server)
```

**Optimization Goal**: Minimize syscalls in hot path
- Good: <100 syscalls/second
- Bad: >10,000 syscalls/second

### 7.3 ltrace - Library Call Tracing

```bash
# Trace library calls
sudo ltrace -c -p $(pgrep hft_server)

# Find malloc/free calls (should be minimal)
sudo ltrace -e malloc,free -p $(pgrep hft_server)
```

---

## 8. Continuous Monitoring

### 8.1 Prometheus + Grafana

**Export Metrics from C++:**
```cpp
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

class MetricsExporter {
private:
    std::shared_ptr<prometheus::Registry> registry_;
    prometheus::Exposer exposer_;

    prometheus::Family<prometheus::Counter>& order_counter_;
    prometheus::Family<prometheus::Histogram>& latency_histogram_;

public:
    MetricsExporter()
        : registry_(std::make_shared<prometheus::Registry>())
        , exposer_("0.0.0.0:8080")
        , order_counter_(prometheus::BuildCounter()
            .Name("orders_processed_total")
            .Help("Total number of orders processed")
            .Register(*registry_))
        , latency_histogram_(prometheus::BuildHistogram()
            .Name("order_latency_ns")
            .Help("Order processing latency in nanoseconds")
            .Register(*registry_)) {

        exposer_.RegisterCollectable(registry_);
    }

    void recordOrder() {
        order_counter_.Add({{"type", "limit"}}).Increment();
    }

    void recordLatency(uint64_t latency_ns) {
        latency_histogram_.Add({{"operation", "order_processing"}},
            {100, 500, 1000, 5000, 10000, 50000})
            .Observe(latency_ns);
    }
};
```

**Prometheus Configuration (prometheus.yml):**
```yaml
scrape_configs:
  - job_name: 'hft_server'
    static_configs:
      - targets: ['localhost:8080']
    scrape_interval: 1s
```

### 8.2 Custom Logging for Analysis

```cpp
class PerformanceLogger {
public:
    static void logOrderLatency(uint64_t order_id, uint64_t latency_ns,
                                const std::string& stage) {
        // CSV format for easy analysis
        std::cout << std::chrono::system_clock::now().time_since_epoch().count()
                  << "," << order_id
                  << "," << latency_ns
                  << "," << stage << std::endl;
    }
};

// Usage
PerformanceLogger::logOrderLatency(order_id, latency, "order_book_add");
```

**Analyze with awk:**
```bash
# Average latency per stage
cat performance.log | awk -F',' '{sum[$4]+=$3; count[$4]++} END {for(stage in sum) print stage, sum[stage]/count[stage]}'

# 95th percentile latency
cat performance.log | awk -F',' '{print $3}' | sort -n | awk 'BEGIN{c=0} {a[c++]=$1} END{print a[int(c*0.95)]}'
```

---

## 9. Performance Checklist

### Before Optimization
- [ ] Establish baseline metrics (latency, throughput, CPU usage)
- [ ] Profile to identify actual bottlenecks (don't guess!)
- [ ] Set measurable performance targets

### During Optimization
- [ ] Change one thing at a time
- [ ] Measure after each change
- [ ] Document what worked and what didn't

### Verification
- [ ] Run under realistic load
- [ ] Check 99th percentile, not just average
- [ ] Monitor for at least 1 hour
- [ ] Check for memory leaks
- [ ] Verify no excessive context switches
- [ ] Check CPU cache hit rates

---

## 10. Common Performance Issues and Solutions

| Issue | Symptom | Solution |
|-------|---------|----------|
| **High Latency Spikes** | P99 >> P50 | Check for: CPU throttling, context switches, page faults, GC pauses |
| **Low Throughput** | Messages/sec lower than expected | Profile for lock contention, check queue sizes, verify CPU utilization |
| **High CPU Usage** | CPU at 100% but low throughput | Look for: busy waiting, inefficient algorithms, cache misses |
| **Memory Growth** | RSS increasing over time | Check for: memory leaks (valgrind), unbounded queues, data structure bloat |
| **Network Drops** | Packets lost | Increase buffers, check NIC ring buffer, verify no CPU overload |
| **Disk I/O Wait** | High iowait% | Use faster storage, optimize I/O patterns, use memory mapping |

---

## Quick Profiling Workflow

```bash
# 1. Quick check
perf stat ./hft_server

# 2. Find hotspots
perf record -g ./hft_server
perf report

# 3. Cache analysis
perf stat -e cache-references,cache-misses ./hft_server

# 4. Latency measurement
./hft_server  # With built-in TSC timing

# 5. System monitoring
htop  # CPU/memory
iotop  # Disk I/O
iftop  # Network

# 6. Debug if needed
gdb -p $(pgrep hft_server)
strace -c -p $(pgrep hft_server)
```

---

## Tools Summary

| Category | Tool | Use Case |
|----------|------|----------|
| **CPU** | perf | Profiling, cache analysis |
| | VTune | Deep microarchitecture analysis |
| | gprof | Function-level profiling |
| **Latency** | cyclictest | Kernel latency testing |
| | TSC | Application-level nanosecond timing |
| **Memory** | Valgrind | Memory leaks, cache simulation |
| | Massif | Heap profiling |
| | Heaptrack | Memory allocation tracking |
| **Network** | tcpdump | Packet capture |
| | iperf3 | Bandwidth/latency testing |
| | ethtool | NIC statistics |
| **System** | htop | Process monitoring |
| | sar | Historical system stats |
| | atop | Comprehensive monitoring |
| **Debug** | gdb | Interactive debugging |
| | strace | System call tracing |
| | ltrace | Library call tracing |

---

**Remember**: "Premature optimization is the root of all evil" - Always profile first!

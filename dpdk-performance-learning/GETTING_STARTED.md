# Getting Started Guide

## Quick Start

### 1. Clone and Build

```bash
cd dpdk-performance-learning

# Option 1: Using Make
make all

# Option 2: Using CMake
mkdir build && cd build
cmake ..
make
cd ..
```

### 2. Run Your First Examples

```bash
# Performance Optimization
cd 02-performance-optimization/cache
./cache_friendly

# Memory Management
cd ../../04-memory-management/allocators
./arena_allocator

# CPU Optimization with SIMD
cd ../../05-cpu-optimization/simd
./simd_basics

# Distributed Systems
cd ../../06-distributed-systems/networking
./tcp_server_epoll 8080
# In another terminal: telnet localhost 8080
```

### 3. System Programming (requires NUMA)

```bash
# Install NUMA library first
sudo apt-get install libnuma-dev

# Build and run
cd 03-system-programming/threads
gcc -O2 -o thread_affinity thread_affinity.c -lpthread -lnuma
./thread_affinity
```

### 4. DPDK Setup (Advanced)

DPDK requires special setup. Follow these steps:

```bash
# See detailed instructions
cat 01-dpdk/docs/setup.md

# Quick DPDK install (Ubuntu)
sudo apt-get install -y dpdk dpdk-dev

# Or build from source
wget https://fast.dpdk.org/rel/dpdk-21.11.6.tar.xz
tar xf dpdk-21.11.6.tar.xz
cd dpdk-21.11.6
meson setup build
cd build
ninja
sudo ninja install

# Set environment
export RTE_SDK=/path/to/dpdk-21.11.6

# Setup huge pages
sudo sysctl -w vm.nr_hugepages=1024

# Build DPDK examples
cd dpdk-performance-learning
make dpdk

# Run (requires root)
cd 01-dpdk/examples
sudo ./basic_eal_init -l 0-3 -n 4
```

## Learning Path

### Week 1: Foundations
- Day 1-2: System Programming basics (03-system-programming)
- Day 3-4: Memory Management (04-memory-management/allocators)
- Day 5-7: Cache Optimization (02-performance-optimization/cache)

### Week 2: CPU Optimization
- Day 1-3: SIMD Programming (05-cpu-optimization/simd)
- Day 4-5: Profiling and Benchmarking (02-performance-optimization)
- Day 6-7: Practice exercises

### Week 3: Networking
- Day 1-3: High-Performance Networking (06-distributed-systems/networking)
- Day 4-5: Distributed Systems (06-distributed-systems)
- Day 6-7: Build a project combining concepts

### Week 4: DPDK Deep Dive
- Day 1-2: DPDK Setup and EAL (01-dpdk)
- Day 3-4: Memory Pools and Rings (01-dpdk/examples)
- Day 5-7: Build a packet processing application

## Recommended Reading Order

1. **Start Here**: Main README.md
2. **System Programming**: 03-system-programming/README.md
3. **Performance**: 02-performance-optimization/README.md
4. **Memory**: 04-memory-management/README.md
5. **CPU Opt**: 05-cpu-optimization/README.md
6. **Distributed**: 06-distributed-systems/README.md
7. **DPDK**: 01-dpdk/README.md

## Common Issues

### Issue: "cannot find -lnuma"
```bash
sudo apt-get install libnuma-dev
```

### Issue: "DPDK not found"
DPDK requires separate installation. See `01-dpdk/docs/setup.md`

### Issue: "Permission denied" for DPDK
DPDK apps need root or capabilities:
```bash
sudo ./basic_eal_init -l 0-3 -n 4
```

### Issue: "No huge pages available"
```bash
# Allocate huge pages
sudo sysctl -w vm.nr_hugepages=1024

# Make persistent
echo "vm.nr_hugepages=1024" | sudo tee -a /etc/sysctl.conf
```

### Issue: AVX/AVX2 not supported
Your CPU might not support AVX2. Build without:
```bash
gcc -O2 -msse4.2 simd_basics.c -o simd_basics -lm
```

## Performance Tuning Tips

### For Best Performance

1. **Disable CPU frequency scaling**
```bash
sudo cpupower frequency-set -g performance
```

2. **Isolate CPUs**
Add to kernel boot parameters:
```
isolcpus=1-7 nohz_full=1-7
```

3. **Disable Hyperthreading** (in BIOS)

4. **Use NUMA-aware allocation**
```bash
numactl --cpunodebind=0 --membind=0 ./your_app
```

5. **Monitor performance**
```bash
# Cache statistics
perf stat -e cache-references,cache-misses ./your_app

# CPU performance
perf record -g ./your_app
perf report
```

## Project Ideas

### Beginner
1. Custom memory allocator
2. Lock-free queue
3. Simple packet parser
4. TCP echo server

### Intermediate
1. Memory pool with NUMA awareness
2. SIMD-optimized data processing
3. epoll-based web server
4. Consistent hashing load balancer

### Advanced
1. DPDK packet forwarder
2. Zero-copy network proxy
3. Distributed cache with Raft
4. High-frequency trading system

## Resources

### Documentation
- Each module has its own README.md
- Inline code comments explain concepts
- See `docs/` directory for additional guides

### External Resources
- [DPDK Documentation](https://doc.dpdk.org/)
- [Intel Optimization Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)
- [Linux perf Wiki](https://perf.wiki.kernel.org/)
- [Brendan Gregg's Blog](http://www.brendangregg.com/)

### Books
- "What Every Programmer Should Know About Memory" - Ulrich Drepper
- "Systems Performance" - Brendan Gregg
- "The Linux Programming Interface" - Michael Kerrisk
- "High Performance Browser Networking" - Ilya Grigorik

## Contributing

Feel free to:
- Add more examples
- Improve documentation
- Report issues
- Share your projects built with these techniques

## Getting Help

1. Check module-specific README files
2. Read inline code comments
3. Search for error messages online
4. Check DPDK mailing list for DPDK-specific issues

## Next Steps

1. ✅ Build all examples
2. ✅ Run and understand each example
3. ✅ Modify examples to experiment
4. ✅ Complete exercises in each module
5. ✅ Build your own project
6. ✅ Measure and optimize performance
7. ✅ Share your learnings!

Good luck with your high-performance systems programming journey! 🚀

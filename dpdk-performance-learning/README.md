# DPDK & High-Performance Systems Programming Learning Project

A comprehensive deep-dive project covering DPDK, performance optimization, system programming, memory management, CPU optimization, and distributed systems in C/C++.

## 🎯 Learning Objectives

This project is designed to provide hands-on experience with:
- **DPDK (Data Plane Development Kit)** - High-performance packet processing
- **Performance Optimization** - Profiling, benchmarking, and optimization techniques
- **System Programming** - IPC, signals, threads, processes
- **Memory Management** - Custom allocators, memory pools, huge pages, zero-copy
- **CPU Optimization** - SIMD, cache optimization, prefetching, atomic operations
- **Distributed Systems** - Networking, message passing, consensus algorithms

## 📁 Project Structure

```
dpdk-performance-learning/
├── 01-dpdk/                    # DPDK Learning Module
│   ├── examples/               # Working DPDK examples
│   ├── exercises/              # Practice exercises
│   └── docs/                   # DPDK documentation and guides
├── 02-performance-optimization/ # Performance Optimization
│   ├── cache/                  # Cache-friendly programming
│   ├── branch-prediction/      # Branch prediction optimization
│   ├── profiling/              # Profiling tools and techniques
│   └── benchmarking/           # Micro-benchmarking
├── 03-system-programming/      # System Programming
│   ├── ipc/                    # Inter-Process Communication
│   ├── signals/                # Signal handling
│   ├── threads/                # Multi-threading
│   └── processes/              # Process management
├── 04-memory-management/       # Advanced Memory Management
│   ├── allocators/             # Custom memory allocators
│   ├── memory-pools/           # Memory pool implementations
│   ├── huge-pages/             # Huge pages usage
│   └── zero-copy/              # Zero-copy techniques
├── 05-cpu-optimization/        # CPU Optimization Techniques
│   ├── simd/                   # SIMD programming (SSE, AVX)
│   ├── cache-optimization/     # Cache line optimization
│   ├── prefetching/            # Data prefetching
│   └── atomic-operations/      # Lock-free programming
├── 06-distributed-systems/     # Distributed Systems
│   ├── networking/             # High-performance networking
│   ├── message-passing/        # Message passing patterns
│   ├── consensus/              # Consensus algorithms
│   └── load-balancing/         # Load balancing strategies
├── common/                     # Shared code
│   ├── include/                # Common headers
│   └── lib/                    # Common libraries
├── docs/                       # General documentation
└── build/                      # Build artifacts
```

## 🚀 Getting Started

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake git \
    libnuma-dev libpcap-dev pkg-config \
    linux-headers-$(uname -r) \
    python3-pip python3-pyelftools

# For DPDK (optional - see 01-dpdk/docs/setup.md)
# Download and build DPDK separately
```

### Building the Project

```bash
# Build all modules
make all

# Or build with CMake
mkdir build && cd build
cmake ..
make
```

### Running Examples

Each module has its own README with specific instructions. Generally:

```bash
# Navigate to a module
cd 02-performance-optimization/cache

# Build the example
make

# Run the example
./cache_example
```

## 📚 Learning Path

### 1. Foundation (Start Here)
- **03-system-programming/**: Learn system programming basics
- **04-memory-management/allocators**: Understand memory allocation
- **05-cpu-optimization/cache-optimization**: Learn about CPU caches

### 2. Performance Fundamentals
- **02-performance-optimization/profiling**: Learn to measure performance
- **02-performance-optimization/benchmarking**: Write micro-benchmarks
- **05-cpu-optimization/simd**: Vectorize your code

### 3. Advanced Techniques
- **04-memory-management/memory-pools**: Implement custom memory pools
- **04-memory-management/huge-pages**: Use huge pages for performance
- **05-cpu-optimization/atomic-operations**: Lock-free programming

### 4. DPDK Deep Dive
- **01-dpdk/examples**: Start with basic DPDK examples
- **01-dpdk/exercises**: Practice with DPDK programming
- **04-memory-management/zero-copy**: Understand zero-copy with DPDK

### 5. Distributed Systems
- **06-distributed-systems/networking**: High-performance networking
- **06-distributed-systems/message-passing**: Distributed communication
- **06-distributed-systems/consensus**: Implement consensus protocols

## 🔧 Tools Used

- **Compilers**: GCC 9+, Clang 10+
- **Build Systems**: Make, CMake
- **Profiling**: perf, gprof, Valgrind, Intel VTune
- **Benchmarking**: Google Benchmark, custom tools
- **DPDK**: Version 21.11 or later (LTS)
- **Libraries**: pthread, libnuma, libpcap

## 📖 Key Concepts Covered

### DPDK
- Environment Abstraction Layer (EAL)
- Memory pools (mempools)
- Ring buffers
- Poll Mode Drivers (PMD)
- Packet processing pipelines
- Multi-core scaling

### Performance Optimization
- Cache-friendly data structures
- Branch prediction optimization
- Loop unrolling
- Function inlining
- Profile-guided optimization

### Memory Management
- Arena allocators
- Slab allocators
- Object pools
- Huge pages (2MB, 1GB)
- NUMA-aware allocation
- Memory alignment

### CPU Optimization
- SIMD (SSE, AVX, AVX-512)
- Cache line optimization
- False sharing prevention
- Prefetching strategies
- Memory barriers
- Lock-free data structures

### Distributed Systems
- TCP/UDP optimization
- Zero-copy networking
- RDMA basics
- Raft consensus
- Two-phase commit
- Load balancing algorithms

## 📊 Performance Metrics

Each module includes performance benchmarks and metrics:
- Throughput (ops/sec, packets/sec)
- Latency (p50, p95, p99, p999)
- CPU utilization
- Memory bandwidth
- Cache hit rates
- Context switch rates

## 🎓 Learning Resources

- [DPDK Documentation](https://doc.dpdk.org/)
- [Intel 64 and IA-32 Architectures Optimization Reference Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [What Every Programmer Should Know About Memory](https://people.freebsd.org/~lstewart/articles/cpumemory.pdf)
- [The C10K Problem](http://www.kegel.com/c10k.html)
- [Systems Performance by Brendan Gregg](http://www.brendangregg.com/systems-performance-2nd-edition-book.html)

## 🤝 Contributing

This is a learning project. Feel free to:
- Add more examples
- Improve documentation
- Add exercises
- Share performance results

## 📝 Notes

- All code includes detailed comments explaining concepts
- Each example has a corresponding exercise
- Performance numbers are from specific hardware (see docs/hardware.md)
- DPDK examples require proper setup (see 01-dpdk/docs/setup.md)

## ⚠️ Safety Notes

- DPDK applications require root privileges or capabilities
- Huge pages need to be configured in the system
- Some examples bind network devices (backup your config)
- Always test in a safe environment first

## 🏆 Goals

By completing this project, you should be able to:
1. Write high-performance packet processing applications with DPDK
2. Profile and optimize C/C++ code for maximum performance
3. Implement lock-free data structures
4. Design NUMA-aware applications
5. Build distributed systems with optimal network performance
6. Understand CPU microarchitecture and its impact on performance

## License

MIT License - Feel free to use for learning purposes.

---

**Happy Learning! 🚀**

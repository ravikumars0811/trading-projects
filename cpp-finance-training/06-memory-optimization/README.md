# Module 6: Memory Optimization

## Topics Covered

### 1. Memory Layout and Alignment
- Stack vs Heap
- Memory alignment requirements
- Padding and structure packing
- Cache line alignment
- alignas specifier

### 2. Cache Optimization
- CPU cache hierarchy (L1, L2, L3)
- Cache line size (typically 64 bytes)
- Cache-friendly data structures
- Avoiding cache misses
- Prefetching data

### 3. Data Structure Layout
- Array of Structures (AoS) vs Structure of Arrays (SoA)
- Hot/cold data splitting
- Data-oriented design
- Minimizing indirection

### 4. Custom Allocators
- Memory pools
- Slab allocators
- Arena allocators
- Stack allocators
- Avoiding allocations in hot paths

### 5. Memory Fragmentation
- Internal vs external fragmentation
- Avoiding fragmentation
- Compaction strategies
- Fixed-size blocks

### 6. SIMD Operations
- SSE/AVX instructions
- Vectorization
- Aligned memory for SIMD
- Auto-vectorization hints

### 7. Branch Prediction
- Likely/unlikely macros
- Branchless programming
- Profile-guided optimization (PGO)

### 8. Profiling and Benchmarking
- valgrind/cachegrind
- perf tool
- Google Benchmark
- Measuring cache misses

## Code Examples

1. `01_memory_layout.cpp` - Stack, heap, alignment
2. `02_cache_optimization.cpp` - Cache-friendly code
3. `03_aos_vs_soa.cpp` - Data layout comparison
4. `04_custom_allocators.cpp` - Memory pools
5. `05_simd_vectorization.cpp` - SIMD examples
6. `06_branch_optimization.cpp` - Branch prediction
7. `07_profiling.cpp` - Performance measurement

## Key Principles

### Cache-Friendly Programming
1. **Locality**: Access nearby memory
2. **Sequential**: Linear access is faster
3. **Alignment**: Align to cache line boundaries
4. **Prefetch**: Hint next memory access
5. **Size**: Keep working set in L1/L2 cache

### Memory Access Patterns
```
Fast:    Sequential access (streaming)
Medium:  Strided access (skip N bytes)
Slow:    Random access (pointer chasing)
```

### False Sharing
```cpp
// BAD: Different threads modify adjacent data
struct {
    std::atomic<int> counter1;  // Same cache line!
    std::atomic<int> counter2;  // False sharing!
} shared_data;

// GOOD: Pad to separate cache lines
struct alignas(64) {
    std::atomic<int> counter1;
    char padding[60];
} data1;

struct alignas(64) {
    std::atomic<int> counter2;
    char padding[60];
} data2;
```

## Optimization Checklist

- [ ] Minimize allocations in hot paths
- [ ] Use memory pools for frequent allocations
- [ ] Align data structures to cache lines
- [ ] Avoid false sharing in multi-threaded code
- [ ] Prefer SoA for SIMD-friendly code
- [ ] Keep hot data together, cold data separate
- [ ] Use const for read-only data (can be shared)
- [ ] Profile before optimizing
- [ ] Measure cache miss rates

## HFT Applications

### Order Book
- Cache-friendly price level layout
- Memory pool for order objects
- Minimize pointer indirection

### Market Data Processing
- SIMD for parallel price calculations
- Sequential access patterns
- Prefetch next tick data

### Risk Calculations
- Batch processing for cache efficiency
- Vectorized mathematical operations
- Avoid branches in inner loops

## Compiler Flags

```bash
# Optimization flags
-O3                 # Aggressive optimization
-march=native       # Use CPU-specific instructions
-mtune=native       # Tune for this CPU
-ffast-math         # Fast math (may lose precision)
-funroll-loops      # Loop unrolling
-flto               # Link-time optimization

# SIMD flags
-msse4.2           # Enable SSE 4.2
-mavx2             # Enable AVX2
-mavx512f          # Enable AVX-512

# Profiling
-fprofile-generate # Generate profile data
-fprofile-use      # Use profile data (PGO)
```

## Measurement Tools

```bash
# Cache profiling
valgrind --tool=cachegrind ./program
perf stat -e cache-misses,cache-references ./program

# Memory profiling
valgrind --tool=massif ./program
heaptrack ./program

# CPU profiling
perf record -g ./program
perf report
```

## Typical Sizes

```
L1 Cache:    32-64 KB (per core)
L2 Cache:    256-512 KB (per core)
L3 Cache:    8-32 MB (shared)
Cache Line:  64 bytes
Page Size:   4 KB
```

## Performance Gains

Proper memory optimization can yield:
- **2-10x**: Cache-friendly data layout
- **4-8x**: SIMD vectorization
- **10-100x**: Eliminating allocations in hot path
- **2-5x**: Avoiding false sharing
- **10-1000x**: Lock-free vs locked structures

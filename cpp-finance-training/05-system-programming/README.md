# Module 5: System Programming

## Topics Covered

### 1. Multi-threading and Concurrency
- std::thread basics
- Thread synchronization
- Mutexes and locks
- Condition variables
- Thread pools
- Lock-free programming

### 2. Atomics and Memory Ordering
- std::atomic operations
- Memory order semantics
- Lock-free data structures
- Compare-and-swap (CAS)
- ABA problem

### 3. Inter-Process Communication (IPC)
- Shared memory
- Message queues
- Semaphores
- Pipes and FIFOs
- Memory-mapped files

### 4. Network Programming
- Berkeley sockets
- TCP/UDP programming
- Non-blocking I/O
- epoll/select/poll
- Zero-copy techniques

### 5. Time and Timing
- High-resolution clocks
- Clock synchronization
- TSC (Time Stamp Counter)
- RDTSC instruction
- PTP (Precision Time Protocol)

### 6. Signal Handling
- Signal handlers
- Async-signal-safe functions
- Real-time signals

### 7. Process Management
- Fork and exec
- Process affinity
- CPU pinning
- Priority and scheduling

## Code Examples

1. `01_threading.cpp` - Thread creation and synchronization
2. `02_atomics.cpp` - Lock-free programming
3. `03_mutex_locks.cpp` - Thread-safe data structures
4. `04_lockfree_queue.cpp` - Lock-free queue implementation
5. `05_shared_memory.cpp` - IPC with shared memory
6. `06_sockets.cpp` - Network programming basics
7. `07_high_resolution_timing.cpp` - Nanosecond-precision timing

## HFT Focus Areas

### Low-Latency Requirements
- Lock-free algorithms
- CPU pinning
- Memory-mapped I/O
- Kernel bypass (DPDK concepts)
- Busy polling vs blocking

### Concurrency Patterns
- Producer-consumer queues
- Ring buffers
- Wait-free algorithms
- Thread-safe order books

### Timing and Synchronization
- TSC for timestamp
- Clock synchronization across machines
- Measuring and minimizing jitter

## Compilation

```bash
# With pthread support
g++ -std=c++20 -O3 -pthread filename.cpp -o output

# With atomic support
g++ -std=c++20 -O3 -pthread -latomic filename.cpp -o output

# With real-time priority
g++ -std=c++20 -O3 -pthread -lrt filename.cpp -o output
```

## Performance Considerations

1. **Lock-Free > Lock-Based**: When possible
2. **Busy-Wait > Sleep**: For ultra-low latency
3. **CPU Affinity**: Pin threads to cores
4. **NUMA Awareness**: Memory locality matters
5. **Cache Line Padding**: Avoid false sharing

## Real-World Applications

- Market data feed handlers
- Order management systems
- Matching engines
- Risk management systems
- Trade execution algorithms

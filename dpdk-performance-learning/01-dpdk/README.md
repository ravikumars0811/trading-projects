# DPDK Learning Module

## Overview

Data Plane Development Kit (DPDK) is a set of libraries and drivers for fast packet processing. It enables direct access to network hardware, bypassing the kernel network stack for maximum performance.

## Key Concepts

### 1. Environment Abstraction Layer (EAL)
- Initialization and configuration
- CPU core affinity
- Memory management
- PCI device access

### 2. Memory Pools (Mempools)
- Pre-allocated memory pools for packets
- Lock-free allocation/deallocation
- NUMA-aware allocation

### 3. Ring Buffers
- Lock-free multi-producer/multi-consumer queues
- Used for inter-core communication
- Bulk operations for efficiency

### 4. Poll Mode Drivers (PMD)
- User-space drivers that poll for packets
- No interrupts = lower latency
- Dedicated CPU cores for polling

### 5. Packet Buffers (mbuf)
- Standard packet buffer format
- Metadata and data separation
- Chain support for large packets

## Examples

1. **basic_eal_init.c** - EAL initialization
2. **mempool_example.c** - Memory pool creation and usage
3. **ring_example.c** - Ring buffer operations
4. **packet_forward.c** - Simple packet forwarding
5. **multi_core_forward.c** - Multi-core packet processing
6. **l2_forward.c** - Layer 2 forwarding
7. **packet_capture.c** - Packet capture to file

## Performance Tips

1. **Use Huge Pages**: Reduces TLB misses
2. **CPU Isolation**: Isolate cores from kernel scheduler
3. **NUMA Awareness**: Allocate memory on same NUMA node as NIC
4. **Batch Processing**: Process packets in batches
5. **Prefetching**: Prefetch packet data before processing

## Setup Required

See `docs/setup.md` for detailed setup instructions.

## Building

```bash
# Make sure DPDK is installed and RTE_SDK is set
export RTE_SDK=/path/to/dpdk
export RTE_TARGET=x86_64-native-linux-gcc

# Build examples
cd examples
make
```

## Running

```bash
# Most DPDK apps need huge pages configured
sudo sysctl -w vm.nr_hugepages=1024

# Run with EAL parameters
sudo ./basic_eal_init -l 0-3 -n 4
```

## Learning Path

1. Start with `basic_eal_init.c` to understand EAL
2. Learn memory management with `mempool_example.c`
3. Understand ring buffers with `ring_example.c`
4. Practice with `packet_forward.c`
5. Scale with `multi_core_forward.c`
6. Complete exercises in `exercises/` directory

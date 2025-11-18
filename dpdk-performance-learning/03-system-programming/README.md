# System Programming Module

## Overview

System programming fundamentals for high-performance applications in C/C++.

## Topics

### 1. Inter-Process Communication (IPC)
- Shared memory
- Message queues
- Pipes and FIFOs
- Unix domain sockets
- Memory-mapped files

### 2. Signal Handling
- Signal basics
- Real-time signals
- Signal-safe programming
- Async-signal-safe functions

### 3. Multi-threading
- POSIX threads (pthreads)
- Thread synchronization (mutexes, semaphores)
- Thread affinity
- Thread-local storage

### 4. Process Management
- Fork and exec
- Process groups and sessions
- Resource limits
- Process scheduling

## Building

```bash
cd 03-system-programming
make
```

## Examples

- **shared_memory.c**: Shared memory IPC
- **thread_pool.c**: High-performance thread pool
- **signal_handling.c**: Safe signal handling
- **cpu_affinity.c**: CPU core pinning

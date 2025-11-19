# Distributed Systems Module

## Overview

High-performance distributed systems programming in C/C++.

## Topics

### 1. High-Performance Networking
- TCP optimization (Nagle, delayed ACK)
- UDP programming
- Zero-copy networking
- epoll/io_uring
- Non-blocking I/O

### 2. Message Passing
- Protocol design
- Serialization (Protocol Buffers, FlatBuffers)
- Message queues
- Publisher-Subscriber patterns

### 3. Consensus Algorithms
- Raft consensus
- Two-phase commit
- Leader election
- State machine replication

### 4. Load Balancing
- Round-robin
- Least connections
- Consistent hashing
- Health checking

## Key Concepts

- **Fault Tolerance**: Handle failures gracefully
- **Scalability**: Horizontal scaling
- **Consistency**: CAP theorem, eventual consistency
- **Latency**: Minimize network round-trips
- **Throughput**: Batch operations

## Networking Performance

### TCP Optimization
```c
// Disable Nagle's algorithm
int flag = 1;
setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

// Enable TCP_QUICKACK
setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag));

// Set receive/send buffer sizes
int bufsize = 4 * 1024 * 1024;  // 4MB
setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
```

### UDP for Low Latency
```c
// Use UDP for lowest latency (no connection overhead)
// Handle packet loss and ordering in application
```

### Zero-Copy with sendfile
```c
// Transfer file without copying to userspace
sendfile(out_fd, in_fd, &offset, count);
```

## Examples

- **tcp_server.c**: High-performance TCP server with epoll
- **udp_pingpong.c**: UDP latency measurement
- **message_queue.c**: Lock-free message queue
- **consistent_hash.c**: Consistent hashing implementation
- **raft_simple.c**: Simplified Raft consensus

## Tools

- **iperf3**: Network bandwidth testing
- **netperf**: Network performance benchmark
- **tcpdump**: Packet capture and analysis
- **ss**: Socket statistics

## Performance Metrics

- **Latency**: RTT (Round-Trip Time)
- **Throughput**: Messages/sec, Bytes/sec
- **Packet Loss**: Percentage
- **Jitter**: Latency variance

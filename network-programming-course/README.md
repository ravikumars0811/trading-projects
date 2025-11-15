# Network Programming Course for High-Performance Applications

## 🎯 Course Overview

This comprehensive course covers network programming with a focus on low-latency, high-performance applications and AWS cloud networking. All examples are provided in C++ to demonstrate production-grade implementations.

## 🎓 Learning Objectives

- Master network fundamentals and the TCP/IP stack
- Build high-performance network applications in C++
- Understand and implement low-latency networking techniques
- Learn advanced I/O models (epoll, io_uring, DPDK)
- Optimize network applications for AWS cloud environments
- Implement zero-copy, kernel bypass, and RDMA techniques
- Design scalable and resilient network architectures

## 📚 Course Modules

### [Module 1: Network Fundamentals](./module-01-fundamentals/README.md)
- OSI and TCP/IP Models
- Network Protocols (TCP, UDP, IP, ICMP)
- Packet Structure and Analysis
- Network Performance Metrics
- Latency, Throughput, and Jitter

### [Module 2: Socket Programming in C++](./module-02-socket-programming/README.md)
- BSD Sockets API
- TCP Client/Server Implementation
- UDP Client/Server Implementation
- Socket Options and Performance Tuning
- Error Handling and Best Practices

### [Module 3: Advanced I/O Models](./module-03-advanced-io/README.md)
- Blocking vs Non-Blocking I/O
- I/O Multiplexing (select, poll, epoll)
- Event-Driven Architecture
- io_uring (Modern Linux Async I/O)
- Asynchronous I/O Patterns

### [Module 4: High-Performance Networking Techniques](./module-04-high-performance/README.md)
- Zero-Copy Techniques
- Memory Management and Buffer Pools
- CPU Affinity and NUMA Awareness
- Kernel Bypass (DPDK, XDP)
- RDMA (Remote Direct Memory Access)
- Network Stack Optimization

### [Module 5: Network Protocols and Optimization](./module-05-protocols/README.md)
- TCP Optimization (Nagle, TCP_NODELAY, TCP_QUICKACK)
- UDP Optimization and Reliability
- Multicast and Broadcast
- Custom Protocol Design
- Message Framing and Serialization
- Protocol Buffers, FlatBuffers, Cap'n Proto

### [Module 6: AWS Networking for Developers](./module-06-aws-networking/README.md)
- VPC Architecture and Design
- Security Groups and NACLs
- Elastic Load Balancing
- CloudFront and Edge Networking
- Direct Connect and VPN
- Route 53 and DNS
- AWS Enhanced Networking (SR-IOV, ENA)
- Container Networking (ECS, EKS)
- Lambda and Serverless Networking

### [Module 7: Low-Latency Application Design](./module-07-low-latency/README.md)
- Latency Sources and Measurement
- Timestamping Techniques (Hardware, Software)
- Lock-Free Data Structures
- Cache Optimization
- Jitter Reduction
- Real-World Case Studies (HFT, Gaming, Streaming)

## 🛠️ Prerequisites

- Strong C++ knowledge (C++17 or later)
- Linux operating system basics
- Basic networking concepts
- CMake and build systems
- AWS account (for Module 6)

## 🔧 Development Environment Setup

```bash
# Install required packages (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    g++ \
    libboost-all-dev \
    libaio-dev \
    liburing-dev \
    numactl \
    linux-tools-common \
    linux-tools-generic \
    tcpdump \
    wireshark \
    iperf3 \
    netcat \
    socat

# Clone the course repository
git clone <repository-url>
cd network-programming-course

# Build all examples
./build_all.sh
```

## 📖 How to Use This Course

1. **Sequential Learning**: Start from Module 1 and progress through each module
2. **Hands-On Practice**: Every module includes practical C++ examples
3. **Lab Exercises**: Complete the exercises at the end of each module
4. **Performance Testing**: Benchmark your implementations
5. **AWS Practice**: Set up AWS resources for Module 6

## 🔬 Lab Environment

Each module contains:
- `README.md`: Theory and concepts
- `examples/`: Working C++ code examples
- `exercises/`: Practice problems
- `solutions/`: Reference solutions
- `diagrams/`: Visual explanations

## 📊 Performance Metrics

Throughout the course, you'll learn to measure:
- **Latency**: p50, p95, p99, p99.9 percentiles
- **Throughput**: Messages/second, GB/s
- **Jitter**: Standard deviation, max deviation
- **CPU Usage**: Per-core utilization
- **Memory**: Allocations, cache misses

## 🎯 Real-World Applications

This course prepares you for:
- High-Frequency Trading (HFT) systems
- Real-time multiplayer gaming
- Live video streaming
- IoT and edge computing
- Microservices architecture
- Cloud-native applications

## 📚 Additional Resources

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [TCP/IP Illustrated by Stevens](https://www.amazon.com/TCP-Illustrated-Vol-Addison-Wesley-Professional/dp/0201633469)
- [Linux System Programming by Robert Love](https://www.amazon.com/Linux-System-Programming-Talking-Directly/dp/1449339530)
- [AWS Networking Documentation](https://docs.aws.amazon.com/vpc/)

## 🤝 Contributing

Feel free to submit issues, improvements, or additional examples!

## 📝 License

This course material is provided for educational purposes.

---

**Ready to master network programming? Start with [Module 1: Network Fundamentals](./module-01-fundamentals/README.md)!**

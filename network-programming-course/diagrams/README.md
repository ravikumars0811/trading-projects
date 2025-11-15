# Network Programming Course Diagrams

This directory contains various diagrams used throughout the course.

## ASCII Art Diagrams

All diagrams in the course modules are created using ASCII art for:
- Maximum compatibility
- Easy viewing in terminals
- Version control friendly
- No external dependencies

## Topics Covered

### Module 1: Network Fundamentals
- OSI Model layers
- TCP/IP stack
- Packet structure
- Network latency breakdown

### Module 2: Socket Programming
- TCP client/server flow
- UDP communication
- Byte order conversion
- Socket options impact

### Module 3: Advanced I/O Models
- Blocking vs non-blocking I/O
- select/poll/epoll comparison
- io_uring architecture
- Event-driven patterns

### Module 4: High-Performance Techniques
- Zero-copy data path
- NUMA architecture
- DPDK architecture
- RDMA communication

### Module 5: Protocol Optimization
- TCP handshake
- Nagle's algorithm impact
- Multicast distribution
- Serialization comparison

### Module 6: AWS Networking
- VPC architecture
- Load balancer types
- CloudFront edge locations
- Direct Connect topology

### Module 7: Low-Latency Design
- Latency budget breakdown
- Cache hierarchy
- Lock-free queue design
- HFT system architecture

## Creating New Diagrams

When adding new diagrams:
1. Use ASCII art for text-based diagrams
2. Keep width under 80 characters when possible
3. Use consistent symbols:
   - Boxes: ┌─┐ └─┘
   - Arrows: → ← ↑ ↓
   - Lines: │ ─
4. Add clear labels
5. Include legend if needed

## Tools

Recommended tools for creating ASCII diagrams:
- [Monodraw](https://monodraw.helftone.com/) (macOS)
- [ASCIIFlow](https://asciiflow.com/) (Web)
- [Textik](https://textik.com/) (Web)
- Manual creation in text editor

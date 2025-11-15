# Module 1: Network Fundamentals

## Overview

This module covers the foundational concepts of computer networking essential for building high-performance applications.

## Table of Contents

1. [OSI and TCP/IP Models](#osi-and-tcpip-models)
2. [Network Protocols](#network-protocols)
3. [Packet Structure](#packet-structure)
4. [Performance Metrics](#performance-metrics)
5. [Latency Analysis](#latency-analysis)

---

## OSI and TCP/IP Models

### OSI Model (7 Layers)

```
┌─────────────────────────────────────┐
│  7. Application Layer               │  HTTP, FTP, DNS, SMTP
│     (Data)                          │
├─────────────────────────────────────┤
│  6. Presentation Layer              │  SSL/TLS, Encryption
│     (Data)                          │
├─────────────────────────────────────┤
│  5. Session Layer                   │  Session Management
│     (Data)                          │
├─────────────────────────────────────┤
│  4. Transport Layer                 │  TCP, UDP
│     (Segments/Datagrams)            │
├─────────────────────────────────────┤
│  3. Network Layer                   │  IP, ICMP, Routing
│     (Packets)                       │
├─────────────────────────────────────┤
│  2. Data Link Layer                 │  Ethernet, MAC, Switches
│     (Frames)                        │
├─────────────────────────────────────┤
│  1. Physical Layer                  │  Cables, Signals, Bits
│     (Bits)                          │
└─────────────────────────────────────┘
```

### TCP/IP Model (4 Layers)

```
┌─────────────────────────────────────┐
│  Application Layer                  │  HTTP, FTP, DNS, SSH
│  (OSI Layers 5-7)                   │
├─────────────────────────────────────┤
│  Transport Layer                    │  TCP, UDP
│  (OSI Layer 4)                      │
├─────────────────────────────────────┤
│  Internet Layer                     │  IP, ICMP, ARP
│  (OSI Layer 3)                      │
├─────────────────────────────────────┤
│  Network Access Layer               │  Ethernet, Wi-Fi
│  (OSI Layers 1-2)                   │
└─────────────────────────────────────┘
```

---

## Network Protocols

### TCP (Transmission Control Protocol)

**Characteristics:**
- Connection-oriented
- Reliable delivery
- Ordered packets
- Flow control
- Congestion control
- Error checking

**Use Cases:**
- Web browsing (HTTP/HTTPS)
- Email (SMTP, IMAP)
- File transfer (FTP, SFTP)
- When data integrity is critical

**TCP Header Structure:**

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          Source Port          |       Destination Port        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Sequence Number                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Acknowledgment Number                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Data |           |U|A|P|R|S|F|                               |
| Offset| Reserved  |R|C|S|S|Y|I|            Window             |
|       |           |G|K|H|T|N|N|                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           Checksum            |         Urgent Pointer        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Options                    |    Padding    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                             data                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

### UDP (User Datagram Protocol)

**Characteristics:**
- Connectionless
- Unreliable (no guarantees)
- Unordered
- No flow control
- Minimal overhead
- Low latency

**Use Cases:**
- Video streaming
- Voice over IP (VoIP)
- Online gaming
- DNS queries
- Real-time data feeds (market data)

**UDP Header Structure:**

```
 0      7 8     15 16    23 24    31
+--------+--------+--------+--------+
|     Source      |   Destination   |
|      Port       |      Port       |
+--------+--------+--------+--------+
|                 |                 |
|     Length      |    Checksum     |
+--------+--------+--------+--------+
|
|          data octets ...
+---------------- ...
```

### TCP vs UDP Comparison

| Feature | TCP | UDP |
|---------|-----|-----|
| Connection | Connection-oriented | Connectionless |
| Reliability | Guaranteed delivery | No guarantee |
| Ordering | Packets ordered | No ordering |
| Speed | Slower (overhead) | Faster |
| Header Size | 20-60 bytes | 8 bytes |
| Error Checking | Extensive | Basic checksum |
| Use Case | Reliability critical | Speed critical |

---

## Packet Structure

### Complete Packet Journey

```
Application Data
      ↓
┌─────────────────────────┐
│   Application Header   │
│   Application Data     │  ← Application Layer
└─────────────────────────┘
      ↓
┌─────────────────────────┐
│   TCP/UDP Header       │  ← Transport Layer
│   Application Data     │
└─────────────────────────┘
      ↓
┌─────────────────────────┐
│   IP Header            │  ← Network Layer
│   TCP/UDP Header       │
│   Application Data     │
└─────────────────────────┘
      ↓
┌─────────────────────────┐
│   Ethernet Header      │  ← Data Link Layer
│   IP Header            │
│   TCP/UDP Header       │
│   Application Data     │
│   Ethernet Trailer     │
└─────────────────────────┘
      ↓
    Network
```

### IP Packet Structure

```
IPv4 Header:
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Version|  IHL  |Type of Service|          Total Length         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|         Identification        |Flags|      Fragment Offset    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Time to Live |    Protocol   |         Header Checksum       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Source Address                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Destination Address                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Options                    |    Padding    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

---

## Performance Metrics

### Key Metrics for High-Performance Applications

#### 1. Latency
- **Definition**: Time taken for a packet to travel from source to destination
- **Measured in**: Microseconds (μs) or milliseconds (ms)
- **Types**:
  - One-way latency
  - Round-trip time (RTT)
  - Processing latency
  - Queuing latency

#### 2. Throughput
- **Definition**: Amount of data transferred per unit time
- **Measured in**: Mbps, Gbps, or messages/second
- **Factors**:
  - Bandwidth
  - Protocol overhead
  - Network congestion
  - Application efficiency

#### 3. Jitter
- **Definition**: Variation in packet arrival times
- **Measured in**: Standard deviation of latency
- **Impact**: Critical for real-time applications

#### 4. Packet Loss
- **Definition**: Percentage of packets that don't reach destination
- **Measured in**: Percentage (%)
- **Causes**:
  - Network congestion
  - Buffer overflow
  - Transmission errors

### Latency Breakdown

```
Total Latency = Propagation + Transmission + Processing + Queuing

┌────────────────────────────────────────────────────────┐
│                                                        │
│  Application Processing                                │  ← App Latency
│  ↓                                                     │
│  System Call                                           │  ← Kernel Transition
│  ↓                                                     │
│  Kernel Processing                                     │  ← OS Latency
│  ↓                                                     │
│  Network Stack                                         │  ← Stack Latency
│  ↓                                                     │
│  NIC Processing                                        │  ← Hardware Latency
│  ↓                                                     │
│  Wire Transmission                                     │  ← Physical Latency
│  ↓                                                     │
│  Network Switches/Routers                              │  ← Network Latency
│  ↓                                                     │
│  Destination NIC                                       │
│  ↓                                                     │
│  Destination Stack                                     │
│  ↓                                                     │
│  Destination Application                               │
│                                                        │
└────────────────────────────────────────────────────────┘

Typical Latencies:
- L1 Cache: ~1 ns
- L2 Cache: ~3 ns
- L3 Cache: ~10 ns
- RAM: ~100 ns
- SSD: ~100 μs
- LAN (1 Gbps): ~0.5 ms
- Internet (cross-country): ~50-100 ms
```

---

## Latency Analysis

### Sources of Latency

1. **Propagation Delay**
   - Speed of light in fiber: ~200,000 km/s
   - 1000 km ≈ 5 ms one-way

2. **Transmission Delay**
   - Time to push packet onto wire
   - Depends on link bandwidth

3. **Processing Delay**
   - Router/switch processing
   - Application processing
   - System call overhead

4. **Queuing Delay**
   - Buffer waiting time
   - Most variable component

### Latency Optimization Strategies

```
┌────────────────────────────────────────────────────┐
│  Strategy              │  Improvement              │
├────────────────────────────────────────────────────┤
│  Kernel Bypass         │  10-50x reduction         │
│  Zero-Copy             │  2-5x reduction           │
│  CPU Pinning           │  20-30% reduction         │
│  TCP Optimization      │  10-30% reduction         │
│  Protocol Simplification│  30-50% reduction        │
│  Direct Hardware Access│  100-1000x reduction      │
└────────────────────────────────────────────────────┘
```

---

## Network Bandwidth Hierarchy

```
Technology          Bandwidth       Latency (typical)
────────────────────────────────────────────────────
CPU L1 Cache        ~1 TB/s         1 ns
CPU L2 Cache        ~500 GB/s       3 ns
CPU L3 Cache        ~200 GB/s       10 ns
DDR4 RAM            ~25 GB/s        100 ns
NVMe SSD            ~7 GB/s         100 μs
100 GbE             12.5 GB/s       1-10 μs
40 GbE              5 GB/s          1-10 μs
10 GbE              1.25 GB/s       1-10 μs
1 GbE               125 MB/s        100 μs - 1 ms
InfiniBand (200G)   25 GB/s         <1 μs
RDMA                50+ GB/s        <1 μs
```

---

## Practical Example: Ping Analysis

### Understanding Ping Output

```bash
$ ping -c 4 google.com
PING google.com (142.250.185.46) 56(84) bytes of data.
64 bytes from lga25s65-in-f14.1e100.net (142.250.185.46): icmp_seq=1 ttl=117 time=12.3 ms
64 bytes from lga25s65-in-f14.1e100.net (142.250.185.46): icmp_seq=2 ttl=117 time=11.8 ms
64 bytes from lga25s65-in-f14.1e100.net (142.250.185.46): icmp_seq=3 ttl=117 time=12.1 ms
64 bytes from lga25s65-in-f14.1e100.net (142.250.185.46): icmp_seq=4 ttl=117 time=12.0 ms

--- google.com ping statistics ---
4 packets transmitted, 4 received, 0% packet loss, time 3005ms
rtt min/avg/max/mdev = 11.845/12.050/12.316/0.175 ms
```

**Key Metrics:**
- **min**: Best-case latency (11.845 ms)
- **avg**: Average latency (12.050 ms)
- **max**: Worst-case latency (12.316 ms)
- **mdev**: Standard deviation/jitter (0.175 ms)

---

## MTU and MSS

### Maximum Transmission Unit (MTU)

```
Ethernet Frame:
┌────────┬────────┬──────┬─────────────┬─────┐
│ Header │  IP    │ TCP  │   Payload   │ CRC │
│ 14 B   │ 20 B   │ 20 B │   1460 B    │ 4 B │
└────────┴────────┴──────┴─────────────┴─────┘
         └──────────────────────────────┘
                  MTU = 1500 bytes

MSS (Maximum Segment Size) = MTU - IP Header - TCP Header
                            = 1500 - 20 - 20
                            = 1460 bytes
```

### Jumbo Frames

```
Standard Ethernet MTU: 1500 bytes
Jumbo Frame MTU:       9000 bytes

Benefits:
- Reduced CPU overhead (fewer packets)
- Lower header overhead
- Higher throughput

Requirements:
- All network devices must support jumbo frames
- Common in data center environments
```

---

## Exercise Questions

1. Calculate the theoretical maximum throughput for a 10 Gbps link with 1500-byte MTU sending TCP traffic.

2. If a fiber optic link is 500 km long, what is the minimum possible RTT?

3. Explain why UDP is preferred for high-frequency trading despite being unreliable.

4. What is the total overhead (in bytes) for sending 100 bytes of application data over TCP/IP on Ethernet?

5. Design a simple protocol for a low-latency market data feed. What transport protocol would you choose and why?

---

## Key Takeaways

1. **TCP provides reliability at the cost of latency**
   - Use for applications requiring guaranteed delivery
   - Avoid for ultra-low-latency applications

2. **UDP trades reliability for speed**
   - Minimal overhead
   - Application must handle reliability if needed

3. **Every layer adds overhead and latency**
   - Minimize layers when latency is critical
   - Consider kernel bypass for extreme performance

4. **MTU matters for throughput**
   - Larger MTU = less overhead
   - But fragmentation can hurt performance

5. **Measure everything**
   - p50, p95, p99, p99.9 percentiles
   - Don't just look at averages

---

## Next Steps

Continue to [Module 2: Socket Programming in C++](../module-02-socket-programming/README.md) where you'll implement these concepts in code.

## Additional Reading

- RFC 793: Transmission Control Protocol
- RFC 768: User Datagram Protocol
- RFC 791: Internet Protocol
- "TCP/IP Illustrated, Volume 1" by W. Richard Stevens

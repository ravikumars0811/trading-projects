# Module 5: Network Protocols and Optimization

## Overview

This module covers protocol-level optimizations, custom protocol design, and efficient serialization techniques for high-performance applications.

## Table of Contents

1. [TCP Optimization](#tcp-optimization)
2. [UDP Optimization](#udp-optimization)
3. [Multicast](#multicast)
4. [Custom Protocol Design](#custom-protocol-design)
5. [Message Framing](#message-framing)
6. [Serialization](#serialization)

---

## TCP Optimization

### TCP Parameters Deep Dive

#### Nagle's Algorithm

```
Problem: Small packets waste bandwidth
┌─────┬─────┬─────┐
│ IP  │ TCP │  1  │  41 bytes for 1 byte of data!
│ 20B │ 20B │byte │  97.5% overhead
└─────┴─────┴─────┘

Nagle's Algorithm:
- Buffer small writes
- Send when: (1) buffer full OR (2) ACK received
- Good for throughput, BAD for latency

Example without Nagle:
send("H");  → packet sent immediately
send("e");  → packet sent immediately
send("l");  → packet sent immediately
send("l");  → packet sent immediately
send("o");  → packet sent immediately
5 packets, 5 RTTs

Example with Nagle:
send("H");  → wait
send("e");  → wait
send("l");  → wait
send("l");  → wait
send("o");  → send "Hello" together
1 packet, 1 RTT

For latency-sensitive apps: DISABLE IT!
```

```cpp
// Disable Nagle's algorithm
int flag = 1;
setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

/*
 * Impact:
 * - Latency reduction: 40-200ms → <1ms
 * - Essential for: HFT, gaming, interactive protocols
 * - Cost: More packets, lower throughput for small messages
 */
```

#### Delayed ACK

```
TCP normally delays ACKs for 40-200ms to piggyback on return traffic

Timeline with Delayed ACK:
Client: send(data) →
Server:              ← receive (waits 40ms for return data)
Client:              ← ACK (after 40ms timeout)
Total: 40ms added latency!

Disable with TCP_QUICKACK:
```

```cpp
// Disable delayed ACK (must be set after each recv!)
int flag = 1;
while (1) {
    recv(sockfd, buf, len, 0);
    setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag));
    // Process data
}

/*
 * Note: TCP_QUICKACK is NOT persistent!
 * Must be re-enabled after each receive operation
 */
```

#### TCP Cork

```cpp
// Buffer data and send as single packet
int flag = 1;
setsockopt(sockfd, IPPROTO_TCP, TCP_CORK, &flag, sizeof(flag));

// All these writes are buffered
send(sockfd, header, header_len, 0);
send(sockfd, body, body_len, 0);
send(sockfd, footer, footer_len, 0);

// Uncork to send everything
flag = 0;
setsockopt(sockfd, IPPROTO_TCP, TCP_CORK, &flag, sizeof(flag));

/*
 * Use case:
 * - HTTP response with headers + body
 * - Prevents partial packets
 * - Opposite of TCP_NODELAY
 *
 * DON'T use TCP_CORK and TCP_NODELAY together!
 */
```

### TCP Window Scaling

```
Default TCP window: 65,535 bytes
Throughput = Window / RTT

Example: 10ms RTT
Throughput = 65535 / 0.01 = 6.5 MB/s (52 Mbps)

Can't use 10 Gbps link!

Solution: TCP Window Scaling
```

```cpp
// Enable in kernel (usually already enabled)
// sysctl -w net.ipv4.tcp_window_scaling=1

// Set large receive buffer
int bufsize = 16 * 1024 * 1024; // 16 MB
setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

/*
 * Now with 16MB window and 10ms RTT:
 * Throughput = 16MB / 0.01s = 1.6 GB/s (12.8 Gbps)
 *
 * Can fully utilize 10 Gbps link!
 */
```

### TCP Congestion Control Algorithms

```bash
# View available algorithms
sysctl net.ipv4.tcp_available_congestion_control

# Set congestion control algorithm
sysctl -w net.ipv4.tcp_congestion_control=bbr

# Or per-socket
int opt = 1;
setsockopt(fd, IPPROTO_TCP, TCP_CONGESTION, "bbr", 3);
```

**Algorithms:**

| Algorithm | Best For | Characteristics |
|-----------|----------|-----------------|
| Reno | Legacy | Conservative, slow recovery |
| Cubic | Default | Aggressive, good for high-BDP |
| BBR | Modern | Bottleneck bandwidth, low latency |
| Vegas | Low latency | Proactive, delay-based |
| Westwood | Wireless | Adapts to wireless conditions |

**BBR (Bottleneck Bandwidth and RTT):**
- Google's algorithm
- Up to 2-25x higher throughput
- Lower latency
- Better for lossy networks

### TCP Fast Open (TFO)

```cpp
// Server: Enable TFO
int qlen = 5;
setsockopt(listen_fd, SOL_TCP, TCP_FASTOPEN, &qlen, sizeof(qlen));

// Client: Send data with SYN
char request[] = "GET / HTTP/1.1\r\n\r\n";
sendto(sockfd, request, sizeof(request), MSG_FASTOPEN,
       (struct sockaddr*)&server_addr, sizeof(server_addr));

/*
 * Traditional TCP handshake:
 * Client → SYN
 * Server → SYN-ACK
 * Client → ACK + Data
 * (1.5 RTT before data transfer)
 *
 * With TFO:
 * Client → SYN + Data
 * Server → SYN-ACK + Response Data
 * (0.5 RTT saved!)
 *
 * Use case:
 * - HTTP requests
 * - DNS queries
 * - Any request-response protocol
 */
```

---

## UDP Optimization

### UDP Socket Options

```cpp
// Increase socket buffers (critical for UDP!)
int bufsize = 16 * 1024 * 1024; // 16 MB
setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));

// Set TTL (time-to-live)
int ttl = 64;
setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

// Set TOS (Type of Service) for QoS
int tos = IPTOS_LOWDELAY; // Minimize delay
setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));

/*
 * TOS values:
 * IPTOS_LOWDELAY     - Minimize delay (VoIP)
 * IPTOS_THROUGHPUT   - Maximize throughput (FTP)
 * IPTOS_RELIABILITY  - Maximize reliability
 * IPTOS_LOWCOST      - Minimize cost
 */
```

### Reliable UDP Implementation

```cpp
// Simple reliability layer over UDP
class ReliableUDP {
private:
    struct Packet {
        uint32_t seq_num;
        uint32_t ack_num;
        uint16_t flags;
        uint16_t length;
        char data[1400];
    };

    std::unordered_map<uint32_t, Packet> send_window;
    uint32_t next_seq = 0;
    uint32_t expected_seq = 0;

public:
    void send_reliable(int sockfd, const char* data, size_t len) {
        Packet pkt{};
        pkt.seq_num = next_seq++;
        pkt.length = len;
        memcpy(pkt.data, data, len);

        // Add to send window
        send_window[pkt.seq_num] = pkt;

        // Send packet
        sendto(sockfd, &pkt, sizeof(pkt), 0, ...);

        // Start retransmission timer
        // (implementation omitted for brevity)
    }

    void on_ack(uint32_t ack_num) {
        // Remove acknowledged packet from window
        send_window.erase(ack_num);
    }

    void on_timeout(uint32_t seq_num) {
        // Retransmit packet
        auto it = send_window.find(seq_num);
        if (it != send_window.end()) {
            sendto(sockfd, &it->second, sizeof(Packet), 0, ...);
        }
    }
};

/*
 * Features to add:
 * - Sequence numbers
 * - ACKs
 * - Timeouts and retransmission
 * - Flow control
 * - Congestion control
 *
 * Libraries that do this:
 * - QUIC (Google)
 * - UDT (UDP-based Data Transfer)
 * - ENet (game networking)
 */
```

### UDP GSO (Generic Segmentation Offload)

```cpp
// Send large UDP packet, NIC splits it
char large_buffer[64000];

// Enable UDP GSO
int val = 1400; // Segment size
setsockopt(sockfd, SOL_UDP, UDP_SEGMENT, &val, sizeof(val));

// Send 64KB in one syscall, NIC segments into 1400-byte packets
send(sockfd, large_buffer, sizeof(large_buffer), 0);

/*
 * Benefits:
 * - Fewer syscalls (better CPU efficiency)
 * - Higher throughput
 * - Available in Linux 4.18+
 */
```

---

## Multicast

### Multicast Basics

```
Unicast:    One-to-One
Broadcast:  One-to-All
Multicast:  One-to-Many (selective)

Multicast IP Range:
224.0.0.0 to 239.255.255.255

Common multicast addresses:
224.0.0.1   - All hosts on subnet
224.0.0.2   - All routers on subnet
224.0.1.1   - NTP (Network Time Protocol)
239.x.x.x   - Organization-local scope
```

### Multicast Sender

```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

// Set multicast TTL
int ttl = 5; // Limit hop count
setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

// Optionally disable loopback
int loop = 0;
setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

// Send to multicast group
struct sockaddr_in addr{};
addr.sin_family = AF_INET;
addr.sin_port = htons(5000);
inet_pton(AF_INET, "239.1.1.1", &addr.sin_addr);

const char* message = "Multicast message";
sendto(sockfd, message, strlen(message), 0,
       (struct sockaddr*)&addr, sizeof(addr));
```

### Multicast Receiver

```cpp
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

// Allow multiple listeners
int reuse = 1;
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

// Bind to multicast port
struct sockaddr_in addr{};
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = INADDR_ANY;
addr.sin_port = htons(5000);
bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));

// Join multicast group
struct ip_mreq mreq{};
inet_pton(AF_INET, "239.1.1.1", &mreq.imr_multiaddr);
mreq.imr_interface.s_addr = INADDR_ANY;
setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

// Receive messages
char buffer[1024];
recvfrom(sockfd, buffer, sizeof(buffer), 0, nullptr, nullptr);

/*
 * Use cases:
 * - Market data distribution
 * - Video streaming
 * - Service discovery
 * - Cluster communication
 *
 * Advantages:
 * - Efficient one-to-many
 * - Network switches replicate packets
 * - Scales better than multiple unicast
 */
```

---

## Custom Protocol Design

### Protocol Design Principles

1. **Minimize overhead**
   - Small headers
   - Binary encoding
   - Compact representation

2. **Fixed-size headers**
   - Faster parsing
   - Predictable performance

3. **Version field**
   - Future compatibility
   - Protocol evolution

4. **Length field**
   - Variable-length messages
   - Framing

5. **Checksum/CRC**
   - Error detection
   - Data integrity

### Example: Simple Binary Protocol

```cpp
// Message format
struct Message {
    uint8_t  version;      // Protocol version
    uint8_t  type;         // Message type
    uint16_t length;       // Payload length (network byte order)
    uint32_t sequence;     // Sequence number (network byte order)
    uint32_t timestamp;    // Timestamp (network byte order)
    uint16_t checksum;     // CRC-16 checksum
    uint8_t  payload[];    // Variable-length payload
} __attribute__((packed));

// Message types
enum MessageType : uint8_t {
    MSG_HEARTBEAT = 1,
    MSG_DATA = 2,
    MSG_ACK = 3,
    MSG_NACK = 4
};

// Send message
void send_message(int sockfd, MessageType type,
                  const void* payload, uint16_t payload_len) {
    size_t total_len = sizeof(Message) + payload_len;
    auto* msg = (Message*)malloc(total_len);

    msg->version = 1;
    msg->type = type;
    msg->length = htons(payload_len);
    msg->sequence = htonl(next_sequence++);
    msg->timestamp = htonl(time(nullptr));

    if (payload && payload_len > 0) {
        memcpy(msg->payload, payload, payload_len);
    }

    // Calculate checksum (CRC-16)
    msg->checksum = calculate_crc16((uint8_t*)msg,
                                     total_len - sizeof(msg->checksum));

    send(sockfd, msg, total_len, 0);
    free(msg);
}

// Receive and parse message
bool receive_message(int sockfd, Message** out_msg) {
    // Read header
    Message header;
    if (recv(sockfd, &header, sizeof(header), MSG_PEEK) < sizeof(header)) {
        return false;
    }

    // Validate version
    if (header.version != 1) {
        return false;
    }

    // Get payload length
    uint16_t payload_len = ntohs(header.length);
    size_t total_len = sizeof(Message) + payload_len;

    // Allocate and read full message
    auto* msg = (Message*)malloc(total_len);
    if (recv(sockfd, msg, total_len, 0) < total_len) {
        free(msg);
        return false;
    }

    // Verify checksum
    uint16_t received_checksum = msg->checksum;
    msg->checksum = 0;
    uint16_t calculated_checksum = calculate_crc16((uint8_t*)msg,
                                                    total_len - sizeof(msg->checksum));

    if (received_checksum != calculated_checksum) {
        free(msg);
        return false;
    }

    *out_msg = msg;
    return true;
}

/*
 * Protocol features:
 * - 14-byte header overhead
 * - Binary encoding (efficient)
 * - Versioning support
 * - Error detection (CRC)
 * - Sequence numbers
 * - Timestamps
 */
```

---

## Message Framing

### Length-Prefixed Framing

```cpp
// Send: [4-byte length][message]
void send_framed(int sockfd, const char* data, uint32_t len) {
    uint32_t net_len = htonl(len);

    // Send length prefix
    send(sockfd, &net_len, sizeof(net_len), 0);

    // Send message
    send(sockfd, data, len, 0);
}

// Receive: read length, then read that many bytes
bool recv_framed(int sockfd, char** out_data, uint32_t* out_len) {
    // Read length prefix
    uint32_t net_len;
    if (recv(sockfd, &net_len, sizeof(net_len), MSG_WAITALL) != sizeof(net_len)) {
        return false;
    }

    uint32_t len = ntohl(net_len);

    // Allocate buffer
    *out_data = (char*)malloc(len);

    // Read message
    if (recv(sockfd, *out_data, len, MSG_WAITALL) != len) {
        free(*out_data);
        return false;
    }

    *out_len = len;
    return true;
}
```

### Delimiter-Based Framing

```cpp
// Send: [message][delimiter]
void send_delimited(int sockfd, const char* data, size_t len) {
    send(sockfd, data, len, 0);
    send(sockfd, "\n", 1, 0); // Newline delimiter
}

// Receive: read until delimiter
bool recv_delimited(int sockfd, std::string& message) {
    char buf[1];
    message.clear();

    while (true) {
        if (recv(sockfd, buf, 1, 0) != 1) {
            return false;
        }

        if (buf[0] == '\n') {
            break;
        }

        message += buf[0];
    }

    return true;
}

/*
 * Pros:
 * - Simple
 * - Human-readable debugging
 *
 * Cons:
 * - Slow (byte-by-byte reading)
 * - Escaping required if delimiter in data
 * - Not suitable for binary data
 */
```

---

## Serialization

### Serialization Methods Comparison

| Method | Size | Speed | Schema | Language | Use Case |
|--------|------|-------|--------|----------|----------|
| JSON | Large | Slow | No | All | APIs, config |
| Protocol Buffers | Small | Fast | Yes | Many | RPC, storage |
| FlatBuffers | Small | Fastest | Yes | Many | Gaming, HFT |
| Cap'n Proto | Small | Fastest | Yes | Many | RPC |
| MessagePack | Medium | Fast | No | Many | General |
| Raw Binary | Smallest | Fastest | No | C/C++ | Ultra-low latency |

### Protocol Buffers Example

```protobuf
// Define schema (message.proto)
syntax = "proto3";

message MarketData {
    string symbol = 1;
    double price = 2;
    int64 volume = 3;
    int64 timestamp = 4;
}
```

```cpp
// Usage in C++
#include "message.pb.h"

// Serialize
MarketData data;
data.set_symbol("AAPL");
data.set_price(150.25);
data.set_volume(1000000);
data.set_timestamp(time(nullptr));

std::string serialized;
data.SerializeToString(&serialized);

// Send
send(sockfd, serialized.data(), serialized.size(), 0);

// Deserialize
MarketData received;
received.ParseFromString(serialized);

std::cout << "Symbol: " << received.symbol() << "\n";
std::cout << "Price: " << received.price() << "\n";
```

### FlatBuffers Example

```fbs
// Schema (market_data.fbs)
namespace Market;

table MarketData {
    symbol: string;
    price: double;
    volume: long;
    timestamp: long;
}

root_type MarketData;
```

```cpp
#include "market_data_generated.h"

// Build message
flatbuffers::FlatBufferBuilder builder(1024);

auto symbol = builder.CreateString("AAPL");

auto market_data = Market::CreateMarketData(builder,
    symbol,
    150.25,    // price
    1000000,   // volume
    time(nullptr)
);

builder.Finish(market_data);

// Get buffer (zero-copy access!)
uint8_t *buf = builder.GetBufferPointer();
int size = builder.GetSize();

// Send
send(sockfd, buf, size, 0);

// Deserialize (zero-copy!)
auto received = Market::GetMarketData(buf);
std::cout << "Symbol: " << received->symbol()->str() << "\n";
std::cout << "Price: " << received->price() << "\n";

/*
 * FlatBuffers advantages:
 * - Zero-copy deserialization
 * - No parsing needed
 * - Direct memory access
 * - Perfect for low-latency
 *
 * Used by: Google, Facebook, Unreal Engine
 */
```

### Custom Binary Serialization

```cpp
// Fixed-size struct (fastest!)
struct __attribute__((packed)) MarketData {
    char symbol[8];      // Fixed 8 chars
    double price;        // 8 bytes
    uint64_t volume;     // 8 bytes
    uint64_t timestamp;  // 8 bytes
    // Total: 32 bytes

    // Endianness handling for portability
    void to_network_order() {
        price = htobe64(*(uint64_t*)&price);
        volume = htobe64(volume);
        timestamp = htobe64(timestamp);
    }

    void to_host_order() {
        *(uint64_t*)&price = be64toh(*(uint64_t*)&price);
        volume = be64toh(volume);
        timestamp = be64toh(timestamp);
    }
};

// Send (32 bytes, no overhead!)
MarketData data{};
strncpy(data.symbol, "AAPL", 8);
data.price = 150.25;
data.volume = 1000000;
data.timestamp = time(nullptr);
data.to_network_order();

send(sockfd, &data, sizeof(data), 0);

// Receive
MarketData received;
recv(sockfd, &received, sizeof(received), MSG_WAITALL);
received.to_host_order();

/*
 * Pros:
 * - Absolute minimum latency
 * - No parsing/serialization overhead
 * - Cache-friendly
 * - 32 bytes total
 *
 * Cons:
 * - Not extensible
 * - Language-specific
 * - Alignment issues
 *
 * Perfect for: HFT, ultra-low latency
 */
```

---

## Performance Comparison

### Serialization Benchmark

```
Message: Symbol (8 chars), Price, Volume, Timestamp

Format          | Size (bytes) | Serialize (ns) | Deserialize (ns) |
----------------|--------------|----------------|------------------|
JSON            | 95           | 1,200          | 800              |
Protocol Buffers| 35           | 150            | 100              |
FlatBuffers     | 48           | 80             | 5  (zero-copy!)  |
Cap'n Proto     | 48           | 50             | 0  (zero-copy!)  |
Raw Binary      | 32           | 10             | 10               |

For 1 million messages:
- JSON: 95 MB, 2 seconds
- Protocol Buffers: 35 MB, 250 ms
- Raw Binary: 32 MB, 20 ms  (100x faster!)
```

---

## Key Takeaways

1. **TCP_NODELAY is critical for latency**
   - Disables Nagle's algorithm
   - 40-200ms → <1ms

2. **TCP_QUICKACK reduces ACK delay**
   - Must be reset after each recv()

3. **UDP needs large buffers**
   - Prevent packet drops
   - 16MB+ recommended

4. **Multicast is efficient for one-to-many**
   - Market data feeds
   - Cluster communication

5. **Design protocols for your use case**
   - Binary for performance
   - Fixed-size for predictability

6. **Serialization choice matters**
   - Raw binary: fastest
   - FlatBuffers: zero-copy
   - Protocol Buffers: good balance

7. **Measure your specific workload**
   - Different protocols suit different needs

---

## Next Steps

Continue to [Module 6: AWS Networking for Developers](../module-06-aws-networking/README.md).

## Additional Resources

- RFC 793: TCP
- RFC 768: UDP
- Protocol Buffers: https://protobuf.dev/
- FlatBuffers: https://google.github.io/flatbuffers/
- "High Performance Browser Networking" by Ilya Grigorik

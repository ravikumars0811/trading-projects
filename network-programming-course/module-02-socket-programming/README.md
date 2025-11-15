# Module 2: Socket Programming in C++

## Overview

This module covers socket programming using the BSD Sockets API in C++. You'll learn to build both TCP and UDP client/server applications with proper error handling and performance optimization.

## Table of Contents

1. [Socket Basics](#socket-basics)
2. [TCP Socket Programming](#tcp-socket-programming)
3. [UDP Socket Programming](#udp-socket-programming)
4. [Socket Options](#socket-options)
5. [Error Handling](#error-handling)
6. [Performance Tuning](#performance-tuning)

---

## Socket Basics

### What is a Socket?

A socket is an endpoint for sending or receiving data across a network. It's the fundamental abstraction for network communication in Unix-like systems.

### Socket Types

```cpp
// SOCK_STREAM: TCP - connection-oriented, reliable, byte stream
int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

// SOCK_DGRAM: UDP - connectionless, unreliable, datagrams
int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

// SOCK_RAW: Raw IP packets (requires root)
int raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
```

### Socket Address Structures

```cpp
// IPv4 address structure
struct sockaddr_in {
    sa_family_t    sin_family;  // Address family (AF_INET)
    in_port_t      sin_port;    // Port number (network byte order)
    struct in_addr sin_addr;    // IP address
    char           sin_zero[8]; // Padding
};

// Generic socket address
struct sockaddr {
    sa_family_t sa_family;      // Address family
    char        sa_data[14];    // Address data
};

// IPv6 address structure
struct sockaddr_in6 {
    sa_family_t     sin6_family;   // AF_INET6
    in_port_t       sin6_port;     // Port number
    uint32_t        sin6_flowinfo; // IPv6 flow information
    struct in6_addr sin6_addr;     // IPv6 address
    uint32_t        sin6_scope_id; // Scope ID
};
```

---

## TCP Socket Programming

### TCP Client/Server Flow

```
Server Side                          Client Side
────────────────                    ────────────────
socket()                            socket()
   ↓                                   ↓
bind()                              [optional bind()]
   ↓                                   ↓
listen()                            connect() ────────────┐
   ↓                                   ↓                  │
accept() ←──────────────────────── (SYN handshake)       │
   ↓                                   ↓                  │
recv()/send() ←──────────────────→ send()/recv()         │
   ↓                                   ↓                  │
close()                             close()               │
                                                          │
Legend:                                                   │
→ : Data flow                                            │
└─: Connection establishment                             │
```

### TCP Server Implementation

See [examples/tcp_server.cpp](./examples/tcp_server.cpp)

### TCP Client Implementation

See [examples/tcp_client.cpp](./examples/tcp_client.cpp)

---

## UDP Socket Programming

### UDP Client/Server Flow

```
Server Side                          Client Side
────────────────                    ────────────────
socket()                            socket()
   ↓                                   ↓
bind()                              [optional bind()]
   ↓                                   ↓
recvfrom() ←──────────────────────  sendto()
   ↓                                   ↓
sendto() ──────────────────────────→ recvfrom()
   ↓                                   ↓
close()                             close()
```

### UDP Server Implementation

See [examples/udp_server.cpp](./examples/udp_server.cpp)

### UDP Client Implementation

See [examples/udp_client.cpp](./examples/udp_client.cpp)

---

## Socket Options

### Common Socket Options

```cpp
// SO_REUSEADDR: Allow reuse of local addresses
int opt = 1;
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

// SO_REUSEPORT: Allow multiple sockets to bind to same port
setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

// SO_RCVBUF: Set receive buffer size
int bufsize = 1024 * 1024; // 1MB
setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

// SO_SNDBUF: Set send buffer size
setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));

// SO_KEEPALIVE: Enable TCP keepalive
setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

// SO_LINGER: Control socket close behavior
struct linger lin;
lin.l_onoff = 1;
lin.l_linger = 0; // Immediate close, discard data
setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin));
```

### TCP-Specific Options

```cpp
// TCP_NODELAY: Disable Nagle's algorithm (critical for low latency!)
int flag = 1;
setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

// TCP_QUICKACK: Disable delayed ACKs
setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag));

// TCP_CORK: Don't send partial frames
setsockopt(sockfd, IPPROTO_TCP, TCP_CORK, &flag, sizeof(flag));

// TCP_KEEPIDLE: Time before sending keepalive probes
int keepidle = 60; // seconds
setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));

// TCP_KEEPINTVL: Interval between keepalive probes
int keepintvl = 10;
setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));

// TCP_KEEPCNT: Number of keepalive probes
int keepcnt = 3;
setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
```

### Socket Options Impact on Performance

| Option | Impact | Use Case |
|--------|--------|----------|
| TCP_NODELAY | -50% latency | Low-latency apps (HFT, gaming) |
| TCP_QUICKACK | -20% latency | Interactive protocols |
| SO_RCVBUF/SO_SNDBUF | +50% throughput | Bulk transfers |
| SO_REUSEPORT | +Nx throughput | Multi-threaded servers |
| TCP_CORK | +10% throughput | Batch sending |

---

## Error Handling

### Proper Error Checking

```cpp
#include <errno.h>
#include <string.h>

// Always check return values!
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd < 0) {
    perror("socket");
    // or
    fprintf(stderr, "socket failed: %s\n", strerror(errno));
    return -1;
}

// Check bind
if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    close(sockfd);
    return -1;
}

// Check recv/send
ssize_t n = recv(sockfd, buffer, sizeof(buffer), 0);
if (n < 0) {
    if (errno == EINTR) {
        // Interrupted by signal, retry
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // Non-blocking socket, no data available
    } else {
        perror("recv");
        close(sockfd);
        return -1;
    }
} else if (n == 0) {
    // Connection closed by peer
    printf("Connection closed\n");
    close(sockfd);
    return 0;
}
```

### Common Errno Values

| errno | Meaning | Action |
|-------|---------|--------|
| EINTR | Interrupted system call | Retry |
| EAGAIN/EWOULDBLOCK | Resource temporarily unavailable | Retry later |
| ECONNRESET | Connection reset by peer | Close socket |
| EPIPE | Broken pipe | Close socket |
| ETIMEDOUT | Connection timed out | Reconnect |
| EADDRINUSE | Address already in use | Use SO_REUSEADDR |

---

## Performance Tuning

### 1. Buffer Sizes

```cpp
// Increase socket buffers for high-throughput applications
int sndbuf = 4 * 1024 * 1024; // 4MB send buffer
int rcvbuf = 4 * 1024 * 1024; // 4MB receive buffer

setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

// Verify the actual buffer size (kernel may limit)
socklen_t len = sizeof(sndbuf);
getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, &len);
printf("Actual send buffer: %d bytes\n", sndbuf);
```

### 2. Nagle's Algorithm

```cpp
// Disable Nagle's algorithm for low-latency
// Nagle combines small packets - adds latency!
int flag = 1;
setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

// Impact:
// With Nagle:    send(10 bytes) → wait → send(10 bytes) → wait
//                Latency: ~40ms (typical delayed ACK timeout)
// Without Nagle: send(10 bytes) → immediate → send(10 bytes) → immediate
//                Latency: <1ms
```

### 3. Delayed ACK

```cpp
// Disable delayed ACKs for interactive applications
int flag = 1;
setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag));

// Note: TCP_QUICKACK is not persistent, must be set after each recv()
while (1) {
    recv(sockfd, buf, sizeof(buf), 0);
    setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag));
    // process data
}
```

### 4. Zero-Copy with sendfile()

```cpp
#include <sys/sendfile.h>

// Send file efficiently without copying to userspace
int filefd = open("data.bin", O_RDONLY);
off_t offset = 0;
size_t count = file_size;

// Zero-copy transfer
ssize_t sent = sendfile(sockfd, filefd, &offset, count);

// Traditional approach (2 copies):
// read(filefd, buf, size) → kernel to userspace
// send(sockfd, buf, size) → userspace to kernel
//
// sendfile() approach (0 copies):
// sendfile() → kernel to kernel (DMA)
```

### 5. MSG_ZEROCOPY

```cpp
// Enable zero-copy transmission (Linux 4.14+)
int opt = 1;
setsockopt(sockfd, SOL_SOCKET, SO_ZEROCOPY, &opt, sizeof(opt));

// Send with zero-copy
char buffer[8192];
ssize_t sent = send(sockfd, buffer, sizeof(buffer), MSG_ZEROCOPY);

// Receive completion notification
struct msghdr msg = {};
recvmsg(sockfd, &msg, MSG_ERRQUEUE);
```

---

## Practical Examples

### Example 1: Echo Server (TCP)

See [examples/tcp_echo_server.cpp](./examples/tcp_echo_server.cpp)

**Features:**
- Accepts multiple clients (sequential)
- Echoes back received data
- Proper error handling

### Example 2: Time Server (UDP)

See [examples/udp_time_server.cpp](./examples/udp_time_server.cpp)

**Features:**
- Sends current time to clients
- Connectionless operation
- Minimal latency

### Example 3: High-Performance TCP Server

See [examples/optimized_tcp_server.cpp](./examples/optimized_tcp_server.cpp)

**Optimizations:**
- TCP_NODELAY enabled
- TCP_QUICKACK enabled
- Large socket buffers
- SO_REUSEPORT for load balancing

---

## Byte Order (Endianness)

### Network Byte Order

```cpp
#include <arpa/inet.h>

// Network byte order is BIG ENDIAN
// Host byte order may be little or big endian

// Host to Network (short - 16 bit)
uint16_t port = 8080;
uint16_t net_port = htons(port);

// Host to Network (long - 32 bit)
uint32_t addr = 0x7F000001; // 127.0.0.1
uint32_t net_addr = htonl(addr);

// Network to Host (short)
uint16_t host_port = ntohs(net_port);

// Network to Host (long)
uint32_t host_addr = ntohl(net_addr);

// Convert IP address string to network format
struct in_addr ip_addr;
inet_pton(AF_INET, "192.168.1.1", &ip_addr);

// Convert network format to string
char ip_str[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &ip_addr, ip_str, sizeof(ip_str));
```

### Endianness Example

```
Value: 0x12345678

Big Endian (Network):    [12] [34] [56] [78]
                          ↑
                        MSB (Most Significant Byte first)

Little Endian (x86):     [78] [56] [34] [12]
                          ↑
                        LSB (Least Significant Byte first)
```

---

## Non-Blocking Sockets

```cpp
#include <fcntl.h>

// Make socket non-blocking
int flags = fcntl(sockfd, F_GETFL, 0);
fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

// Now send/recv return immediately
ssize_t n = recv(sockfd, buf, sizeof(buf), 0);
if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // No data available, try later
    } else {
        perror("recv");
    }
}

// Use with select/poll/epoll for efficient I/O multiplexing
// (covered in Module 3)
```

---

## Complete Example: Modern C++ TCP Server

See [examples/modern_tcp_server.cpp](./examples/modern_tcp_server.cpp)

**Features:**
- RAII wrapper for sockets
- Exception-based error handling
- Modern C++17 features
- Thread-safe logging

---

## Exercises

### Exercise 1: Echo Server
Implement a TCP echo server that:
- Accepts connections on port 8080
- Echoes back all received data
- Handles multiple clients sequentially
- Implements proper error handling

### Exercise 2: Chat Application
Build a simple UDP-based chat application:
- Client sends messages to server
- Server broadcasts to all connected clients
- Use recvfrom/sendto
- Handle client list management

### Exercise 3: File Transfer
Create a TCP file transfer application:
- Client sends filename
- Server sends file contents
- Implement progress reporting
- Use sendfile() for efficiency

### Exercise 4: Performance Benchmark
Write a benchmark to measure:
- TCP vs UDP latency
- Impact of TCP_NODELAY
- Impact of buffer sizes
- Throughput vs message size

### Exercise 5: Port Scanner
Implement a simple port scanner:
- Scan range of ports on target host
- Use connect() with timeout
- Report open/closed ports
- Implement concurrent scanning

---

## Common Pitfalls

1. **Forgetting byte order conversion**
   ```cpp
   // WRONG
   addr.sin_port = 8080;

   // CORRECT
   addr.sin_port = htons(8080);
   ```

2. **Not checking return values**
   ```cpp
   // WRONG
   send(sockfd, buf, len, 0);

   // CORRECT
   ssize_t sent = send(sockfd, buf, len, 0);
   if (sent < 0) {
       perror("send");
   }
   ```

3. **Assuming send/recv transfers all data**
   ```cpp
   // WRONG - may not send all data
   send(sockfd, buf, 1000, 0);

   // CORRECT - loop until all sent
   size_t total_sent = 0;
   while (total_sent < len) {
       ssize_t sent = send(sockfd, buf + total_sent,
                          len - total_sent, 0);
       if (sent < 0) {
           perror("send");
           break;
       }
       total_sent += sent;
   }
   ```

4. **Not handling EINTR**
   ```cpp
   // recv() can be interrupted by signals
   ssize_t n;
   do {
       n = recv(sockfd, buf, sizeof(buf), 0);
   } while (n < 0 && errno == EINTR);
   ```

5. **Binding to wrong interface**
   ```cpp
   // Listen on all interfaces
   addr.sin_addr.s_addr = INADDR_ANY;

   // Listen on specific interface
   inet_pton(AF_INET, "192.168.1.100", &addr.sin_addr);

   // Loopback only
   addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   ```

---

## Key Takeaways

1. **Always check return values** - Network operations can fail!
2. **Use TCP_NODELAY** for low-latency applications
3. **Handle partial send/recv** - They may not transfer all data
4. **Remember byte order** - Use htons/htonl/ntohs/ntohl
5. **Set appropriate buffer sizes** for your workload
6. **Non-blocking sockets** require careful error handling
7. **SO_REUSEADDR** prevents "Address already in use" errors
8. **Close sockets properly** to free resources

---

## Next Steps

Continue to [Module 3: Advanced I/O Models](../module-03-advanced-io/README.md) to learn about epoll, io_uring, and asynchronous I/O.

## Additional Resources

- `man 2 socket` - Socket system call
- `man 7 socket` - Socket options
- `man 7 tcp` - TCP protocol
- `man 7 udp` - UDP protocol
- "Unix Network Programming, Volume 1" by W. Richard Stevens

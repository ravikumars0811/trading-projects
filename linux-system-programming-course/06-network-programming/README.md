# Module 6: Network Programming

## 📖 Overview

Network programming enables applications to communicate over networks. This module covers socket programming, TCP/UDP protocols, and building client-server applications.

## 🎯 Learning Objectives

- Understand socket programming fundamentals
- Create TCP and UDP clients and servers
- Handle multiple connections
- Implement non-blocking I/O
- Build real-world network applications

## 📚 Socket Programming Basics

### Socket Creation

```c
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

**Domain:**
- `AF_INET` - IPv4
- `AF_INET6` - IPv6
- `AF_UNIX` - Unix domain sockets

**Type:**
- `SOCK_STREAM` - TCP (reliable, connection-oriented)
- `SOCK_DGRAM` - UDP (unreliable, connectionless)

### TCP Server Steps

```c
1. socket()      // Create socket
2. bind()        // Bind to address/port
3. listen()      // Listen for connections
4. accept()      // Accept client connection
5. send/recv()   // Exchange data
6. close()       // Close connection
```

### TCP Client Steps

```c
1. socket()      // Create socket
2. connect()     // Connect to server
3. send/recv()   // Exchange data
4. close()       // Close connection
```

## 💻 Code Examples

### TCP Socket Programming

- [tcp_server.c](./examples/tcp_server.c) - Simple TCP echo server
- [tcp_client.c](./examples/tcp_client.c) - TCP client
- [multi_client_server.c](./examples/multi_client_server.c) - Multi-client server

### UDP Socket Programming

- [udp_server.c](./examples/udp_server.c) - UDP server
- [udp_client.c](./examples/udp_client.c) - UDP client

### Real-World Examples

- [http_server.c](./examples/http_server.c) - Simple HTTP server
- [chat_server.c](./examples/chat_server.c) - Multi-user chat server

## 🔧 Key Functions

### Server Functions
```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

### Client Functions
```c
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

### Data Transfer
```c
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
```

## 🎓 Key Concepts

### TCP vs UDP

| Feature | TCP | UDP |
|---------|-----|-----|
| Connection | Connection-oriented | Connectionless |
| Reliability | Reliable | Unreliable |
| Ordering | Ordered | Unordered |
| Speed | Slower | Faster |
| Use Case | Web, Email, FTP | DNS, Video streaming |

### Port Numbers
- 0-1023: Well-known ports (HTTP=80, HTTPS=443, SSH=22)
- 1024-49151: Registered ports
- 49152-65535: Dynamic/private ports

### Byte Order
- Use `htons()`, `htonl()` for network byte order (big-endian)
- Use `ntohs()`, `ntohl()` for host byte order

## 📝 Compilation

```bash
gcc tcp_server.c -o tcp_server
gcc tcp_client.c -o tcp_client
gcc multi_client_server.c -o multi_client_server -lpthread
```

## ➡️ Next Module

Continue to [Module 7: Linux Commands for Developers](../07-linux-commands/README.md)

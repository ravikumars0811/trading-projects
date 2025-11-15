# Module 3: Advanced I/O Models

## Overview

This module covers advanced I/O multiplexing techniques essential for building scalable, high-performance network applications. You'll learn about select, poll, epoll, and the modern io_uring interface.

## Table of Contents

1. [I/O Models Overview](#io-models-overview)
2. [select()](#select)
3. [poll()](#poll)
4. [epoll - Linux](#epoll---linux)
5. [io_uring - Modern Linux](#io_uring---modern-linux)
6. [Event-Driven Architecture](#event-driven-architecture)
7. [Performance Comparison](#performance-comparison)

---

## I/O Models Overview

### Five I/O Models in Unix

```
1. Blocking I/O
┌─────────┐              ┌────────┐
│  App    │ ─── read ──→ │ Kernel │
│         │   (blocks)   │        │
│  (wait) │ ←── data ─── │        │
└─────────┘              └────────┘

2. Non-Blocking I/O
┌─────────┐              ┌────────┐
│  App    │ ─── read ──→ │ Kernel │
│         │ ←── EAGAIN ── │        │
│  (poll) │ ─── read ──→ │        │
│         │ ←── EAGAIN ── │        │
│  (poll) │ ─── read ──→ │        │
│         │ ←── data ─── │        │
└─────────┘              └────────┘
(CPU intensive polling!)

3. I/O Multiplexing (select/poll/epoll)
┌─────────┐              ┌────────┐
│  App    │ ─── select ─→│ Kernel │
│         │   (blocks)   │        │
│  (wait) │ ←── ready ─── │        │
│         │ ─── read ──→ │        │
│         │ ←── data ─── │        │
└─────────┘              └────────┘

4. Signal-Driven I/O
┌─────────┐              ┌────────┐
│  App    │ ─── setup ─→ │ Kernel │
│ (async) │              │        │
│         │ ←── SIGIO ─── │ (data) │
│         │ ─── read ──→ │        │
│         │ ←── data ─── │        │
└─────────┘              └────────┘

5. Asynchronous I/O (io_uring, Windows IOCP)
┌─────────┐              ┌────────┐
│  App    │ ─── aio_read │ Kernel │
│ (async) │              │        │
│         │              │ (DMA)  │
│         │ ←── done ──── │        │
└─────────┘              └────────┘
(True async - data copied by kernel)
```

---

## select()

### Overview

`select()` monitors multiple file descriptors to see if they're ready for I/O operations.

**Limitations:**
- Max 1024 file descriptors (FD_SETSIZE)
- O(n) complexity
- Requires copying fd_set on each call
- Not suitable for high-performance servers

### select() Prototype

```cpp
#include <sys/select.h>

int select(int nfds,
           fd_set *readfds,
           fd_set *writefds,
           fd_set *exceptfds,
           struct timeval *timeout);

// Manipulate fd_set
FD_ZERO(&set);        // Clear all bits
FD_SET(fd, &set);     // Add fd to set
FD_CLR(fd, &set);     // Remove fd from set
FD_ISSET(fd, &set);   // Test if fd is in set
```

### select() Example

See [examples/select_server.cpp](./examples/select_server.cpp)

### select() Visualization

```
Before select():
readfds = {0, 3, 4, 5}  (stdin, 3 sockets)
         ┌─┬─┬─┬─┬─┬─┬─┬─┐
Bits:    │1│0│0│1│1│1│0│0│ ...
         └─┴─┴─┴─┴─┴─┴─┴─┘
          0 1 2 3 4 5 6 7

After select():
readfds = {3, 5}  (fds 3 and 5 have data)
         ┌─┬─┬─┬─┬─┬─┬─┬─┐
Bits:    │0│0│0│1│0│1│0│0│ ...
         └─┴─┴─┴─┴─┴─┴─┴─┘
          0 1 2 3 4 5 6 7

Must rebuild fd_set for next call!
```

---

## poll()

### Overview

`poll()` is similar to select() but uses an array of structures instead of bitmasks.

**Advantages over select():**
- No maximum file descriptor limit
- Cleaner API
- More efficient for sparse file descriptor sets

**Limitations:**
- Still O(n) complexity
- Still need to scan all fds

### poll() Prototype

```cpp
#include <poll.h>

struct pollfd {
    int   fd;         // File descriptor
    short events;     // Requested events
    short revents;    // Returned events
};

int poll(struct pollfd *fds,
         nfds_t nfds,
         int timeout);

// Event flags
POLLIN    - Data to read
POLLOUT   - Ready for writing
POLLERR   - Error condition
POLLHUP   - Hang up
POLLNVAL  - Invalid request
```

### poll() Example

See [examples/poll_server.cpp](./examples/poll_server.cpp)

### poll() vs select()

| Feature | select() | poll() |
|---------|----------|--------|
| Max FDs | 1024 (FD_SETSIZE) | No limit |
| API | Bitmasks | Array of structs |
| Complexity | O(n) | O(n) |
| State | Must rebuild | Preserved |
| Portability | POSIX, portable | POSIX, portable |

---

## epoll - Linux

### Overview

`epoll` is Linux's high-performance I/O multiplexing mechanism.

**Key Advantages:**
- O(1) complexity for add/remove/modify
- O(k) complexity for wait (k = ready fds)
- Edge-triggered mode available
- Scales to tens of thousands of connections

**Perfect for:**
- Web servers
- Proxy servers
- High-frequency trading
- Game servers

### epoll API

```cpp
#include <sys/epoll.h>

// Create epoll instance
int epoll_create1(int flags);

// Control interface
int epoll_ctl(int epfd,
              int op,      // EPOLL_CTL_ADD/MOD/DEL
              int fd,
              struct epoll_event *event);

// Wait for events
int epoll_wait(int epfd,
               struct epoll_event *events,
               int maxevents,
               int timeout);

struct epoll_event {
    uint32_t events;   // Events (EPOLLIN, EPOLLOUT, etc.)
    epoll_data_t data; // User data
};

typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

// Event flags
EPOLLIN      - Read ready
EPOLLOUT     - Write ready
EPOLLET      - Edge-triggered mode
EPOLLONESHOT - One-shot mode
EPOLLRDHUP   - Peer closed connection
```

### Edge-Triggered vs Level-Triggered

```
Level-Triggered (default):
─────────────────────────────
Data arrives: ████████
epoll_wait:   ████████  (returns while data available)
epoll_wait:   ████████  (returns again!)
epoll_wait:   ████████  (keeps returning)
read():       ────────  (must read all data)

Edge-Triggered (EPOLLET):
─────────────────────────────
Data arrives: █───────
epoll_wait:   █───────  (returns once)
epoll_wait:   blocked   (won't return until new data)
Must read ALL available data in loop!

More data:    █───────
epoll_wait:   █───────  (returns again)
```

### epoll Example

See [examples/epoll_server.cpp](./examples/epoll_server.cpp)

### epoll Architecture

```
Application                     Kernel

epoll_create1()  ──→  ┌─────────────────┐
                      │  epoll instance │
                      │   (red-black    │
                      │      tree)      │
epoll_ctl(ADD)   ──→  │                 │
                      │  fd1 → events   │
epoll_ctl(ADD)   ──→  │  fd2 → events   │
                      │  fd3 → events   │
                      └─────────────────┘
                               ↓
                      ┌─────────────────┐
epoll_wait()     ←──  │  Ready list     │
                      │  (only ready    │
                      │   fds queued)   │
                      └─────────────────┘

Complexity:
- epoll_ctl:  O(log n)  (red-black tree)
- epoll_wait: O(k)      (k = ready fds)

vs select/poll:
- scan all:   O(n)      (must check all fds)
```

---

## io_uring - Modern Linux

### Overview

`io_uring` is the modern asynchronous I/O interface in Linux (kernel 5.1+).

**Revolutionary Features:**
- True async I/O (not just network, ALL I/O!)
- Zero system calls possible
- Shared memory ring buffers
- Batching support
- Lower latency than epoll
- Supports all file operations

### io_uring Architecture

```
Application              Kernel
───────────              ──────

Submission Queue (SQE)   Completion Queue (CQE)
┌──────────────┐        ┌──────────────┐
│  SQE │ SQE  │        │  CQE │ CQE  │
│  SQE │ SQE  │        │  CQE │      │
│      │      │        │      │      │
└──────┬───────┘        └──────▲─────┘
       │                       │
       │  Shared Memory        │
       │  (mmap)              │
       │                       │
       └───────────┬───────────┘
                   │
            ┌──────▼──────┐
            │   Kernel    │
            │  io_uring   │
            │   Worker    │
            └─────────────┘

Process:
1. App writes SQE to submission queue
2. App calls io_uring_enter() [or none if polling!]
3. Kernel processes requests
4. Kernel writes CQE to completion queue
5. App reads results

Zero syscall mode:
- Kernel polls submission queue
- App polls completion queue
- NO context switches!
```

### io_uring API

```cpp
#include <liburing.h>

struct io_uring ring;

// Initialize
io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

// Submit a read operation
struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
io_uring_prep_read(sqe, fd, buffer, size, offset);
io_uring_sqe_set_data(sqe, user_data);
io_uring_submit(&ring);

// Wait for completion
struct io_uring_cqe *cqe;
io_uring_wait_cqe(&ring, &cqe);
int result = cqe->res;
void *user_data = io_uring_cqe_get_data(cqe);
io_uring_cqe_seen(&ring, cqe);

// Cleanup
io_uring_queue_exit(&ring);
```

### io_uring Example

See [examples/io_uring_server.cpp](./examples/io_uring_server.cpp)

### io_uring Operations Supported

```cpp
// Network I/O
io_uring_prep_recv()
io_uring_prep_send()
io_uring_prep_accept()
io_uring_prep_connect()

// File I/O
io_uring_prep_read()
io_uring_prep_write()
io_uring_prep_readv()    // scatter-gather
io_uring_prep_writev()

// Advanced
io_uring_prep_splice()   // zero-copy pipe
io_uring_prep_fsync()    // file sync
io_uring_prep_fallocate()
io_uring_prep_openat()   // async open!
io_uring_prep_close()    // async close!
io_uring_prep_timeout()  // async timer

// Can even submit batches!
io_uring_submit_and_wait()
```

---

## Event-Driven Architecture

### Reactor Pattern

```
┌─────────────────────────────────────────┐
│           Event Loop                    │
│                                         │
│  while (running) {                      │
│      events = epoll_wait()              │
│      for each event:                    │
│          if (event == ACCEPT)           │
│              handle_accept()            │
│          else if (event == READ)        │
│              handle_read()              │
│          else if (event == WRITE)       │
│              handle_write()             │
│  }                                      │
└─────────────────────────────────────────┘

Handler Registration:
┌──────────┐
│  Socket  │ ──register──→ ┌──────────────┐
│  Event   │               │ Event Loop   │
└──────────┘               │  (epoll/     │
                           │  io_uring)   │
┌──────────┐               └──────────────┘
│ Handler  │ ──callback──→       ↑
│ Function │                     │
└──────────┘              dispatch event
                                 │
                          ┌──────▼──────┐
                          │   Handler   │
                          │  Executes   │
                          └─────────────┘
```

### Proactor Pattern (io_uring)

```
Application initiates async operations:

┌──────────────┐
│   App        │
│              │
│ async_read() │ ──┐
│ async_write()│   │  Submit operations
│ async_accept()   │
└──────────────┘   │
                   ▼
           ┌──────────────┐
           │  io_uring    │
           │  (Proactor)  │
           └──────┬───────┘
                  │
                  │ Operations complete
                  │
                  ▼
          ┌──────────────┐
          │  Completion  │
          │   Handler    │
          └──────────────┘

Key difference from Reactor:
- Reactor: notified when ready to read → app reads
- Proactor: app requests read → notified when done
```

---

## Performance Comparison

### Scalability

```
Connections vs CPU Usage

select/poll:                  epoll:
CPU │                         CPU │
    │        ╱                    │
    │      ╱                      │    ──────
    │    ╱                        │ ──
    │  ╱                          │─
    │╱                            │
    └─────── Connections          └─────── Connections
    O(n) - linear growth          O(1) - constant time


io_uring:
CPU │
    │           (even lower with polling mode)
    │ ─
    │
    │
    │
    └─────── Connections
    O(1) + zero syscalls
```

### Benchmark Results (Typical)

```
Echo Server - 10,000 concurrent connections
1 KB messages, localhost

Method      │ Throughput    │ Latency (μs)  │ CPU Usage
────────────┼───────────────┼───────────────┼──────────
select()    │ 50K msg/s     │ 200           │ 100%
poll()      │ 60K msg/s     │ 167           │ 100%
epoll (LT)  │ 450K msg/s    │ 22            │ 60%
epoll (ET)  │ 500K msg/s    │ 20            │ 55%
io_uring    │ 750K msg/s    │ 13            │ 40%
io_uring*   │ 1.2M msg/s    │ 8             │ 100%

* with IORING_SETUP_SQPOLL (kernel polling)
```

### When to Use Each

| Use Case | Best Choice | Reason |
|----------|-------------|--------|
| <100 connections | select/poll | Simple, portable |
| 100-10K connections | epoll | Good performance, mature |
| >10K connections | epoll/io_uring | Scalability |
| Ultra-low latency | io_uring + polling | Lowest overhead |
| Mixed I/O (file+net) | io_uring | Unified interface |
| Cross-platform | select/poll | Available everywhere |
| Linux-only, modern | io_uring | Best performance |

---

## Practical Guidelines

### Error Handling Best Practices

```cpp
// Always handle EINTR
while (true) {
    int nready = epoll_wait(epfd, events, MAX_EVENTS, -1);
    if (nready < 0) {
        if (errno == EINTR) {
            continue; // Interrupted by signal, retry
        }
        perror("epoll_wait");
        break;
    }
    // Process events
}

// Handle EAGAIN in edge-triggered mode
while (true) {
    ssize_t n = read(fd, buf, sizeof(buf));
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break; // No more data available
        }
        if (errno == EINTR) {
            continue; // Interrupted, retry
        }
        perror("read");
        return -1;
    }
    if (n == 0) {
        // EOF
        break;
    }
    // Process data
}
```

### Edge-Triggered epoll Recipe

```cpp
// 1. Make socket non-blocking
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);

// 2. Add to epoll with EPOLLET
struct epoll_event ev;
ev.events = EPOLLIN | EPOLLET;  // Edge-triggered
ev.data.fd = fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);

// 3. Read ALL data when notified
void handle_read(int fd) {
    while (true) {
        char buf[4096];
        ssize_t n = read(fd, buf, sizeof(buf));

        if (n < 0) {
            if (errno == EAGAIN) {
                break; // Done reading
            }
            perror("read");
            return;
        }
        if (n == 0) {
            close(fd); // Connection closed
            return;
        }

        process(buf, n);
    }
}

// Critical: Must read until EAGAIN!
// Otherwise won't be notified again.
```

---

## Advanced Patterns

### Thread Pool with epoll

```
Main Thread              Worker Threads
───────────              ──────────────
┌─────────┐              ┌─────────┐
│ accept()│              │ Worker  │
│         │──────────────→│ Thread 1│
│ epoll   │              └─────────┘
│  on     │              ┌─────────┐
│ listen  │──────────────→│ Worker  │
│ socket  │              │ Thread 2│
└─────────┘              └─────────┘
                         ┌─────────┐
Each worker has own epoll│ Worker  │
for its connections      │ Thread N│
                         └─────────┘

Benefits:
- Scalable to multiple cores
- Connection affinity
- Cache-friendly
```

### SO_REUSEPORT Load Balancing

```cpp
// Each thread creates socket with SO_REUSEPORT
for (int i = 0; i < num_threads; i++) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bind(fd, &addr, sizeof(addr));
    listen(fd, backlog);

    // Each thread epoll_waits on its own fd
    // Kernel distributes connections!
}

Result:
- Automatic load balancing
- No accept() contention
- Better cache locality
```

---

## Exercises

### Exercise 1: epoll Echo Server
Implement an echo server using epoll that:
- Handles 10,000+ concurrent connections
- Uses edge-triggered mode
- Properly handles partial reads/writes
- Measures and reports latency percentiles

### Exercise 2: io_uring File Server
Build a file server using io_uring:
- Serves files over TCP
- Uses async read/send operations
- Implements zero-copy where possible
- Benchmarks against traditional sendfile()

### Exercise 3: Performance Comparison
Write benchmarks comparing:
- select vs poll vs epoll
- Level-triggered vs edge-triggered epoll
- epoll vs io_uring
- Measure latency and throughput

### Exercise 4: Multi-threaded Server
Implement a multi-threaded server using:
- SO_REUSEPORT
- One epoll instance per thread
- Measure scalability vs number of threads

---

## Key Takeaways

1. **select/poll: Simple but don't scale**
   - Use for <100 connections
   - Portable but slow

2. **epoll: Linux standard for high performance**
   - Scales to 100K+ connections
   - Use edge-triggered for best performance
   - Requires non-blocking sockets

3. **io_uring: The future of Linux I/O**
   - Lowest latency possible
   - True async for all I/O types
   - Zero syscall mode available

4. **Edge-triggered requires discipline**
   - MUST read/write until EAGAIN
   - Non-blocking sockets mandatory
   - More complex but more efficient

5. **Choose based on requirements**
   - Portability → select/poll
   - Performance → epoll
   - Ultimate performance → io_uring

---

## Next Steps

Continue to [Module 4: High-Performance Networking Techniques](../module-04-high-performance/README.md) for kernel bypass, zero-copy, and RDMA.

## Additional Resources

- `man 2 select`
- `man 2 poll`
- `man 7 epoll`
- `man 7 io_uring`
- "The C10K Problem" by Dan Kegel
- io_uring documentation: https://kernel.dk/io_uring.pdf

# Module 4: Signals and Signal Handling

## 📖 Overview

Signals are software interrupts that provide a way to handle asynchronous events. This module covers signal concepts, handling, and best practices.

## 🎯 Learning Objectives

- Understand signal concepts and types
- Implement custom signal handlers
- Use signal masks and blocking
- Handle signals reliably
- Implement graceful shutdown

## 📚 Signal Fundamentals

### Common Signals

| Signal    | Number | Default Action | Description |
|-----------|--------|----------------|-------------|
| SIGHUP    | 1      | Terminate      | Hangup detected |
| SIGINT    | 2      | Terminate      | Interrupt (Ctrl+C) |
| SIGQUIT   | 3      | Core dump      | Quit (Ctrl+\\) |
| SIGKILL   | 9      | Terminate      | Kill (cannot be caught) |
| SIGSEGV   | 11     | Core dump      | Segmentation fault |
| SIGTERM   | 15     | Terminate      | Termination signal |
| SIGCHLD   | 17     | Ignore         | Child status changed |
| SIGALRM   | 14     | Terminate      | Timer alarm |

### Signal Handling

```c
#include <signal.h>

// Simple signal handler
void signal_handler(int signo) {
    // Handle signal
}

// Register handler
signal(SIGINT, signal_handler);

// Modern approach - sigaction
struct sigaction sa;
sa.sa_handler = signal_handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = 0;
sigaction(SIGINT, &sa, NULL);
```

### Signal Operations

```c
// Send signal
kill(pid, SIGTERM);

// Raise signal to self
raise(SIGTERM);

// Set alarm
alarm(seconds);

// Block signals
sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGINT);
sigprocmask(SIG_BLOCK, &set, NULL);
```

## 💻 Code Examples

- [signal_basic.c](./examples/signal_basic.c) - Basic signal handling
- [signal_handler.c](./examples/signal_handler.c) - Custom signal handlers
- [graceful_shutdown.c](./examples/graceful_shutdown.c) - Graceful application shutdown

## 🔧 Compilation

```bash
gcc signal_basic.c -o signal_basic
gcc signal_handler.c -o signal_handler
gcc graceful_shutdown.c -o graceful_shutdown
```

## 🎓 Key Concepts

### Signal Safety
- Use async-signal-safe functions only
- Avoid: printf, malloc, etc. in handlers
- Safe: write, _exit, signal-related functions

### Reliable Signals
- Use sigaction() instead of signal()
- Handle signal races properly
- Block signals during critical sections

## ➡️ Next Module

Continue to [Module 5: Threading and Synchronization](../05-threading/README.md)

# Module 3: Inter-Process Communication (IPC)

## 📖 Overview

Inter-Process Communication (IPC) allows processes to exchange data and synchronize their actions. This module covers various IPC mechanisms available in Linux.

## 🎯 Learning Objectives

- Understand different IPC mechanisms
- Implement pipes for process communication
- Use message queues for asynchronous communication
- Work with shared memory for high-performance IPC
- Apply semaphores for synchronization

## 📚 IPC Mechanisms

### 1. Pipes

**Anonymous Pipes:**
```c
int pipe(int pipefd[2]);
// pipefd[0] - read end
// pipefd[1] - write end
```

**Named Pipes (FIFOs):**
```c
int mkfifo(const char *pathname, mode_t mode);
```

### 2. Message Queues

```c
#include <sys/msg.h>

int msgget(key_t key, int msgflg);
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
```

### 3. Shared Memory

```c
#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg);
void *shmat(int shmid, const void *shmaddr, int shmflg);
int shmdt(const void *shmaddr);
```

### 4. Semaphores

```c
#include <sys/sem.h>

int semget(key_t key, int nsems, int semflg);
int semop(int semid, struct sembuf *sops, size_t nsops);
```

## 💻 Code Examples

- [pipe_demo.c](./examples/pipe_demo.c) - Pipe communication
- [fifo_demo.c](./examples/fifo_demo.c) - Named pipes
- [msgqueue_demo.c](./examples/msgqueue_demo.c) - Message queues
- [shared_memory.c](./examples/shared_memory.c) - Shared memory
- [producer_consumer.c](./examples/producer_consumer.c) - Producer-consumer pattern

## ➡️ Next Module

Continue to [Module 4: Signals](../04-signals/README.md)

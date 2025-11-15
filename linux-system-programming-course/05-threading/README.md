# Module 5: Threading and Synchronization

## 📖 Overview

Multithreading allows concurrent execution within a single process. This module covers POSIX threads (pthreads), synchronization mechanisms, and thread-safe programming.

## 🎯 Learning Objectives

- Create and manage threads
- Implement thread synchronization
- Use mutexes and condition variables
- Avoid race conditions and deadlocks
- Write thread-safe code

## 📚 POSIX Threads (pthreads)

### Thread Creation

```c
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);

int pthread_join(pthread_t thread, void **retval);
int pthread_detach(pthread_t thread);
void pthread_exit(void *retval);
```

### Synchronization Primitives

#### Mutexes
```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_lock(&mutex);
// Critical section
pthread_mutex_unlock(&mutex);
```

#### Condition Variables
```c
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_cond_wait(&cond, &mutex);
pthread_cond_signal(&cond);
pthread_cond_broadcast(&cond);
```

#### Read-Write Locks
```c
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

pthread_rwlock_rdlock(&rwlock);  // Read lock
pthread_rwlock_wrlock(&rwlock);  // Write lock
pthread_rwlock_unlock(&rwlock);
```

## 💻 Code Examples

- [thread_basic.c](./examples/thread_basic.c) - Basic thread creation
- [thread_sync.c](./examples/thread_sync.c) - Synchronization with mutexes
- [producer_consumer.c](./examples/producer_consumer.c) - Producer-consumer with condition variables
- [thread_pool.c](./examples/thread_pool.c) - Thread pool implementation

## 🔧 Compilation

```bash
gcc thread_basic.c -o thread_basic -lpthread
gcc thread_sync.c -o thread_sync -lpthread
gcc producer_consumer.c -o producer_consumer -lpthread
```

## 🎓 Key Concepts

### Thread vs Process
- Threads share memory space
- Lighter than processes
- Faster context switching
- Shared resources need synchronization

### Common Issues
- **Race Conditions**: Unsynchronized access to shared data
- **Deadlock**: Circular waiting for resources
- **Priority Inversion**: Low priority thread blocks high priority
- **Thread Safety**: Functions safe for concurrent use

### Best Practices
- Always initialize synchronization primitives
- Lock in consistent order (prevent deadlock)
- Keep critical sections small
- Use RAII pattern for locks (in C++)
- Check return values

## ➡️ Next Module

Continue to [Module 6: Network Programming](../06-network-programming/README.md)

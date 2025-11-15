# Linux System Programming Course

A comprehensive hands-on course covering Linux system programming concepts, real-world examples, and essential commands for developers.

## 📚 Course Overview

This course is designed for developers who want to master Linux system programming. You'll learn how to write robust, efficient system-level applications and understand the underlying mechanisms of the Linux operating system.

## 🎯 Learning Objectives

By the end of this course, you will:
- Understand Linux system calls and their usage
- Master file I/O operations at the system level
- Create and manage processes and threads
- Implement inter-process communication (IPC) mechanisms
- Handle signals and implement robust error handling
- Build network applications using sockets
- Use essential Linux commands for development and debugging

## 📖 Course Modules

### Module 1: [File I/O Operations](./01-file-io/README.md)
- System calls vs library functions
- File descriptors and file operations
- Advanced I/O techniques
- Memory-mapped files
- Real-world examples: Log file manager, configuration parser

### Module 2: [Process Management](./02-process-management/README.md)
- Process lifecycle and states
- Process creation (fork, exec family)
- Process termination and exit status
- Zombie and orphan processes
- Real-world examples: Daemon processes, process monitoring

### Module 3: [Inter-Process Communication](./03-ipc/README.md)
- Pipes and FIFOs
- Message queues
- Shared memory
- Semaphores
- Real-world examples: Producer-consumer, client-server communication

### Module 4: [Signals and Signal Handling](./04-signals/README.md)
- Signal concepts and types
- Signal handlers and masks
- Reliable signal handling
- Real-world examples: Graceful shutdown, crash handling

### Module 5: [Threading and Synchronization](./05-threading/README.md)
- POSIX threads (pthreads)
- Thread synchronization (mutexes, condition variables)
- Thread-safe programming
- Real-world examples: Thread pool, concurrent data structures

### Module 6: [Network Programming](./06-network-programming/README.md)
- Socket programming basics
- TCP/UDP communication
- Non-blocking I/O and select/poll/epoll
- Real-world examples: HTTP server, chat application

### Module 7: [Essential Linux Commands for Developers](./07-linux-commands/README.md)
- File and directory operations
- Process management commands
- Text processing and searching
- Network debugging tools
- System monitoring and profiling

## 🛠️ Prerequisites

- Basic C programming knowledge
- Familiarity with command-line interface
- Access to a Linux environment (Ubuntu, Fedora, etc.)

## 💻 Development Environment Setup

```bash
# Install essential development tools
sudo apt-get update
sudo apt-get install build-essential gcc gdb valgrind strace

# Clone this repository
git clone <repository-url>
cd linux-system-programming-course

# Each module contains compilable examples
cd 01-file-io/examples
make
```

## 📝 How to Use This Course

1. **Read the theory**: Each module starts with conceptual explanations
2. **Study the examples**: Review well-commented code examples
3. **Compile and run**: Build and execute the examples yourself
4. **Practice exercises**: Complete the exercises at the end of each module
5. **Experiment**: Modify the code and observe the results

## 🚀 Quick Start

```bash
# Start with Module 1
cd 01-file-io
cat README.md

# Compile and run the first example
cd examples
gcc file_operations.c -o file_operations
./file_operations
```

## 📚 Additional Resources

- **Man Pages**: Use `man 2 <syscall>` for system call documentation
- **Linux Programming Interface** by Michael Kerrisk
- **Advanced Programming in the UNIX Environment** by W. Richard Stevens
- [Linux Kernel Documentation](https://www.kernel.org/doc/)

## 🤝 Contributing

Feel free to submit issues, improvements, or additional examples!

## 📄 License

This course is available for educational purposes.

---

**Happy Learning! 🐧**

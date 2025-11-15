# Module 1: File I/O Operations

## 📖 Overview

File I/O is fundamental to Linux system programming. This module covers how to interact with files at the system call level, providing direct control over file operations.

## 🎯 Learning Objectives

- Understand the difference between system calls and library functions
- Master file descriptors and their management
- Perform efficient file I/O operations
- Handle errors properly
- Implement advanced I/O techniques

## 📚 Topics Covered

### 1. System Calls vs Library Functions

**System Calls** are direct requests to the kernel:
- `open()`, `read()`, `write()`, `close()`
- Lower level, unbuffered
- Direct kernel interaction

**Library Functions** provide buffered I/O:
- `fopen()`, `fread()`, `fwrite()`, `fclose()`
- Higher level, buffered
- Built on top of system calls

### 2. File Descriptors

A file descriptor is a non-negative integer that the kernel uses to identify an open file.

**Standard file descriptors:**
- 0: STDIN (standard input)
- 1: STDOUT (standard output)
- 2: STDERR (standard error)

### 3. Essential File I/O System Calls

#### open()
```c
#include <fcntl.h>
int open(const char *pathname, int flags, mode_t mode);
```

**Flags:**
- `O_RDONLY`: Read only
- `O_WRONLY`: Write only
- `O_RDWR`: Read and write
- `O_CREAT`: Create if doesn't exist
- `O_APPEND`: Append mode
- `O_TRUNC`: Truncate to zero length

#### read()
```c
#include <unistd.h>
ssize_t read(int fd, void *buf, size_t count);
```

Returns: number of bytes read, 0 on EOF, -1 on error

#### write()
```c
#include <unistd.h>
ssize_t write(int fd, const void *buf, size_t count);
```

Returns: number of bytes written, -1 on error

#### close()
```c
#include <unistd.h>
int close(int fd);
```

Returns: 0 on success, -1 on error

### 4. Advanced I/O Techniques

- **lseek()**: Random access within files
- **dup() / dup2()**: Duplicate file descriptors
- **fcntl()**: File control operations
- **mmap()**: Memory-mapped I/O

## 💻 Code Examples

### Example 1: Basic File Operations
[file_operations.c](./examples/file_operations.c)
- Opening files with different flags
- Reading and writing data
- Error handling
- Proper resource cleanup

### Example 2: File Copy Utility
[file_copy.c](./examples/file_copy.c)
- Efficient file copying
- Buffer management
- Progress tracking

### Example 3: Log File Manager
[log_manager.c](./examples/log_manager.c)
- Real-world logging system
- Append operations
- File rotation
- Thread-safe logging

### Example 4: Memory-Mapped File I/O
[mmap_example.c](./examples/mmap_example.c)
- Using mmap() for fast file access
- Shared memory mapping
- Performance comparison with read/write

### Example 5: Configuration File Parser
[config_parser.c](./examples/config_parser.c)
- Reading configuration files
- Parsing key-value pairs
- Error handling and validation

## 🔧 Compilation and Execution

```bash
# Compile individual examples
gcc file_operations.c -o file_operations
gcc file_copy.c -o file_copy
gcc log_manager.c -o log_manager -lpthread
gcc mmap_example.c -o mmap_example

# Or use the provided Makefile
make all

# Run examples
./file_operations
./file_copy source.txt destination.txt
./log_manager
./mmap_example testfile.dat
```

## 🎓 Key Concepts

### Error Handling
Always check return values and handle errors:
```c
int fd = open("file.txt", O_RDONLY);
if (fd == -1) {
    perror("open");
    return 1;
}
```

### Resource Management
Always close file descriptors:
```c
if (close(fd) == -1) {
    perror("close");
}
```

### Atomic Operations
Some operations are atomic (all-or-nothing):
- `O_APPEND` flag ensures atomic append
- `O_EXCL` with `O_CREAT` for exclusive creation

## 📝 Practice Exercises

1. **Exercise 1**: Write a program that counts the number of lines, words, and characters in a file (like `wc` command)

2. **Exercise 2**: Implement a simple text editor that can read, modify, and save files

3. **Exercise 3**: Create a file encryption/decryption utility using XOR cipher

4. **Exercise 4**: Build a directory tree walker that displays file sizes

5. **Exercise 5**: Implement a tail -f like utility that monitors a log file for new content

## 🐛 Common Pitfalls

1. **Not checking return values**: Always check for errors
2. **Buffer overflows**: Ensure buffers are large enough
3. **File descriptor leaks**: Always close files
4. **Race conditions**: Use appropriate flags (O_EXCL, O_APPEND)
5. **Partial reads/writes**: Loop until all data is transferred

## 🔍 Debugging Tools

```bash
# Trace system calls
strace ./file_operations

# Check for file descriptor leaks
lsof -p <pid>

# Monitor file access
inotifywait -m /path/to/file
```

## 📚 Further Reading

- `man 2 open`
- `man 2 read`
- `man 2 write`
- `man 2 lseek`
- `man 2 mmap`
- "The Linux Programming Interface" - Chapter 4-5

## ➡️ Next Module

Continue to [Module 2: Process Management](../02-process-management/README.md)

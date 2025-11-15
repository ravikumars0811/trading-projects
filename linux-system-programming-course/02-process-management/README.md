# Module 2: Process Management

## 📖 Overview

Process management is a core concept in Linux system programming. This module covers creating, managing, and controlling processes using system calls.

## 🎯 Learning Objectives

- Understand process lifecycle and states
- Create child processes using fork()
- Execute programs using exec() family
- Handle process termination and collect exit status
- Manage zombie and orphan processes
- Implement daemon processes

## 📚 Topics Covered

### 1. Process Fundamentals

#### What is a Process?
A process is an instance of a running program. Each process has:
- **Process ID (PID)**: Unique identifier
- **Parent Process ID (PPID)**: ID of the parent process
- **Memory space**: Code, data, stack, heap
- **File descriptors**: Open files
- **Environment variables**: Configuration
- **User and group IDs**: Ownership

#### Process States
- **Running**: Currently executing
- **Sleeping**: Waiting for an event
- **Stopped**: Suspended by signal
- **Zombie**: Terminated but not reaped
- **Orphan**: Parent has terminated

### 2. Process Creation - fork()

```c
#include <unistd.h>
pid_t fork(void);
```

**Returns:**
- In parent: PID of child process
- In child: 0
- On error: -1

**How fork() works:**
1. Creates exact copy of parent process
2. Both processes continue from the same point
3. Child gets copy of parent's memory, file descriptors
4. Only difference: return value of fork()

**Example:**
```c
pid_t pid = fork();
if (pid == -1) {
    perror("fork");
    exit(1);
} else if (pid == 0) {
    // Child process code
    printf("I am the child, PID=%d\n", getpid());
} else {
    // Parent process code
    printf("I am the parent, child PID=%d\n", pid);
}
```

### 3. Process Execution - exec() Family

The exec() family replaces the current process image with a new program.

```c
int execl(const char *path, const char *arg, ..., NULL);
int execlp(const char *file, const char *arg, ..., NULL);
int execle(const char *path, const char *arg, ..., NULL, char *const envp[]);
int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execve(const char *path, char *const argv[], char *const envp[]);
```

**Naming convention:**
- `l` - arguments passed as list
- `v` - arguments passed as vector (array)
- `p` - searches PATH for executable
- `e` - environment variables can be specified

**Example:**
```c
// Using execl
execl("/bin/ls", "ls", "-l", NULL);

// Using execvp (searches PATH)
char *args[] = {"ls", "-l", NULL};
execvp("ls", args);
```

### 4. Process Termination

#### Normal Termination
```c
#include <stdlib.h>
void exit(int status);
void _exit(int status);
```

**Difference:**
- `exit()`: Flushes buffers, calls atexit handlers
- `_exit()`: Immediate termination, no cleanup

#### Waiting for Child Processes
```c
#include <sys/wait.h>
pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);
```

**Status Macros:**
- `WIFEXITED(status)`: True if terminated normally
- `WEXITSTATUS(status)`: Get exit status
- `WIFSIGNALED(status)`: True if terminated by signal
- `WTERMSIG(status)`: Get signal number

### 5. Zombie and Orphan Processes

#### Zombie Process
- Process has terminated but parent hasn't collected exit status
- Shows as `<defunct>` in ps output
- Prevention: Always wait() for child processes

#### Orphan Process
- Parent terminates before child
- Child is adopted by init process (PID 1)
- Init automatically reaps zombie orphans

### 6. Daemon Processes

Daemon processes run in the background without a controlling terminal.

**Steps to create a daemon:**
1. Fork and exit parent
2. Create new session (setsid)
3. Fork again and exit first child
4. Change working directory
5. Close file descriptors
6. Open /dev/null for stdin, stdout, stderr

## 💻 Code Examples

### Example 1: Basic Process Creation
[fork_basic.c](./examples/fork_basic.c)
- Understanding fork() behavior
- Parent and child process execution
- Process IDs

### Example 2: Process Hierarchy
[process_tree.c](./examples/process_tree.c)
- Creating multiple child processes
- Process tree visualization
- Parent-child relationships

### Example 3: exec() Demonstrations
[exec_demo.c](./examples/exec_demo.c)
- Using different exec() variants
- Replacing process image
- Command execution

### Example 4: Process Synchronization
[wait_demo.c](./examples/wait_demo.c)
- Waiting for child processes
- Collecting exit status
- Avoiding zombie processes

### Example 5: Daemon Process
[daemon_process.c](./examples/daemon_process.c)
- Creating background daemon
- Proper daemon initialization
- Logging daemon activity

### Example 6: Process Monitor
[process_monitor.c](./examples/process_monitor.c)
- Monitoring child processes
- Automatic restart on failure
- Real-world process supervision

## 🔧 Compilation and Execution

```bash
# Compile examples
gcc fork_basic.c -o fork_basic
gcc process_tree.c -o process_tree
gcc exec_demo.c -o exec_demo
gcc wait_demo.c -o wait_demo
gcc daemon_process.c -o daemon_process
gcc process_monitor.c -o process_monitor

# Or use Makefile
make all

# Run examples
./fork_basic
./process_tree
./exec_demo
./wait_demo
./daemon_process
./process_monitor
```

## 🎓 Key Concepts

### Copy-on-Write (COW)
Modern systems use COW optimization:
- Child shares parent's memory pages initially
- Pages are copied only when modified
- Efficient memory usage

### Process Groups and Sessions
- **Process Group**: Collection of related processes
- **Session**: Collection of process groups
- Used for job control in shells

### File Descriptor Inheritance
- Child inherits parent's file descriptors
- Both share same file offset
- Close unnecessary descriptors in child

## 📝 Practice Exercises

1. **Exercise 1**: Write a program that creates 5 child processes and displays their PIDs in order

2. **Exercise 2**: Implement a simple shell that can execute commands (use fork/exec)

3. **Exercise 3**: Create a program that monitors a child process and restarts it if it crashes

4. **Exercise 4**: Write a daemon that logs system information every 60 seconds

5. **Exercise 5**: Implement a process pool that distributes work among multiple workers

## 🐛 Common Pitfalls

1. **Forgetting to wait()**: Causes zombie processes
2. **Fork bombs**: Uncontrolled fork() calls
3. **Not checking fork() return**: Missing error handling
4. **Assuming execution order**: Parent/child race conditions
5. **File descriptor leaks**: Not closing in child processes

## 🔍 Debugging Tools

```bash
# View process tree
pstree -p

# List all processes
ps aux

# Monitor process in real-time
top

# Trace system calls
strace ./program

# Show process details
cat /proc/<pid>/status
```

## 📚 Useful Commands

```bash
# Get current process ID
echo $$

# Get parent process ID
ps -o ppid= -p $$

# Kill process by PID
kill <pid>

# Kill all processes with name
killall <name>

# View zombie processes
ps aux | grep defunct

# Count number of processes
ps aux | wc -l
```

## 📚 Further Reading

- `man 2 fork`
- `man 3 exec`
- `man 2 wait`
- `man 2 waitpid`
- `man 3 daemon`
- "The Linux Programming Interface" - Chapters 24-28

## ➡️ Next Module

Continue to [Module 3: Inter-Process Communication](../03-ipc/README.md)

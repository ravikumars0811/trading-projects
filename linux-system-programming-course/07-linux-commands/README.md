# Module 7: Essential Linux Commands for Developers

## 📖 Overview

A comprehensive guide to Linux commands that every developer should know. This module covers file operations, process management, text processing, networking, and debugging tools.

## 📚 Command Categories

## 1. File and Directory Operations

### Navigation
```bash
# Print working directory
pwd

# List files
ls                  # Basic list
ls -l               # Long format (permissions, owner, size, date)
ls -lh              # Human-readable sizes
ls -la              # Include hidden files
ls -ltr             # Sort by time, reverse (oldest first)
ls -lS              # Sort by size

# Change directory
cd /path/to/dir     # Absolute path
cd ../              # Parent directory
cd ~                # Home directory
cd -                # Previous directory
```

### File Operations
```bash
# Create files
touch file.txt                    # Create empty file
touch file1.txt file2.txt         # Multiple files
echo "Hello" > file.txt           # Create with content
cat > file.txt                    # Create and write (Ctrl+D to save)

# Copy files
cp source.txt dest.txt            # Copy file
cp -r /source/dir /dest/dir       # Copy directory recursively
cp -i source.txt dest.txt         # Interactive (prompt before overwrite)
cp -v source.txt dest.txt         # Verbose

# Move/Rename
mv old.txt new.txt                # Rename file
mv file.txt /new/location/        # Move file
mv -i old.txt new.txt             # Interactive

# Remove
rm file.txt                       # Delete file
rm -r directory/                  # Delete directory recursively
rm -rf directory/                 # Force delete (use with caution!)
rm -i file.txt                    # Interactive delete
```

### Directory Operations
```bash
# Create directories
mkdir mydir                       # Create directory
mkdir -p path/to/nested/dir       # Create nested directories
mkdir dir1 dir2 dir3              # Multiple directories

# Remove directories
rmdir emptydir                    # Remove empty directory
rm -r directory                   # Remove directory and contents
```

### File Viewing
```bash
# View entire file
cat file.txt                      # Display file contents
cat file1.txt file2.txt           # Concatenate files

# View with paging
less file.txt                     # Page through file (q to quit)
more file.txt                     # Page through file (older)

# View beginning/end
head file.txt                     # First 10 lines
head -n 20 file.txt               # First 20 lines
tail file.txt                     # Last 10 lines
tail -n 20 file.txt               # Last 20 lines
tail -f logfile.log               # Follow file (real-time updates)

# Line numbers
cat -n file.txt                   # Show line numbers
nl file.txt                       # Number lines
```

## 2. File Search and Content Search

### Find Files
```bash
# Find by name
find . -name "*.c"                # Find all .c files
find . -name "test*"              # Find files starting with "test"
find . -iname "*.C"               # Case-insensitive search

# Find by type
find . -type f                    # Find files only
find . -type d                    # Find directories only

# Find by size
find . -size +10M                 # Files larger than 10MB
find . -size -1M                  # Files smaller than 1MB

# Find by time
find . -mtime -7                  # Modified in last 7 days
find . -atime +30                 # Accessed more than 30 days ago

# Execute command on found files
find . -name "*.log" -exec rm {} \;
find . -name "*.c" -exec grep "main" {} \;

# Practical examples
find /var/log -name "*.log" -mtime -1           # Recent logs
find . -name "*.tmp" -delete                    # Delete temp files
find . -type f -perm 0777                       # Find files with 777 permissions
```

### Search File Contents (grep)
```bash
# Basic search
grep "pattern" file.txt                         # Search for pattern
grep "error" logfile.log                        # Find errors in log

# Options
grep -i "pattern" file.txt                      # Case-insensitive
grep -r "pattern" directory/                    # Recursive search
grep -n "pattern" file.txt                      # Show line numbers
grep -v "pattern" file.txt                      # Invert match (exclude)
grep -c "pattern" file.txt                      # Count matches
grep -l "pattern" *.txt                         # Files containing pattern
grep -w "word" file.txt                         # Match whole word only

# Advanced patterns
grep "^pattern" file.txt                        # Lines starting with pattern
grep "pattern$" file.txt                        # Lines ending with pattern
grep "error\|warning" file.txt                  # Multiple patterns (OR)

# Practical examples
grep -r "TODO" .                                # Find all TODOs in project
grep -rn "function_name" src/                   # Find function definition
grep -v "^#" config.conf                        # Exclude comments
ps aux | grep python                            # Find Python processes
cat access.log | grep "404"                     # Find 404 errors
```

## 3. Text Processing

### cut - Extract Columns
```bash
echo "John,Doe,30" | cut -d',' -f1              # Extract first field: John
echo "John,Doe,30" | cut -d',' -f1,3            # Extract fields 1 and 3
cat /etc/passwd | cut -d':' -f1                 # Get all usernames
```

### sed - Stream Editor
```bash
# Substitute
sed 's/old/new/' file.txt                       # Replace first occurrence per line
sed 's/old/new/g' file.txt                      # Replace all occurrences
sed 's/old/new/gi' file.txt                     # Case-insensitive replace

# Delete lines
sed '5d' file.txt                               # Delete line 5
sed '/pattern/d' file.txt                       # Delete lines matching pattern
sed '1,5d' file.txt                             # Delete lines 1-5

# In-place editing
sed -i 's/old/new/g' file.txt                   # Modify file directly
sed -i.bak 's/old/new/g' file.txt               # Create backup before modify
```

### awk - Pattern Scanning
```bash
# Print columns
awk '{print $1}' file.txt                       # Print first column
awk '{print $1, $3}' file.txt                   # Print columns 1 and 3
awk '{print $NF}' file.txt                      # Print last column

# With delimiter
awk -F',' '{print $2}' file.csv                 # CSV processing

# Conditions
awk '$3 > 100' file.txt                         # Lines where col 3 > 100
awk '/pattern/ {print $1}' file.txt             # Print col 1 if pattern matches

# Calculations
awk '{sum += $1} END {print sum}' numbers.txt   # Sum first column
awk '{print $1, $2 * 2}' file.txt               # Multiply column 2 by 2

# Practical examples
ps aux | awk '{print $2, $11}'                  # Process IDs and commands
df -h | awk '$5 > 80 {print $0}'                # Filesystems >80% full
```

### sort and uniq
```bash
# Sort
sort file.txt                                   # Alphabetical sort
sort -n numbers.txt                             # Numerical sort
sort -r file.txt                                # Reverse sort
sort -k2 file.txt                               # Sort by 2nd column
sort -u file.txt                                # Sort and remove duplicates

# Unique lines
uniq file.txt                                   # Remove adjacent duplicates
sort file.txt | uniq                            # Remove all duplicates
uniq -c file.txt                                # Count occurrences
uniq -d file.txt                                # Show only duplicates
```

### wc - Word Count
```bash
wc file.txt                                     # Lines, words, bytes
wc -l file.txt                                  # Count lines only
wc -w file.txt                                  # Count words only
wc -c file.txt                                  # Count bytes
find . -name "*.c" | wc -l                      # Count .c files
```

## 4. Process Management

### Viewing Processes
```bash
# List processes
ps                                              # Current user processes
ps aux                                          # All processes (BSD style)
ps -ef                                          # All processes (System V style)
ps aux | grep apache                            # Find specific process

# Process tree
pstree                                          # Show process tree
pstree -p                                       # With PIDs

# Real-time monitoring
top                                             # Interactive process viewer
htop                                            # Enhanced top (if installed)
```

### Process Control
```bash
# Run in background
command &                                       # Run in background
./script.sh &

# Bring to foreground
fg                                              # Bring last job to foreground
fg %1                                           # Bring job 1 to foreground

# List jobs
jobs                                            # Show background jobs

# Process signals
kill PID                                        # Send SIGTERM (terminate)
kill -9 PID                                     # Send SIGKILL (force kill)
kill -STOP PID                                  # Pause process
kill -CONT PID                                  # Resume process
killall process_name                            # Kill by name
pkill pattern                                   # Kill by pattern

# Process priority
nice -n 10 command                              # Run with lower priority
renice -n 5 -p PID                              # Change priority
```

## 5. System Information

```bash
# System info
uname -a                                        # Kernel and system info
hostname                                        # System hostname
uptime                                          # System uptime
whoami                                          # Current user
id                                              # User ID and groups

# Hardware info
lscpu                                           # CPU information
free -h                                         # Memory usage
df -h                                           # Disk usage
du -sh directory/                               # Directory size
du -h --max-depth=1                             # Size of subdirectories

# Mounted filesystems
mount                                           # Show mounted filesystems
lsblk                                           # List block devices
```

## 6. Networking Commands

### Network Configuration
```bash
# IP configuration
ip addr show                                    # Show IP addresses
ip link show                                    # Show network interfaces
ifconfig                                        # Network config (older)

# Routing
ip route show                                   # Show routing table
netstat -rn                                     # Show routes (older)
```

### Network Testing
```bash
# Connectivity
ping google.com                                 # Test connectivity
ping -c 4 google.com                            # Send 4 packets

# DNS
nslookup google.com                             # DNS lookup
dig google.com                                  # Detailed DNS info
host google.com                                 # Simple DNS lookup

# Port scanning
netstat -tuln                                   # Show listening ports
ss -tuln                                        # Socket statistics (modern)
lsof -i :8080                                   # What's using port 8080

# Download
wget https://example.com/file.txt               # Download file
curl https://api.example.com                    # HTTP request
curl -X POST -d "data" https://api.example.com  # POST request
```

### Network Monitoring
```bash
# Traffic monitoring
tcpdump -i eth0                                 # Capture packets
tcpdump -i eth0 port 80                         # Capture HTTP traffic
tcpdump -i eth0 -w capture.pcap                 # Write to file

# Bandwidth
iftop                                           # Real-time bandwidth
nethogs                                         # Per-process bandwidth
```

## 7. File Permissions and Ownership

```bash
# View permissions
ls -l file.txt                                  # Show permissions

# Change permissions (chmod)
chmod 644 file.txt                              # rw-r--r--
chmod 755 script.sh                             # rwxr-xr-x
chmod +x script.sh                              # Add execute permission
chmod -w file.txt                               # Remove write permission
chmod u+x,g+r file.txt                          # User: +x, Group: +r

# Permissions explained
# 4 = read (r), 2 = write (w), 1 = execute (x)
# 7 = rwx, 6 = rw-, 5 = r-x, 4 = r--

# Change ownership
chown user file.txt                             # Change owner
chown user:group file.txt                       # Change owner and group
chown -R user:group directory/                  # Recursive

# Change group
chgrp group file.txt                            # Change group
```

## 8. Archiving and Compression

```bash
# tar - Archive files
tar -cvf archive.tar files/                     # Create archive
tar -xvf archive.tar                            # Extract archive
tar -tvf archive.tar                            # List contents
tar -czvf archive.tar.gz files/                 # Create compressed (gzip)
tar -xzvf archive.tar.gz                        # Extract compressed
tar -cjvf archive.tar.bz2 files/                # Create (bzip2)

# gzip compression
gzip file.txt                                   # Compress (creates file.txt.gz)
gunzip file.txt.gz                              # Decompress
gzip -k file.txt                                # Keep original

# zip/unzip
zip archive.zip file1.txt file2.txt             # Create zip
zip -r archive.zip directory/                   # Zip directory
unzip archive.zip                               # Extract zip
unzip -l archive.zip                            # List contents
```

## 9. Debugging and Development Tools

### Compiling and Debugging
```bash
# GCC compiler
gcc program.c -o program                        # Compile
gcc -g program.c -o program                     # Compile with debug symbols
gcc -Wall program.c -o program                  # Show all warnings
gcc -O2 program.c -o program                    # Optimize level 2

# Make
make                                            # Build using Makefile
make clean                                      # Clean build files

# GDB debugger
gdb ./program                                   # Start debugger
gdb --args ./program arg1 arg2                  # With arguments
```

### System Call Tracing
```bash
# strace - Trace system calls
strace ./program                                # Trace all system calls
strace -c ./program                             # Summary of calls
strace -e open ./program                        # Trace open() calls
strace -p PID                                   # Attach to running process

# ltrace - Library call tracing
ltrace ./program                                # Trace library calls
```

### Memory Analysis
```bash
# valgrind - Memory debugging
valgrind ./program                              # Check memory leaks
valgrind --leak-check=full ./program            # Detailed leak info
valgrind --track-origins=yes ./program          # Track uninitialized values
```

## 10. Version Control (Git)

```bash
# Repository setup
git init                                        # Initialize repository
git clone <url>                                 # Clone repository

# Basic operations
git status                                      # Check status
git add file.txt                                # Stage file
git add .                                       # Stage all changes
git commit -m "Message"                         # Commit changes
git push                                        # Push to remote
git pull                                        # Pull from remote

# Branching
git branch                                      # List branches
git branch feature-x                            # Create branch
git checkout feature-x                          # Switch branch
git checkout -b feature-y                       # Create and switch
git merge feature-x                             # Merge branch

# History
git log                                         # View commit history
git log --oneline                               # Compact history
git log --graph --all                           # Visual history
git diff                                        # Show changes
git diff file.txt                               # Changes in specific file

# Undoing changes
git reset HEAD file.txt                         # Unstage file
git checkout -- file.txt                        # Discard changes
git reset --hard HEAD~1                         # Undo last commit (destructive)
```

## 11. Useful Command Combinations

```bash
# Find and delete old files
find /tmp -name "*.tmp" -mtime +7 -delete

# Find largest files
du -ah . | sort -rh | head -20

# Monitor log file for errors
tail -f /var/log/syslog | grep -i error

# Count lines of code
find . -name "*.c" -exec cat {} \; | wc -l

# Find and replace in multiple files
find . -name "*.txt" -exec sed -i 's/old/new/g' {} \;

# Check which process is using most CPU
ps aux | sort -nk 3 | tail -5

# Check which process is using most memory
ps aux | sort -nk 4 | tail -5

# Disk usage of each subdirectory
du -h --max-depth=1 | sort -hr

# Find files modified in last 24 hours
find . -type f -mtime -1

# Count files by extension
find . -type f | sed 's/.*\.//' | sort | uniq -c | sort -nr

# Monitor network connections
watch -n 1 'netstat -an | grep ESTABLISHED'
```

## 12. Shell Shortcuts and Tips

```bash
# Keyboard shortcuts
Ctrl+C          # Kill current process
Ctrl+Z          # Suspend current process
Ctrl+D          # Exit shell or end of input
Ctrl+L          # Clear screen
Ctrl+A          # Move to beginning of line
Ctrl+E          # Move to end of line
Ctrl+U          # Delete from cursor to beginning
Ctrl+K          # Delete from cursor to end
Ctrl+R          # Reverse search history

# History commands
history                         # Show command history
!n                             # Run command n from history
!!                             # Run last command
!$                             # Last argument of previous command
sudo !!                        # Run last command with sudo

# Redirection and pipes
command > file.txt             # Redirect stdout to file (overwrite)
command >> file.txt            # Redirect stdout to file (append)
command 2> errors.txt          # Redirect stderr
command &> output.txt          # Redirect both stdout and stderr
command1 | command2            # Pipe output to another command
command < input.txt            # Read input from file
```

## 📚 Additional Resources

- **Man pages**: `man command` - Detailed manual for any command
- **Help**: `command --help` - Quick help
- **Info**: `info command` - Detailed info pages
- **which**: `which command` - Show command location
- **type**: `type command` - Show command type

## 🎓 Practice Exercises

1. Find all .log files larger than 10MB modified in the last week
2. Count the number of lines of code in a project (all .c and .h files)
3. Monitor a log file and email when an error occurs
4. Create a script to backup important files daily
5. Find and kill all zombie processes
6. Extract all IP addresses from an access log
7. Find the top 10 largest directories on system
8. Monitor network traffic on port 80

---

**Remember**: Use `man command` to learn more about any command!

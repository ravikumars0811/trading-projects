/*
 * mmap_example.c - Memory-Mapped File I/O
 *
 * Demonstrates:
 * - Using mmap() for efficient file access
 * - Memory-mapped I/O vs traditional read/write
 * - Shared memory mapping
 * - Performance comparison
 *
 * Usage: ./mmap_example [filename]
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

#define TEST_FILE_SIZE (10 * 1024 * 1024)  // 10MB
#define BUFFER_SIZE 4096

// Traditional read/write method
double traditional_io_read(const char *filename) {
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    long total_bytes = 0;
    clock_t start, end;

    start = clock();

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        total_bytes += bytes_read;
    }

    close(fd);

    end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Traditional I/O: Read %ld bytes in %.6f seconds\n",
           total_bytes, time_spent);

    return time_spent;
}

// Memory-mapped method
double mmap_io_read(const char *filename) {
    int fd;
    struct stat st;
    char *mapped;
    long total_bytes = 0;
    clock_t start, end;

    start = clock();

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    // Get file size
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return -1;
    }

    // Map the entire file into memory
    mapped = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }

    // Access the mapped memory
    for (off_t i = 0; i < st.st_size; i += BUFFER_SIZE) {
        // Simulate reading by accessing memory
        volatile char c = mapped[i];
        total_bytes += (i + BUFFER_SIZE < st.st_size) ? BUFFER_SIZE : (st.st_size - i);
    }

    // Unmap the memory
    if (munmap(mapped, st.st_size) == -1) {
        perror("munmap");
    }

    close(fd);

    end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Memory-mapped I/O: Read %ld bytes in %.6f seconds\n",
           total_bytes, time_spent);

    return time_spent;
}

// Create a test file
int create_test_file(const char *filename, size_t size) {
    int fd;
    char buffer[BUFFER_SIZE];

    printf("Creating test file (%zu MB)...\n", size / (1024 * 1024));

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    // Fill buffer with sample data
    memset(buffer, 'A', BUFFER_SIZE);

    size_t remaining = size;
    while (remaining > 0) {
        size_t to_write = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
        if (write(fd, buffer, to_write) != to_write) {
            perror("write");
            close(fd);
            return -1;
        }
        remaining -= to_write;
    }

    close(fd);
    printf("Test file created: %s\n\n", filename);
    return 0;
}

// Example: Modify file using mmap
void example_mmap_write(const char *filename) {
    int fd;
    struct stat st;
    char *mapped;

    printf("\n=== Example: Modifying file with mmap ===\n");

    fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("open");
        return;
    }

    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return;
    }

    // Map file with read-write permissions
    mapped = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return;
    }

    // Modify the first 100 bytes
    const char *new_data = "MODIFIED: This content was changed using memory-mapped I/O!";
    size_t len = strlen(new_data);
    if (len < st.st_size) {
        memcpy(mapped, new_data, len);
        printf("Modified first %zu bytes of the file\n", len);

        // Changes are automatically written back to the file
        // Use msync() to ensure immediate write-back
        if (msync(mapped, st.st_size, MS_SYNC) == -1) {
            perror("msync");
        }
    }

    munmap(mapped, st.st_size);
    close(fd);

    // Verify the change
    fd = open(filename, O_RDONLY);
    char verify_buffer[200];
    read(fd, verify_buffer, sizeof(verify_buffer) - 1);
    verify_buffer[sizeof(verify_buffer) - 1] = '\0';
    printf("Verification (first 100 chars): %.100s\n", verify_buffer);
    close(fd);
}

// Example: Anonymous mapping (shared memory between processes)
void example_anonymous_mmap() {
    printf("\n=== Example: Anonymous memory mapping ===\n");

    // Create anonymous mapping
    int *shared_data = mmap(NULL, sizeof(int) * 10,
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (shared_data == MAP_FAILED) {
        perror("mmap");
        return;
    }

    // Initialize shared memory
    for (int i = 0; i < 10; i++) {
        shared_data[i] = i * 10;
    }

    printf("Created anonymous shared memory mapping\n");
    printf("Values: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", shared_data[i]);
    }
    printf("\n");

    // This could be shared between parent and child processes via fork()
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        printf("Child process: Modifying shared memory\n");
        for (int i = 0; i < 10; i++) {
            shared_data[i] += 100;
        }
        exit(0);
    } else if (pid > 0) {
        // Parent process
        wait(NULL);
        printf("Parent process: Reading modified values: ");
        for (int i = 0; i < 10; i++) {
            printf("%d ", shared_data[i]);
        }
        printf("\n");
    }

    munmap(shared_data, sizeof(int) * 10);
}

int main(int argc, char *argv[]) {
    const char *test_file = (argc > 1) ? argv[1] : "test_mmap.dat";

    printf("=== Memory-Mapped File I/O Demo ===\n\n");

    // Create test file
    if (create_test_file(test_file, TEST_FILE_SIZE) == -1) {
        return 1;
    }

    // Performance comparison
    printf("=== Performance Comparison ===\n");
    double traditional_time = traditional_io_read(test_file);
    double mmap_time = mmap_io_read(test_file);

    if (traditional_time > 0 && mmap_time > 0) {
        printf("\nSpeed improvement: %.2fx faster with mmap\n",
               traditional_time / mmap_time);
    }

    // Demonstrate write operations
    example_mmap_write(test_file);

    // Demonstrate anonymous mapping
    example_anonymous_mmap();

    printf("\nCleanup: rm %s\n", test_file);
    return 0;
}

/*
 * file_operations.c - Basic File I/O Operations
 *
 * Demonstrates:
 * - Opening files with different modes
 * - Reading and writing data
 * - Error handling
 * - Proper resource cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1024

void example_write_file() {
    int fd;
    const char *filename = "test_output.txt";
    const char *data = "Hello, Linux System Programming!\n";
    ssize_t bytes_written;

    printf("=== Example 1: Writing to a file ===\n");

    // Open file for writing, create if doesn't exist, truncate if exists
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return;
    }

    printf("File descriptor: %d\n", fd);

    // Write data to file
    bytes_written = write(fd, data, strlen(data));
    if (bytes_written == -1) {
        perror("write");
        close(fd);
        return;
    }

    printf("Wrote %zd bytes to %s\n", bytes_written, filename);

    // Close the file
    if (close(fd) == -1) {
        perror("close");
    }

    printf("File closed successfully\n\n");
}

void example_read_file() {
    int fd;
    const char *filename = "test_output.txt";
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    printf("=== Example 2: Reading from a file ===\n");

    // Open file for reading
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return;
    }

    // Read data from file
    bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read == -1) {
        perror("read");
        close(fd);
        return;
    }

    // Null-terminate the buffer
    buffer[bytes_read] = '\0';

    printf("Read %zd bytes from %s\n", bytes_read, filename);
    printf("Content: %s\n", buffer);

    close(fd);
    printf("\n");
}

void example_append_file() {
    int fd;
    const char *filename = "test_output.txt";
    const char *data = "This line was appended!\n";
    ssize_t bytes_written;

    printf("=== Example 3: Appending to a file ===\n");

    // Open file in append mode
    fd = open(filename, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("open");
        return;
    }

    bytes_written = write(fd, data, strlen(data));
    if (bytes_written == -1) {
        perror("write");
        close(fd);
        return;
    }

    printf("Appended %zd bytes to %s\n", bytes_written, filename);

    close(fd);
    printf("\n");
}

void example_lseek() {
    int fd;
    const char *filename = "test_output.txt";
    char buffer[BUFFER_SIZE];
    off_t offset;
    ssize_t bytes_read;

    printf("=== Example 4: Random access with lseek ===\n");

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return;
    }

    // Get file size
    offset = lseek(fd, 0, SEEK_END);
    printf("File size: %ld bytes\n", offset);

    // Seek to beginning
    lseek(fd, 0, SEEK_SET);
    printf("Seeked to beginning\n");

    // Read first 20 bytes
    bytes_read = read(fd, buffer, 20);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("First 20 bytes: %s\n", buffer);
    }

    // Seek to position 10 from current position
    lseek(fd, 10, SEEK_CUR);
    printf("Seeked 10 bytes forward from current position\n");

    close(fd);
    printf("\n");
}

void example_file_permissions() {
    int fd;
    const char *filename = "restricted_file.txt";

    printf("=== Example 5: File permissions ===\n");

    // Create file with specific permissions (rw-r--r--)
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return;
    }

    write(fd, "Restricted content\n", 19);
    close(fd);

    printf("Created %s with permissions 0644 (rw-r--r--)\n", filename);

    // Create file with owner-only permissions (rw-------)
    fd = open("private_file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) {
        perror("open");
        return;
    }

    write(fd, "Private content\n", 16);
    close(fd);

    printf("Created private_file.txt with permissions 0600 (rw-------)\n");
    printf("\nVerify with: ls -l *.txt\n\n");
}

void example_error_handling() {
    int fd;
    char buffer[100];

    printf("=== Example 6: Error handling ===\n");

    // Try to open non-existent file without O_CREAT
    fd = open("nonexistent.txt", O_RDONLY);
    if (fd == -1) {
        printf("Expected error occurred:\n");
        perror("open");
        printf("Error number: %d (%s)\n", errno, strerror(errno));
    }

    // Try to read from invalid file descriptor
    ssize_t result = read(-1, buffer, sizeof(buffer));
    if (result == -1) {
        printf("\nExpected error occurred:\n");
        perror("read");
    }

    printf("\n");
}

int main() {
    printf("Linux System Programming - File I/O Examples\n");
    printf("============================================\n\n");

    example_write_file();
    example_read_file();
    example_append_file();
    example_read_file();  // Read again to see appended content
    example_lseek();
    example_file_permissions();
    example_error_handling();

    printf("All examples completed!\n");
    printf("Check the created files: test_output.txt, restricted_file.txt, private_file.txt\n");

    return 0;
}

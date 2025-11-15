/*
 * file_copy.c - Efficient File Copy Utility
 *
 * Demonstrates:
 * - Efficient file copying using system calls
 * - Buffer management
 * - Progress tracking
 * - Error handling
 *
 * Usage: ./file_copy source.txt destination.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 8192  // 8KB buffer for efficient I/O

int copy_file(const char *source, const char *dest) {
    int src_fd, dest_fd;
    ssize_t bytes_read, bytes_written;
    char buffer[BUFFER_SIZE];
    struct stat st;
    off_t total_bytes = 0;
    off_t file_size = 0;

    // Open source file
    src_fd = open(source, O_RDONLY);
    if (src_fd == -1) {
        fprintf(stderr, "Error opening source file '%s': %s\n",
                source, strerror(errno));
        return -1;
    }

    // Get source file size for progress tracking
    if (fstat(src_fd, &st) == 0) {
        file_size = st.st_size;
        printf("Source file size: %ld bytes\n", file_size);
    }

    // Open destination file (create if doesn't exist, truncate if exists)
    // Use same permissions as source file
    dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    if (dest_fd == -1) {
        fprintf(stderr, "Error opening destination file '%s': %s\n",
                dest, strerror(errno));
        close(src_fd);
        return -1;
    }

    printf("Copying '%s' to '%s'...\n", source, dest);

    // Copy data in chunks
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        char *ptr = buffer;
        ssize_t bytes_to_write = bytes_read;

        // Handle partial writes
        while (bytes_to_write > 0) {
            bytes_written = write(dest_fd, ptr, bytes_to_write);
            if (bytes_written == -1) {
                if (errno == EINTR) {
                    // Interrupted by signal, retry
                    continue;
                }
                fprintf(stderr, "Error writing to destination: %s\n",
                        strerror(errno));
                close(src_fd);
                close(dest_fd);
                return -1;
            }

            bytes_to_write -= bytes_written;
            ptr += bytes_written;
        }

        total_bytes += bytes_read;

        // Show progress
        if (file_size > 0) {
            int progress = (int)((total_bytes * 100) / file_size);
            printf("\rProgress: %d%% (%ld / %ld bytes)",
                   progress, total_bytes, file_size);
            fflush(stdout);
        }
    }

    printf("\n");

    if (bytes_read == -1) {
        fprintf(stderr, "Error reading from source: %s\n", strerror(errno));
        close(src_fd);
        close(dest_fd);
        return -1;
    }

    // Close files
    if (close(src_fd) == -1) {
        fprintf(stderr, "Error closing source file: %s\n", strerror(errno));
    }

    if (close(dest_fd) == -1) {
        fprintf(stderr, "Error closing destination file: %s\n", strerror(errno));
        return -1;
    }

    printf("Successfully copied %ld bytes\n", total_bytes);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        fprintf(stderr, "Example: %s input.txt output.txt\n", argv[0]);
        return 1;
    }

    const char *source = argv[1];
    const char *dest = argv[2];

    // Check if source and destination are the same
    if (strcmp(source, dest) == 0) {
        fprintf(stderr, "Error: Source and destination cannot be the same\n");
        return 1;
    }

    // Perform the copy
    if (copy_file(source, dest) == -1) {
        fprintf(stderr, "File copy failed\n");
        return 1;
    }

    printf("File copy completed successfully!\n");
    return 0;
}

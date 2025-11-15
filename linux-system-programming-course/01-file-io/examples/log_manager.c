/*
 * log_manager.c - Thread-Safe Log File Manager
 *
 * Demonstrates:
 * - Real-world logging system implementation
 * - Thread-safe file operations
 * - Atomic append operations
 * - Log rotation
 * - Different log levels
 *
 * Compile: gcc log_manager.c -o log_manager -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <errno.h>

#define LOG_FILE "application.log"
#define MAX_LOG_SIZE 1024 * 1024  // 1MB
#define MAX_LOG_FILES 5
#define BUFFER_SIZE 1024

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
} LogLevel;

static const char *log_level_strings[] = {
    "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"
};

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static int current_log_fd = -1;
static LogLevel min_log_level = LOG_INFO;

// Get current timestamp
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Get file size
off_t get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

// Rotate log files
int rotate_logs() {
    char old_name[256], new_name[256];

    // Close current log file
    if (current_log_fd != -1) {
        close(current_log_fd);
        current_log_fd = -1;
    }

    // Rotate existing log files
    for (int i = MAX_LOG_FILES - 1; i > 0; i--) {
        if (i == 1) {
            snprintf(old_name, sizeof(old_name), "%s", LOG_FILE);
        } else {
            snprintf(old_name, sizeof(old_name), "%s.%d", LOG_FILE, i - 1);
        }
        snprintf(new_name, sizeof(new_name), "%s.%d", LOG_FILE, i);

        // Remove oldest log if it exists
        if (i == MAX_LOG_FILES - 1) {
            unlink(new_name);
        }

        // Rename log file
        rename(old_name, new_name);
    }

    printf("Log files rotated\n");
    return 0;
}

// Initialize logging system
int log_init() {
    pthread_mutex_lock(&log_mutex);

    // Open log file in append mode
    current_log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (current_log_fd == -1) {
        pthread_mutex_unlock(&log_mutex);
        perror("Failed to open log file");
        return -1;
    }

    pthread_mutex_unlock(&log_mutex);
    printf("Logging system initialized: %s\n", LOG_FILE);
    return 0;
}

// Close logging system
void log_cleanup() {
    pthread_mutex_lock(&log_mutex);
    if (current_log_fd != -1) {
        close(current_log_fd);
        current_log_fd = -1;
    }
    pthread_mutex_unlock(&log_mutex);
    printf("Logging system closed\n");
}

// Write log message
int log_message(LogLevel level, const char *format, ...) {
    if (level < min_log_level) {
        return 0;  // Skip messages below minimum level
    }

    char timestamp[64];
    char message[BUFFER_SIZE];
    char log_entry[BUFFER_SIZE + 128];
    va_list args;

    // Format the message
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    // Get timestamp
    get_timestamp(timestamp, sizeof(timestamp));

    // Format log entry
    int len = snprintf(log_entry, sizeof(log_entry),
                      "[%s] [%s] %s\n",
                      timestamp,
                      log_level_strings[level],
                      message);

    pthread_mutex_lock(&log_mutex);

    // Check if rotation is needed
    off_t current_size = get_file_size(LOG_FILE);
    if (current_size > MAX_LOG_SIZE) {
        rotate_logs();
        // Reopen log file
        current_log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    }

    // Write to log file (O_APPEND ensures atomic writes)
    ssize_t written = write(current_log_fd, log_entry, len);
    if (written == -1) {
        pthread_mutex_unlock(&log_mutex);
        perror("Failed to write log");
        return -1;
    }

    // Also print to console for demonstration
    printf("%s", log_entry);

    pthread_mutex_unlock(&log_mutex);
    return 0;
}

// Thread function for demonstration
void* worker_thread(void *arg) {
    int thread_id = *(int*)arg;

    for (int i = 0; i < 5; i++) {
        log_message(LOG_INFO, "Thread %d: Processing task %d", thread_id, i + 1);
        usleep(100000);  // 100ms
    }

    return NULL;
}

int main() {
    printf("=== Thread-Safe Log Manager Demo ===\n\n");

    // Initialize logging
    if (log_init() == -1) {
        return 1;
    }

    // Log some messages
    log_message(LOG_INFO, "Application started");
    log_message(LOG_DEBUG, "This debug message won't appear (below min level)");
    log_message(LOG_WARNING, "This is a warning message");
    log_message(LOG_ERROR, "An error occurred: %s", "Sample error");
    log_message(LOG_CRITICAL, "Critical system failure!");

    // Demonstrate thread-safe logging
    printf("\nDemonstrating multi-threaded logging:\n");

    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};

    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    log_message(LOG_INFO, "All worker threads completed");

    // Demonstrate log rotation by writing lots of data
    printf("\nDemonstrating log rotation:\n");
    for (int i = 0; i < 100; i++) {
        log_message(LOG_INFO, "Log entry #%d - Adding content to trigger rotation when size exceeds limit", i);
    }

    log_message(LOG_INFO, "Application shutting down");

    // Cleanup
    log_cleanup();

    printf("\nCheck the log files:\n");
    printf("  ls -lh %s*\n", LOG_FILE);
    printf("  cat %s\n", LOG_FILE);

    return 0;
}

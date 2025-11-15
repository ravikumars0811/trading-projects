/*
 * daemon_process.c - Creating a Daemon Process
 *
 * Demonstrates:
 * - Proper daemon initialization
 * - Background process creation
 * - Detaching from terminal
 * - Logging daemon activity
 *
 * Compile: gcc daemon_process.c -o daemon_process
 * Run: ./daemon_process
 * Check: ps aux | grep daemon_process
 * Stop: kill <pid>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#define DAEMON_NAME "my_daemon"
#define LOG_FILE "/tmp/daemon.log"
#define PID_FILE "/tmp/daemon.pid"

// Signal handler for graceful shutdown
volatile sig_atomic_t running = 1;

void signal_handler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        syslog(LOG_INFO, "Received signal %d, shutting down", sig);
        running = 0;
    } else if (sig == SIGHUP) {
        syslog(LOG_INFO, "Received SIGHUP, reloading configuration");
        // Reload configuration here
    }
}

// Write PID to file
int write_pid_file() {
    int fd = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return -1;
    }

    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
    write(fd, pid_str, strlen(pid_str));
    close(fd);

    return 0;
}

// Daemonize the process
int daemonize() {
    pid_t pid, sid;

    // Step 1: Fork off the parent process
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    // Exit parent process
    if (pid > 0) {
        printf("Daemon started with PID: %d\n", pid);
        printf("Log file: %s\n", LOG_FILE);
        printf("PID file: %s\n", PID_FILE);
        printf("To stop: kill %d\n", pid);
        exit(0);
    }

    // Step 2: Create a new session
    sid = setsid();
    if (sid < 0) {
        return -1;
    }

    // Step 3: Fork again to ensure daemon can't acquire terminal
    pid = fork();
    if (pid < 0) {
        return -1;
    }

    if (pid > 0) {
        exit(0);
    }

    // Step 4: Change working directory to root
    if (chdir("/") < 0) {
        return -1;
    }

    // Step 5: Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Step 6: Redirect to /dev/null
    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > STDERR_FILENO) {
        close(fd);
    }

    // Step 7: Set file permissions mask
    umask(0027);

    return 0;
}

// Log to file (since we're detached from terminal)
void log_to_file(const char *message) {
    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0) {
        time_t now = time(NULL);
        char timestamp[64];
        struct tm *tm_info = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

        char log_entry[512];
        snprintf(log_entry, sizeof(log_entry), "[%s] %s\n", timestamp, message);

        write(fd, log_entry, strlen(log_entry));
        close(fd);
    }
}

// Main daemon work
void daemon_work() {
    int counter = 0;
    char message[256];

    while (running) {
        // Perform daemon tasks
        snprintf(message, sizeof(message),
                "Daemon is running... iteration %d, PID=%d",
                ++counter, getpid());

        log_to_file(message);
        syslog(LOG_INFO, "%s", message);

        // Sleep for 5 seconds
        sleep(5);

        // Example: Check system status, monitor files, etc.
        if (counter % 6 == 0) {
            log_to_file("Performing periodic maintenance task");
        }
    }

    log_to_file("Daemon shutting down gracefully");
    syslog(LOG_INFO, "Daemon shutting down");
}

int main(int argc, char *argv[]) {
    // Open system log
    openlog(DAEMON_NAME, LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Starting daemon");

    // Daemonize
    if (daemonize() < 0) {
        syslog(LOG_ERR, "Failed to daemonize");
        closelog();
        return 1;
    }

    // Write PID file
    if (write_pid_file() < 0) {
        syslog(LOG_ERR, "Failed to write PID file");
        log_to_file("ERROR: Failed to write PID file");
        closelog();
        return 1;
    }

    // Setup signal handlers
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGCHLD, SIG_IGN);  // Ignore child signals

    log_to_file("Daemon initialized successfully");
    syslog(LOG_INFO, "Daemon initialized, starting main loop");

    // Main daemon loop
    daemon_work();

    // Cleanup
    unlink(PID_FILE);
    closelog();

    return 0;
}

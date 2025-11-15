/*
 * signal_handler.c - Signal Handling Examples
 *
 * Demonstrates:
 * - Registering signal handlers
 * - Handling different signals
 * - Graceful shutdown
 * - Signal blocking
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

volatile sig_atomic_t got_signal = 0;
volatile sig_atomic_t signal_count = 0;

// Signal handler for SIGINT (Ctrl+C)
void sigint_handler(int signo) {
    const char msg[] = "\nCaught SIGINT (Ctrl+C)! Press Ctrl+C again to exit.\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    signal_count++;
    if (signal_count >= 2) {
        const char exit_msg[] = "Exiting...\n";
        write(STDOUT_FILENO, exit_msg, sizeof(exit_msg) - 1);
        _exit(0);
    }
}

// Signal handler for SIGTERM
void sigterm_handler(int signo) {
    const char msg[] = "Caught SIGTERM! Cleaning up...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    got_signal = 1;
}

// Signal handler for SIGUSR1
void sigusr1_handler(int signo) {
    const char msg[] = "Received SIGUSR1!\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

// Signal handler for SIGALRM
void sigalrm_handler(int signo) {
    const char msg[] = "Alarm triggered!\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    alarm(3);  // Set alarm for 3 more seconds
}

void example_basic_signal() {
    printf("=== Example 1: Basic Signal Handling ===\n");
    printf("Press Ctrl+C twice to exit\n");
    printf("PID: %d\n", getpid());

    // Register signal handler
    signal(SIGINT, sigint_handler);

    // Infinite loop
    while (1) {
        printf("Running... (count=%d)\n", signal_count);
        sleep(2);
    }
}

void example_multiple_signals() {
    printf("=== Example 2: Multiple Signal Handlers ===\n");
    printf("PID: %d\n", getpid());
    printf("Send signals: kill -TERM %d or kill -USR1 %d\n\n",
           getpid(), getpid());

    // Register multiple handlers
    signal(SIGTERM, sigterm_handler);
    signal(SIGUSR1, sigusr1_handler);

    int counter = 0;
    while (!got_signal && counter < 20) {
        printf("Working... (%d/20)\n", ++counter);
        sleep(1);
    }

    if (got_signal) {
        printf("Received termination signal, exiting gracefully\n");
    }
}

void example_sigaction() {
    printf("=== Example 3: Using sigaction (Recommended) ===\n");

    struct sigaction sa;

    // Setup SIGINT handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;  // Restart interrupted system calls

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return;
    }

    printf("Signal handler installed with sigaction\n");
    printf("Press Ctrl+C to test\n");

    for (int i = 0; i < 10 && signal_count < 2; i++) {
        printf("Iteration %d\n", i + 1);
        sleep(1);
    }
}

void example_signal_blocking() {
    printf("=== Example 4: Signal Blocking ===\n");

    sigset_t block_set, old_set;

    // Initialize signal set
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);

    printf("Blocking SIGINT for 5 seconds...\n");
    printf("Try pressing Ctrl+C (it will be queued)\n");

    // Block SIGINT
    sigprocmask(SIG_BLOCK, &block_set, &old_set);

    sleep(5);

    printf("Unblocking SIGINT...\n");

    // Unblock SIGINT (queued signal will be delivered)
    sigprocmask(SIG_UNBLOCK, &block_set, NULL);

    sleep(2);
    printf("Done\n");
}

void example_alarm_signal() {
    printf("=== Example 5: Alarm Signal ===\n");

    signal(SIGALRM, sigalrm_handler);

    printf("Setting alarm for 3 seconds\n");
    alarm(3);

    for (int i = 0; i < 10; i++) {
        printf("Working... %d\n", i);
        sleep(1);
    }

    alarm(0);  // Cancel alarm
    printf("Alarm cancelled\n");
}

int main(int argc, char *argv[]) {
    printf("Linux System Programming - Signal Handling\n");
    printf("=========================================\n\n");

    if (argc > 1) {
        int example = atoi(argv[1]);
        switch (example) {
            case 1: example_basic_signal(); break;
            case 2: example_multiple_signals(); break;
            case 3: example_sigaction(); break;
            case 4: example_signal_blocking(); break;
            case 5: example_alarm_signal(); break;
            default:
                printf("Usage: %s [1-5]\n", argv[0]);
                printf("1: Basic signal handling\n");
                printf("2: Multiple signals\n");
                printf("3: sigaction\n");
                printf("4: Signal blocking\n");
                printf("5: Alarm signal\n");
        }
    } else {
        printf("Running default examples...\n\n");
        example_sigaction();
    }

    return 0;
}

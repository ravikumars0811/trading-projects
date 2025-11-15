/*
 * wait_demo.c - Process Synchronization and Wait
 *
 * Demonstrates:
 * - wait() and waitpid()
 * - Collecting exit status
 * - Avoiding zombie processes
 * - Non-blocking wait
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void example_basic_wait() {
    printf("=== Example 1: Basic wait() ===\n");

    pid_t pid = fork();

    if (pid == 0) {
        printf("Child: Working for 2 seconds...\n");
        sleep(2);
        printf("Child: Done! Exiting with status 42\n");
        exit(42);
    } else {
        printf("Parent: Waiting for child (PID=%d)...\n", pid);
        int status;
        pid_t waited_pid = wait(&status);

        printf("Parent: Child (PID=%d) finished\n", waited_pid);

        if (WIFEXITED(status)) {
            printf("Parent: Child exited normally with status: %d\n",
                   WEXITSTATUS(status));
        }
    }
    printf("\n");
}

void example_waitpid() {
    printf("=== Example 2: waitpid() - Wait for specific child ===\n");

    pid_t pid1 = fork();
    if (pid1 == 0) {
        printf("Child 1: Sleeping 1 second\n");
        sleep(1);
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        printf("Child 2: Sleeping 3 seconds\n");
        sleep(3);
        exit(2);
    }

    // Wait for specific child (child 2)
    printf("Parent: Waiting specifically for child 2 (PID=%d)...\n", pid2);
    int status;
    waitpid(pid2, &status, 0);
    printf("Parent: Child 2 finished with status: %d\n", WEXITSTATUS(status));

    // Wait for child 1
    waitpid(pid1, &status, 0);
    printf("Parent: Child 1 finished with status: %d\n", WEXITSTATUS(status));

    printf("\n");
}

void example_zombie_prevention() {
    printf("=== Example 3: Preventing Zombie Processes ===\n");

    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            printf("Child %d: PID=%d, exiting\n", i + 1, getpid());
            exit(i);
        }
    }

    // Parent waits for all children
    printf("Parent: Waiting for all children to prevent zombies...\n");
    int status;
    pid_t pid;

    while ((pid = wait(&status)) > 0) {
        printf("Parent: Reaped child PID=%d, status=%d\n",
               pid, WEXITSTATUS(status));
    }

    printf("Parent: All children reaped, no zombies!\n\n");
}

void example_nonblocking_wait() {
    printf("=== Example 4: Non-blocking wait with WNOHANG ===\n");

    pid_t pid = fork();

    if (pid == 0) {
        printf("Child: Will sleep for 3 seconds\n");
        sleep(3);
        exit(0);
    } else {
        printf("Parent: Checking child status without blocking...\n");

        int status;
        pid_t result;

        // Try to wait without blocking
        for (int i = 0; i < 5; i++) {
            result = waitpid(pid, &status, WNOHANG);

            if (result == 0) {
                printf("Parent: Child still running (check %d)\n", i + 1);
                sleep(1);
            } else if (result == pid) {
                printf("Parent: Child finished!\n");
                break;
            } else {
                perror("waitpid");
                break;
            }
        }
    }
    printf("\n");
}

void example_signal_termination() {
    printf("=== Example 5: Child terminated by signal ===\n");

    pid_t pid = fork();

    if (pid == 0) {
        printf("Child: Sleeping, waiting for signal...\n");
        sleep(100);  // Sleep a long time
        exit(0);
    } else {
        sleep(1);
        printf("Parent: Sending SIGTERM to child\n");
        kill(pid, SIGTERM);

        int status;
        wait(&status);

        if (WIFSIGNALED(status)) {
            printf("Parent: Child terminated by signal: %d\n",
                   WTERMSIG(status));
        }
    }
    printf("\n");
}

void example_multiple_children() {
    printf("=== Example 6: Managing multiple children ===\n");

    #define NUM_CHILDREN 5
    pid_t children[NUM_CHILDREN];

    // Create multiple children
    for (int i = 0; i < NUM_CHILDREN; i++) {
        children[i] = fork();

        if (children[i] == 0) {
            // Child process
            printf("Child %d: PID=%d starting\n", i + 1, getpid());
            sleep(1 + (i % 3));  // Sleep 1-3 seconds
            printf("Child %d: PID=%d finishing\n", i + 1, getpid());
            exit(i * 10);
        }
    }

    // Parent waits for all children and collects status
    printf("\nParent: Waiting for all %d children...\n", NUM_CHILDREN);

    int status;
    pid_t pid;
    int count = 0;

    while ((pid = wait(&status)) > 0) {
        count++;
        if (WIFEXITED(status)) {
            printf("Parent: Child PID=%d exited with status=%d (%d/%d)\n",
                   pid, WEXITSTATUS(status), count, NUM_CHILDREN);
        }
    }

    printf("Parent: All children completed!\n\n");
}

int main() {
    printf("Linux System Programming - Process Wait Examples\n");
    printf("===============================================\n\n");

    example_basic_wait();
    example_waitpid();
    example_zombie_prevention();
    example_nonblocking_wait();
    example_signal_termination();
    example_multiple_children();

    printf("All examples completed!\n");
    printf("\nKey Takeaways:\n");
    printf("- Always wait() for child processes to avoid zombies\n");
    printf("- WIFEXITED/WEXITSTATUS macros extract exit status\n");
    printf("- waitpid() allows waiting for specific child\n");
    printf("- WNOHANG option enables non-blocking wait\n");
    printf("- WIFSIGNALED/WTERMSIG detect signal termination\n");

    return 0;
}

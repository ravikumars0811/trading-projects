/*
 * fork_basic.c - Basic Process Creation with fork()
 *
 * Demonstrates:
 * - Understanding fork() behavior
 * - Parent and child process execution
 * - Process IDs and relationships
 * - Memory independence
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void example_simple_fork() {
    printf("=== Example 1: Simple fork() ===\n");
    printf("Before fork: PID=%d\n", getpid());

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("CHILD:  PID=%d, Parent PID=%d\n", getpid(), getppid());
        printf("CHILD:  fork() returned: %d\n", pid);
    } else {
        // Parent process
        printf("PARENT: PID=%d, Parent PID=%d\n", getpid(), getppid());
        printf("PARENT: fork() returned: %d (child's PID)\n", pid);
        wait(NULL);  // Wait for child to finish
    }

    printf("Both processes continue here: PID=%d\n\n", getpid());
}

void example_memory_independence() {
    printf("=== Example 2: Memory Independence ===\n");

    int shared_var = 100;
    printf("Before fork: shared_var = %d\n", shared_var);

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // Child process modifies variable
        shared_var = 200;
        printf("CHILD:  Modified shared_var = %d (PID=%d)\n",
               shared_var, getpid());
        sleep(1);
        printf("CHILD:  After sleep, shared_var = %d\n", shared_var);
    } else {
        // Parent process modifies variable
        sleep(1);  // Let child modify first
        shared_var = 300;
        printf("PARENT: Modified shared_var = %d (PID=%d)\n",
               shared_var, getpid());
        wait(NULL);
        printf("PARENT: After child exits, shared_var = %d\n", shared_var);
    }
    printf("\n");
}

void example_multiple_forks() {
    printf("=== Example 3: Multiple fork() calls ===\n");
    printf("Initial process: PID=%d\n", getpid());

    pid_t pid1 = fork();

    if (pid1 == 0) {
        // First child
        printf("First child:  PID=%d, Parent=%d\n", getpid(), getppid());
        exit(0);
    }

    pid_t pid2 = fork();

    if (pid2 == 0) {
        // Second child
        printf("Second child: PID=%d, Parent=%d\n", getpid(), getppid());
        exit(0);
    }

    // Parent waits for both children
    printf("Parent waiting for children...\n");
    wait(NULL);
    wait(NULL);
    printf("Parent: Both children completed\n\n");
}

void example_fork_chain() {
    printf("=== Example 4: Fork Chain (Exponential Growth) ===\n");
    printf("WARNING: This creates 2^n processes\n");

    int depth = 3;  // Creates 8 processes total
    printf("Creating fork chain with depth=%d\n", depth);

    for (int i = 0; i < depth; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Child continues forking
            printf("Process PID=%d, Parent=%d, Level=%d\n",
                   getpid(), getppid(), i + 1);
        } else {
            // Parent continues forking
            printf("Process PID=%d created child PID=%d at level=%d\n",
                   getpid(), pid, i + 1);
        }
    }

    // All processes reach here
    printf("PID=%d: Finished at depth %d\n", getpid(), depth);

    // Wait for any children
    while (wait(NULL) > 0);

    printf("\n");
}

void example_fork_return_values() {
    printf("=== Example 5: Understanding fork() Return Values ===\n");

    pid_t pid = fork();

    if (pid == -1) {
        // Error case
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process: fork() returns 0
        printf("I am CHILD process:\n");
        printf("  My PID: %d\n", getpid());
        printf("  My parent's PID: %d\n", getppid());
        printf("  fork() returned: %d\n", pid);
        printf("  How I know I'm the child: fork() returned 0\n");
    } else {
        // Parent process: fork() returns child's PID
        printf("I am PARENT process:\n");
        printf("  My PID: %d\n", getpid());
        printf("  My parent's PID: %d\n", getppid());
        printf("  fork() returned: %d (my child's PID)\n", pid);
        printf("  How I know I'm the parent: fork() returned > 0\n");
        wait(NULL);
    }
    printf("\n");
}

void example_orphan_process() {
    printf("=== Example 6: Orphan Process ===\n");

    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        printf("CHILD:  PID=%d, Parent PID=%d\n", getpid(), getppid());
        sleep(2);  // Sleep while parent exits
        printf("CHILD:  After sleep, Parent PID=%d (adopted by init/systemd)\n",
               getppid());
        exit(0);
    } else {
        // Parent process exits immediately
        printf("PARENT: PID=%d, Child PID=%d\n", getpid(), pid);
        printf("PARENT: Exiting immediately, child becomes orphan\n");
        // Note: Not waiting for child, so it becomes orphan
    }
    printf("\n");
}

int main() {
    printf("Linux System Programming - Process Creation Examples\n");
    printf("====================================================\n\n");

    example_simple_fork();
    example_memory_independence();
    example_multiple_forks();
    example_fork_return_values();

    // Uncomment to see fork chain (creates many processes)
    // example_fork_chain();

    // Uncomment to see orphan process
    // example_orphan_process();

    printf("All examples completed!\n");
    return 0;
}

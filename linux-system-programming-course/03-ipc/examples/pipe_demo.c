/*
 * pipe_demo.c - Inter-Process Communication using Pipes
 *
 * Demonstrates:
 * - Creating pipes
 * - Parent-child communication
 * - Bidirectional communication
 * - Error handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void example_basic_pipe() {
    printf("=== Example 1: Basic Pipe Communication ===\n");

    int pipefd[2];
    pid_t pid;
    char buffer[100];

    // Create pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Child process - reads from pipe
        close(pipefd[1]);  // Close write end

        ssize_t n = read(pipefd[0], buffer, sizeof(buffer));
        buffer[n] = '\0';

        printf("Child received: %s\n", buffer);

        close(pipefd[0]);
        exit(0);
    } else {
        // Parent process - writes to pipe
        close(pipefd[0]);  // Close read end

        const char *message = "Hello from parent!";
        write(pipefd[1], message, strlen(message));

        printf("Parent sent: %s\n", message);

        close(pipefd[1]);
        wait(NULL);
    }
    printf("\n");
}

void example_bidirectional_pipe() {
    printf("=== Example 2: Bidirectional Communication (Two Pipes) ===\n");

    int pipe1[2], pipe2[2];  // pipe1: parent->child, pipe2: child->parent
    pid_t pid;
    char buffer[100];

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        return;
    }

    pid = fork();

    if (pid == 0) {
        // Child process
        close(pipe1[1]);  // Close write end of pipe1
        close(pipe2[0]);  // Close read end of pipe2

        // Read from parent
        read(pipe1[0], buffer, sizeof(buffer));
        printf("Child received: %s\n", buffer);

        // Send response to parent
        const char *response = "ACK from child";
        write(pipe2[1], response, strlen(response));

        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
    } else {
        // Parent process
        close(pipe1[0]);  // Close read end of pipe1
        close(pipe2[1]);  // Close write end of pipe2

        // Send to child
        const char *message = "Message from parent";
        write(pipe1[1], message, strlen(message));
        printf("Parent sent: %s\n", message);

        // Read response from child
        ssize_t n = read(pipe2[0], buffer, sizeof(buffer));
        buffer[n] = '\0';
        printf("Parent received: %s\n", buffer);

        close(pipe1[1]);
        close(pipe2[0]);
        wait(NULL);
    }
    printf("\n");
}

void example_pipe_as_filter() {
    printf("=== Example 3: Using Pipe to Filter Data ===\n");

    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid = fork();

    if (pid == 0) {
        // Child process - act as filter (convert to uppercase)
        close(pipefd[1]);

        char c;
        while (read(pipefd[0], &c, 1) > 0) {
            if (c >= 'a' && c <= 'z') {
                c = c - 'a' + 'A';
            }
            write(STDOUT_FILENO, &c, 1);
        }

        printf("\n");
        close(pipefd[0]);
        exit(0);
    } else {
        // Parent process - send data
        close(pipefd[0]);

        const char *text = "hello world";
        printf("Parent sending: %s\n", text);
        printf("Child output:   ");
        fflush(stdout);

        write(pipefd[1], text, strlen(text));

        close(pipefd[1]);
        wait(NULL);
    }
    printf("\n");
}

int main() {
    printf("Linux System Programming - Pipe Communication\n");
    printf("============================================\n\n");

    example_basic_pipe();
    example_bidirectional_pipe();
    example_pipe_as_filter();

    printf("Key Takeaways:\n");
    printf("- Pipes provide unidirectional communication\n");
    printf("- Always close unused pipe ends\n");
    printf("- Use two pipes for bidirectional communication\n");
    printf("- Pipes are useful for parent-child IPC\n");

    return 0;
}

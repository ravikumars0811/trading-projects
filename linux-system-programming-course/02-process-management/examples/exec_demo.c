/*
 * exec_demo.c - Demonstrating exec() Family
 *
 * Demonstrates:
 * - Different exec() variants (execl, execlp, execv, execvp)
 * - Replacing process image
 * - Command execution
 * - Error handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void example_execl() {
    printf("=== Example 1: execl() - Execute with argument list ===\n");

    pid_t pid = fork();

    if (pid == 0) {
        printf("Child: About to execute 'ls -l /tmp'\n");

        // execl: full path, arguments as list, terminated by NULL
        execl("/bin/ls", "ls", "-l", "/tmp", NULL);

        // If we reach here, exec failed
        perror("execl failed");
        exit(1);
    } else {
        wait(NULL);
        printf("\n");
    }
}

void example_execlp() {
    printf("=== Example 2: execlp() - Execute with PATH search ===\n");

    pid_t pid = fork();

    if (pid == 0) {
        printf("Child: Executing 'echo' using PATH\n");

        // execlp: searches PATH, arguments as list
        execlp("echo", "echo", "Hello from execlp!", NULL);

        perror("execlp failed");
        exit(1);
    } else {
        wait(NULL);
        printf("\n");
    }
}

void example_execv() {
    printf("=== Example 3: execv() - Execute with argument vector ===\n");

    pid_t pid = fork();

    if (pid == 0) {
        // Prepare argument vector
        char *args[] = {
            "ls",
            "-l",
            "-h",
            "/home",
            NULL  // Must be NULL-terminated
        };

        printf("Child: Executing 'ls -l -h /home'\n");

        // execv: full path, arguments as array
        execv("/bin/ls", args);

        perror("execv failed");
        exit(1);
    } else {
        wait(NULL);
        printf("\n");
    }
}

void example_execvp() {
    printf("=== Example 4: execvp() - Most commonly used ===\n");

    pid_t pid = fork();

    if (pid == 0) {
        // Build command arguments dynamically
        char *args[] = {
            "ps",
            "aux",
            NULL
        };

        printf("Child: Executing 'ps aux' (showing first 10 lines)\n");

        // execvp: searches PATH, arguments as array
        execvp("ps", args);

        perror("execvp failed");
        exit(1);
    } else {
        wait(NULL);
        printf("\n");
    }
}

void example_exec_with_environment() {
    printf("=== Example 5: execle() - With custom environment ===\n");

    pid_t pid = fork();

    if (pid == 0) {
        // Custom environment variables
        char *env[] = {
            "PATH=/bin:/usr/bin",
            "USER=customuser",
            "CUSTOM_VAR=Hello",
            NULL
        };

        printf("Child: Executing /bin/env with custom environment\n");

        // execle: arguments as list, custom environment
        execle("/bin/env", "env", NULL, env);

        perror("execle failed");
        exit(1);
    } else {
        wait(NULL);
        printf("\n");
    }
}

void example_shell_command() {
    printf("=== Example 6: Executing shell commands ===\n");

    pid_t pid = fork();

    if (pid == 0) {
        printf("Child: Executing complex shell command\n");

        // Execute shell to run complex command
        execl("/bin/sh", "sh", "-c",
              "echo 'Files in current directory:' && ls -1 | head -5",
              NULL);

        perror("execl failed");
        exit(1);
    } else {
        wait(NULL);
        printf("\n");
    }
}

void example_exec_error_handling() {
    printf("=== Example 7: Error handling with exec() ===\n");

    pid_t pid = fork();

    if (pid == 0) {
        printf("Child: Attempting to execute non-existent program\n");

        // Try to execute non-existent program
        execl("/bin/nonexistent", "nonexistent", NULL);

        // This will execute if exec fails
        perror("exec failed");
        fprintf(stderr, "Could not execute: /bin/nonexistent\n");
        exit(127);  // Convention: 127 for command not found
    } else {
        int status;
        wait(&status);

        if (WIFEXITED(status)) {
            printf("Child exited with status: %d\n", WEXITSTATUS(status));
        }
        printf("\n");
    }
}

// Simple program launcher
void launch_program(const char *program, char *args[]) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Child: execute the program
        execvp(program, args);

        // If exec fails
        fprintf(stderr, "Failed to execute: %s\n", program);
        perror("execvp");
        exit(1);
    } else {
        // Parent: wait for child
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Program exited with status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Program terminated by signal: %d\n", WTERMSIG(status));
        }
    }
}

void example_program_launcher() {
    printf("=== Example 8: Generic Program Launcher ===\n");

    // Launch date command
    char *date_args[] = {"date", "+%Y-%m-%d %H:%M:%S", NULL};
    printf("Launching: date\n");
    launch_program("date", date_args);

    // Launch whoami command
    char *whoami_args[] = {"whoami", NULL};
    printf("\nLaunching: whoami\n");
    launch_program("whoami", whoami_args);

    // Launch pwd command
    char *pwd_args[] = {"pwd", NULL};
    printf("\nLaunching: pwd\n");
    launch_program("pwd", pwd_args);

    printf("\n");
}

int main() {
    printf("Linux System Programming - exec() Family Examples\n");
    printf("=================================================\n\n");

    example_execl();
    example_execlp();
    example_execv();

    // Uncomment to see more examples (generates more output)
    // example_execvp();
    // example_exec_with_environment();

    example_shell_command();
    example_exec_error_handling();
    example_program_launcher();

    printf("All examples completed!\n");
    printf("\nKey Takeaways:\n");
    printf("- execl/execlp: Arguments as separate parameters\n");
    printf("- execv/execvp: Arguments as array\n");
    printf("- *p variants: Search PATH for executable\n");
    printf("- *e variants: Allow custom environment\n");
    printf("- exec() only returns on error\n");

    return 0;
}

/*
 * shared_memory.c - IPC using Shared Memory
 *
 * Demonstrates:
 * - Creating shared memory segments
 * - Attaching/detaching shared memory
 * - Process synchronization with shared memory
 * - Cleanup
 *
 * Compile: gcc shared_memory.c -o shared_memory
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SHM_SIZE 1024
#define SHM_KEY 1234

typedef struct {
    int counter;
    char message[256];
} SharedData;

int main() {
    printf("=== Shared Memory IPC Demo ===\n\n");

    int shmid;
    SharedData *shared_data;

    // Create shared memory segment
    shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    printf("Created shared memory segment: ID=%d\n", shmid);

    // Attach shared memory
    shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    // Initialize shared data
    shared_data->counter = 0;
    strcpy(shared_data->message, "Initial message");

    printf("Initial data: counter=%d, message='%s'\n\n",
           shared_data->counter, shared_data->message);

    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        printf("Child: Accessing shared memory\n");

        // Modify shared data
        shared_data->counter = 100;
        strcpy(shared_data->message, "Modified by child");

        printf("Child: counter=%d, message='%s'\n",
               shared_data->counter, shared_data->message);

        // Detach shared memory
        shmdt(shared_data);
        exit(0);
    } else {
        // Parent process
        wait(NULL);  // Wait for child to complete

        printf("\nParent: After child modification\n");
        printf("Parent: counter=%d, message='%s'\n",
               shared_data->counter, shared_data->message);

        // Detach shared memory
        shmdt(shared_data);

        // Remove shared memory segment
        shmctl(shmid, IPC_RMID, NULL);
        printf("\nShared memory segment removed\n");
    }

    printf("\nKey Points:\n");
    printf("- Shared memory is fastest IPC mechanism\n");
    printf("- No data copying between processes\n");
    printf("- Requires synchronization for concurrent access\n");
    printf("- Always cleanup with IPC_RMID\n");

    return 0;
}

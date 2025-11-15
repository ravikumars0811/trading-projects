/*
 * thread_basic.c - Basic Threading with pthreads
 *
 * Demonstrates:
 * - Creating threads
 * - Joining threads
 * - Passing arguments to threads
 * - Returning values from threads
 *
 * Compile: gcc thread_basic.c -o thread_basic -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// Simple thread function
void* simple_thread(void *arg) {
    int thread_num = *(int*)arg;
    printf("Thread %d: Starting (TID=%lu)\n", thread_num, pthread_self());

    sleep(1);

    printf("Thread %d: Finishing\n", thread_num);
    return NULL;
}

void example_basic_threads() {
    printf("=== Example 1: Basic Thread Creation ===\n");

    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};

    // Create threads
    for (int i = 0; i < 3; i++) {
        int ret = pthread_create(&threads[i], NULL, simple_thread, &thread_ids[i]);
        if (ret != 0) {
            fprintf(stderr, "Error creating thread: %d\n", ret);
            exit(1);
        }
        printf("Main: Created thread %d\n", thread_ids[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
        printf("Main: Thread %d joined\n", thread_ids[i]);
    }

    printf("\n");
}

// Thread that returns a value
void* calculator_thread(void *arg) {
    int n = *(int*)arg;
    int *result = malloc(sizeof(int));

    *result = n * n;  // Calculate square

    printf("Calculator: %d^2 = %d\n", n, *result);
    pthread_exit(result);  // Return result
}

void example_thread_return_value() {
    printf("=== Example 2: Thread Return Value ===\n");

    pthread_t thread;
    int input = 5;
    int *result;

    pthread_create(&thread, NULL, calculator_thread, &input);

    // Get return value
    pthread_join(thread, (void**)&result);

    printf("Main: Received result = %d\n", *result);

    free(result);
    printf("\n");
}

// Thread with struct argument
typedef struct {
    int id;
    char name[50];
    int iterations;
} ThreadData;

void* worker_thread(void *arg) {
    ThreadData *data = (ThreadData*)arg;

    printf("Worker %d (%s): Started\n", data->id, data->name);

    for (int i = 0; i < data->iterations; i++) {
        printf("Worker %d: Iteration %d/%d\n",
               data->id, i + 1, data->iterations);
        usleep(500000);  // 500ms
    }

    printf("Worker %d: Completed\n", data->id);
    return NULL;
}

void example_thread_with_struct() {
    printf("=== Example 3: Passing Struct to Thread ===\n");

    pthread_t threads[2];
    ThreadData data[2] = {
        {1, "Alpha", 3},
        {2, "Beta", 3}
    };

    for (int i = 0; i < 2; i++) {
        pthread_create(&threads[i], NULL, worker_thread, &data[i]);
    }

    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n");
}

// Detached thread
void* detached_thread(void *arg) {
    printf("Detached thread: Running independently\n");
    sleep(2);
    printf("Detached thread: Finishing\n");
    return NULL;
}

void example_detached_thread() {
    printf("=== Example 4: Detached Thread ===\n");

    pthread_t thread;
    pthread_attr_t attr;

    // Initialize thread attributes
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // Create detached thread
    pthread_create(&thread, &attr, detached_thread, NULL);

    pthread_attr_destroy(&attr);

    printf("Main: Created detached thread (cannot join)\n");
    printf("Main: Continuing work...\n");

    sleep(3);  // Give detached thread time to finish

    printf("\n");
}

// Race condition demonstration (without synchronization)
int shared_counter = 0;

void* increment_thread(void *arg) {
    int iterations = *(int*)arg;

    for (int i = 0; i < iterations; i++) {
        shared_counter++;  // RACE CONDITION!
    }

    return NULL;
}

void example_race_condition() {
    printf("=== Example 5: Race Condition (Problem) ===\n");

    pthread_t threads[5];
    int iterations = 100000;

    shared_counter = 0;

    // Create multiple threads incrementing shared counter
    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, increment_thread, &iterations);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Expected: %d\n", iterations * 5);
    printf("Actual:   %d\n", shared_counter);
    printf("Race condition occurred! (See next module for solution)\n\n");
}

int main() {
    printf("Linux System Programming - Basic Threading\n");
    printf("==========================================\n\n");

    example_basic_threads();
    example_thread_return_value();
    example_thread_with_struct();
    example_detached_thread();
    example_race_condition();

    printf("Key Takeaways:\n");
    printf("- Use pthread_create() to create threads\n");
    printf("- Use pthread_join() to wait for thread completion\n");
    printf("- Threads share process memory\n");
    printf("- Synchronization needed for shared data\n");

    return 0;
}

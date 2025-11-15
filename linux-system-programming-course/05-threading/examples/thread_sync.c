/*
 * thread_sync.c - Thread Synchronization with Mutexes
 *
 * Demonstrates:
 * - Using mutexes for synchronization
 * - Protecting shared data
 * - Condition variables
 * - Producer-consumer pattern
 *
 * Compile: gcc thread_sync.c -o thread_sync -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Example 1: Mutex protected counter
int counter = 0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void* safe_increment(void *arg) {
    int iterations = *(int*)arg;

    for (int i = 0; i < iterations; i++) {
        pthread_mutex_lock(&counter_mutex);
        counter++;
        pthread_mutex_unlock(&counter_mutex);
    }

    return NULL;
}

void example_mutex_protection() {
    printf("=== Example 1: Mutex Protection ===\n");

    pthread_t threads[5];
    int iterations = 100000;

    counter = 0;

    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, safe_increment, &iterations);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Expected: %d\n", iterations * 5);
    printf("Actual:   %d\n", counter);
    printf("Success! No race condition with mutex.\n\n");
}

// Example 2: Producer-Consumer with condition variable
#define BUFFER_SIZE 10

int buffer[BUFFER_SIZE];
int count = 0;
int in = 0;
int out = 0;

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void* producer(void *arg) {
    int id = *(int*)arg;

    for (int i = 0; i < 20; i++) {
        int item = id * 100 + i;

        pthread_mutex_lock(&buffer_mutex);

        // Wait while buffer is full
        while (count == BUFFER_SIZE) {
            printf("Producer %d: Buffer full, waiting...\n", id);
            pthread_cond_wait(&not_full, &buffer_mutex);
        }

        // Produce item
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;

        printf("Producer %d: Produced item %d (count=%d)\n",
               id, item, count);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&buffer_mutex);

        usleep(50000);  // 50ms
    }

    return NULL;
}

void* consumer(void *arg) {
    int id = *(int*)arg;

    for (int i = 0; i < 20; i++) {
        pthread_mutex_lock(&buffer_mutex);

        // Wait while buffer is empty
        while (count == 0) {
            printf("Consumer %d: Buffer empty, waiting...\n", id);
            pthread_cond_wait(&not_empty, &buffer_mutex);
        }

        // Consume item
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;

        printf("Consumer %d: Consumed item %d (count=%d)\n",
               id, item, count);

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&buffer_mutex);

        usleep(100000);  // 100ms
    }

    return NULL;
}

void example_producer_consumer() {
    printf("=== Example 2: Producer-Consumer Pattern ===\n");

    pthread_t prod_threads[2], cons_threads[2];
    int prod_ids[] = {1, 2};
    int cons_ids[] = {1, 2};

    // Create producers
    for (int i = 0; i < 2; i++) {
        pthread_create(&prod_threads[i], NULL, producer, &prod_ids[i]);
    }

    // Create consumers
    for (int i = 0; i < 2; i++) {
        pthread_create(&cons_threads[i], NULL, consumer, &cons_ids[i]);
    }

    // Wait for all threads
    for (int i = 0; i < 2; i++) {
        pthread_join(prod_threads[i], NULL);
        pthread_join(cons_threads[i], NULL);
    }

    printf("Producer-Consumer completed!\n\n");
}

// Example 3: Bank account (demonstrates critical section)
typedef struct {
    int balance;
    pthread_mutex_t lock;
} BankAccount;

void account_init(BankAccount *acc, int initial) {
    acc->balance = initial;
    pthread_mutex_init(&acc->lock, NULL);
}

void account_deposit(BankAccount *acc, int amount) {
    pthread_mutex_lock(&acc->lock);
    acc->balance += amount;
    printf("Deposited %d, New balance: %d\n", amount, acc->balance);
    pthread_mutex_unlock(&acc->lock);
}

int account_withdraw(BankAccount *acc, int amount) {
    pthread_mutex_lock(&acc->lock);

    if (acc->balance >= amount) {
        acc->balance -= amount;
        printf("Withdrew %d, New balance: %d\n", amount, acc->balance);
        pthread_mutex_unlock(&acc->lock);
        return 1;
    } else {
        printf("Insufficient funds to withdraw %d (balance: %d)\n",
               amount, acc->balance);
        pthread_mutex_unlock(&acc->lock);
        return 0;
    }
}

BankAccount account;

void* transaction_thread(void *arg) {
    int id = *(int*)arg;

    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            account_deposit(&account, 100);
        } else {
            account_withdraw(&account, 50);
        }
        usleep(100000);
    }

    return NULL;
}

void example_bank_account() {
    printf("=== Example 3: Bank Account (Critical Section) ===\n");

    account_init(&account, 1000);
    printf("Initial balance: %d\n\n", account.balance);

    pthread_t threads[3];
    int ids[] = {1, 2, 3};

    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, transaction_thread, &ids[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nFinal balance: %d\n\n", account.balance);
    pthread_mutex_destroy(&account.lock);
}

int main() {
    printf("Linux System Programming - Thread Synchronization\n");
    printf("================================================\n\n");

    example_mutex_protection();
    example_producer_consumer();
    example_bank_account();

    printf("Key Takeaways:\n");
    printf("- Use mutexes to protect shared data\n");
    printf("- Condition variables for thread communication\n");
    printf("- Always unlock mutexes\n");
    printf("- Keep critical sections small\n");

    // Cleanup
    pthread_mutex_destroy(&counter_mutex);
    pthread_mutex_destroy(&buffer_mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    return 0;
}

/*
 * CPU Affinity Example
 *
 * Demonstrates:
 * - Setting CPU affinity for threads
 * - NUMA-aware thread placement
 * - Performance impact of affinity
 * - Core isolation
 *
 * Build: gcc -O2 -o thread_affinity thread_affinity.c -lpthread -lnuma
 * Run: ./thread_affinity
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <numa.h>

#define NUM_THREADS 4
#define ITERATIONS 100000000

/* Thread data structure */
struct thread_data {
    int thread_id;
    int cpu_id;
    uint64_t counter;
    double time_ms;
};

/* Get time in microseconds */
static uint64_t get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

/* Worker function */
static void *worker_thread(void *arg)
{
    struct thread_data *data = (struct thread_data *)arg;
    uint64_t start, end;

    /* Set CPU affinity if specified */
    if (data->cpu_id >= 0) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(data->cpu_id, &cpuset);

        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
            perror("pthread_setaffinity_np");
            return NULL;
        }

        printf("Thread %d pinned to CPU %d\n", data->thread_id, data->cpu_id);
    } else {
        printf("Thread %d: no affinity set (free to migrate)\n", data->thread_id);
    }

    /* Wait a bit for all threads to be ready */
    usleep(100000);

    /* Do work */
    start = get_time_us();
    for (uint64_t i = 0; i < ITERATIONS; i++) {
        data->counter++;
    }
    end = get_time_us();

    data->time_ms = (end - start) / 1000.0;

    return NULL;
}

/* Test without affinity */
static void test_without_affinity(void)
{
    pthread_t threads[NUM_THREADS];
    struct thread_data data[NUM_THREADS];
    double total_time = 0;

    printf("\n=== Test WITHOUT CPU Affinity ===\n");
    printf("Threads can migrate between CPUs\n\n");

    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        data[i].thread_id = i;
        data[i].cpu_id = -1;  /* No affinity */
        data[i].counter = 0;

        pthread_create(&threads[i], NULL, worker_thread, &data[i]);
    }

    /* Wait for completion */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %d: %.2f ms, counter=%lu\n",
               i, data[i].time_ms, data[i].counter);
        total_time += data[i].time_ms;
    }

    printf("Average time: %.2f ms\n", total_time / NUM_THREADS);
}

/* Test with affinity */
static void test_with_affinity(void)
{
    pthread_t threads[NUM_THREADS];
    struct thread_data data[NUM_THREADS];
    double total_time = 0;

    printf("\n=== Test WITH CPU Affinity ===\n");
    printf("Each thread pinned to specific CPU\n\n");

    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        data[i].thread_id = i;
        data[i].cpu_id = i;  /* Pin to CPU i */
        data[i].counter = 0;

        pthread_create(&threads[i], NULL, worker_thread, &data[i]);
    }

    /* Wait for completion */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %d (CPU %d): %.2f ms, counter=%lu\n",
               i, data[i].cpu_id, data[i].time_ms, data[i].counter);
        total_time += data[i].time_ms;
    }

    printf("Average time: %.2f ms\n", total_time / NUM_THREADS);
}

/* Display CPU topology */
static void display_cpu_topology(void)
{
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    printf("\n=== CPU Topology ===\n");
    printf("Number of CPUs: %d\n", num_cpus);

    if (numa_available() >= 0) {
        int num_nodes = numa_num_configured_nodes();
        printf("NUMA nodes: %d\n", num_nodes);

        for (int node = 0; node < num_nodes; node++) {
            struct bitmask *cpus = numa_allocate_cpumask();
            numa_node_to_cpus(node, cpus);

            printf("NUMA node %d CPUs:", node);
            for (int cpu = 0; cpu < num_cpus; cpu++) {
                if (numa_bitmask_isbitset(cpus, cpu)) {
                    printf(" %d", cpu);
                }
            }
            printf("\n");

            numa_free_cpumask(cpus);
        }
    } else {
        printf("NUMA not available\n");
    }
}

/* Set scheduling policy */
static void set_realtime_priority(void)
{
    struct sched_param param;
    param.sched_priority = 50;

    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("sched_setscheduler (run as root for RT priority)");
    } else {
        printf("\nSet real-time scheduling priority: SCHED_FIFO, priority 50\n");
    }
}

int main(void)
{
    printf("CPU Affinity and Thread Pinning Example\n");
    printf("========================================\n");

    /* Display system information */
    display_cpu_topology();

    /* Optional: Set real-time priority (requires root) */
    set_realtime_priority();

    /* Run tests */
    test_without_affinity();
    test_with_affinity();

    /* Key takeaways */
    printf("\n=== Key Takeaways ===\n");
    printf("1. CPU affinity reduces cache misses (less migration)\n");
    printf("2. Pin threads to specific cores for consistent performance\n");
    printf("3. Be aware of NUMA topology for memory allocation\n");
    printf("4. Use taskset/numactl to control affinity from shell\n");
    printf("5. RT scheduling policies need root privileges\n");
    printf("\nCommands:\n");
    printf("  taskset -c 0-3 ./thread_affinity    # Run on CPUs 0-3\n");
    printf("  numactl --cpunodebind=0 ./program   # Bind to NUMA node 0\n");

    return 0;
}

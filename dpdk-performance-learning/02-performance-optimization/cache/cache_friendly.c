/*
 * Cache-Friendly Programming Examples
 *
 * This demonstrates:
 * - Cache line effects
 * - Structure layout optimization
 * - Array of Structures vs Structure of Arrays
 * - False sharing prevention
 * - Data alignment
 *
 * Build: gcc -O2 -o cache_friendly cache_friendly.c -lm
 * Run: ./cache_friendly
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define CACHE_LINE_SIZE 64
#define NUM_ELEMENTS (1024 * 1024)
#define NUM_ITERATIONS 100

/* Get time in microseconds */
static uint64_t get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

/* Example 1: Array of Structures (AoS) - BAD for cache */
struct particle_aos {
    float x, y, z;      /* Position */
    float vx, vy, vz;   /* Velocity */
    float mass;
    float charge;
} __attribute__((aligned(CACHE_LINE_SIZE)));

/* Example 2: Structure of Arrays (SoA) - GOOD for cache */
struct particles_soa {
    float *x, *y, *z;       /* Position arrays */
    float *vx, *vy, *vz;    /* Velocity arrays */
    float *mass;
    float *charge;
    size_t count;
} __attribute__((aligned(CACHE_LINE_SIZE)));

/* Initialize AoS */
static void init_aos(struct particle_aos *particles, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        particles[i].x = (float)i;
        particles[i].y = (float)i * 2;
        particles[i].z = (float)i * 3;
        particles[i].vx = 1.0f;
        particles[i].vy = 1.0f;
        particles[i].vz = 1.0f;
        particles[i].mass = 1.0f;
        particles[i].charge = 1.0f;
    }
}

/* Initialize SoA */
static void init_soa(struct particles_soa *particles, size_t n)
{
    particles->count = n;
    particles->x = aligned_alloc(CACHE_LINE_SIZE, n * sizeof(float));
    particles->y = aligned_alloc(CACHE_LINE_SIZE, n * sizeof(float));
    particles->z = aligned_alloc(CACHE_LINE_SIZE, n * sizeof(float));
    particles->vx = aligned_alloc(CACHE_LINE_SIZE, n * sizeof(float));
    particles->vy = aligned_alloc(CACHE_LINE_SIZE, n * sizeof(float));
    particles->vz = aligned_alloc(CACHE_LINE_SIZE, n * sizeof(float));
    particles->mass = aligned_alloc(CACHE_LINE_SIZE, n * sizeof(float));
    particles->charge = aligned_alloc(CACHE_LINE_SIZE, n * sizeof(float));

    for (size_t i = 0; i < n; i++) {
        particles->x[i] = (float)i;
        particles->y[i] = (float)i * 2;
        particles->z[i] = (float)i * 3;
        particles->vx[i] = 1.0f;
        particles->vy[i] = 1.0f;
        particles->vz[i] = 1.0f;
        particles->mass[i] = 1.0f;
        particles->charge[i] = 1.0f;
    }
}

/* Free SoA */
static void free_soa(struct particles_soa *particles)
{
    free(particles->x);
    free(particles->y);
    free(particles->z);
    free(particles->vx);
    free(particles->vy);
    free(particles->vz);
    free(particles->mass);
    free(particles->charge);
}

/* Update positions - AoS version */
static void update_positions_aos(struct particle_aos *particles, size_t n, float dt)
{
    for (size_t i = 0; i < n; i++) {
        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;
        particles[i].z += particles[i].vz * dt;
    }
}

/* Update positions - SoA version */
static void update_positions_soa(struct particles_soa *particles, float dt)
{
    size_t n = particles->count;
    for (size_t i = 0; i < n; i++) {
        particles->x[i] += particles->vx[i] * dt;
        particles->y[i] += particles->vy[i] * dt;
        particles->z[i] += particles->vz[i] * dt;
    }
}

/* Benchmark AoS vs SoA */
static void benchmark_aos_vs_soa(void)
{
    struct particle_aos *aos;
    struct particles_soa soa;
    uint64_t start, end;
    double aos_time, soa_time;
    float dt = 0.01f;

    printf("\n=== AoS vs SoA Benchmark ===\n");
    printf("Elements: %d\n", NUM_ELEMENTS);
    printf("Iterations: %d\n", NUM_ITERATIONS);

    /* Allocate and initialize AoS */
    aos = aligned_alloc(CACHE_LINE_SIZE, NUM_ELEMENTS * sizeof(struct particle_aos));
    init_aos(aos, NUM_ELEMENTS);

    /* Benchmark AoS */
    start = get_time_us();
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        update_positions_aos(aos, NUM_ELEMENTS, dt);
    }
    end = get_time_us();
    aos_time = (end - start) / 1000.0;

    printf("\nAoS (Array of Structures):\n");
    printf("  Time: %.2f ms\n", aos_time);
    printf("  Structure size: %lu bytes\n", sizeof(struct particle_aos));
    printf("  Total memory: %.2f MB\n",
           (NUM_ELEMENTS * sizeof(struct particle_aos)) / (1024.0 * 1024.0));

    /* Initialize SoA */
    init_soa(&soa, NUM_ELEMENTS);

    /* Benchmark SoA */
    start = get_time_us();
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        update_positions_soa(&soa, dt);
    }
    end = get_time_us();
    soa_time = (end - start) / 1000.0;

    printf("\nSoA (Structure of Arrays):\n");
    printf("  Time: %.2f ms\n", soa_time);
    printf("  Total memory: %.2f MB\n",
           (NUM_ELEMENTS * 8 * sizeof(float)) / (1024.0 * 1024.0));

    printf("\nSpeedup: %.2fx\n", aos_time / soa_time);
    printf("Why SoA is faster:\n");
    printf("  - Better cache line utilization\n");
    printf("  - Sequential memory access\n");
    printf("  - Easier for compiler to vectorize\n");
    printf("  - No wasted cache lines for unused fields\n");

    /* Cleanup */
    free(aos);
    free_soa(&soa);
}

/* Example 3: Cache line effects */
struct counter_unaligned {
    uint64_t value;
};

struct counter_aligned {
    uint64_t value;
    char padding[CACHE_LINE_SIZE - sizeof(uint64_t)];
} __attribute__((aligned(CACHE_LINE_SIZE)));

/* Demonstrate false sharing */
static void demonstrate_false_sharing(void)
{
    printf("\n=== False Sharing Example ===\n");
    printf("Cache line size: %d bytes\n", CACHE_LINE_SIZE);

    struct counter_unaligned *unaligned;
    struct counter_aligned *aligned;

    unaligned = malloc(2 * sizeof(struct counter_unaligned));
    aligned = aligned_alloc(CACHE_LINE_SIZE, 2 * sizeof(struct counter_aligned));

    printf("\nUnaligned counters:\n");
    printf("  Counter[0] address: %p\n", (void *)&unaligned[0].value);
    printf("  Counter[1] address: %p\n", (void *)&unaligned[1].value);
    printf("  Distance: %ld bytes\n",
           (char *)&unaligned[1] - (char *)&unaligned[0]);
    printf("  Same cache line: %s\n",
           ((uintptr_t)&unaligned[0] / CACHE_LINE_SIZE ==
            (uintptr_t)&unaligned[1] / CACHE_LINE_SIZE) ? "YES (BAD!)" : "NO");

    printf("\nAligned counters:\n");
    printf("  Counter[0] address: %p\n", (void *)&aligned[0].value);
    printf("  Counter[1] address: %p\n", (void *)&aligned[1].value);
    printf("  Distance: %ld bytes\n",
           (char *)&aligned[1] - (char *)&aligned[0]);
    printf("  Same cache line: %s\n",
           ((uintptr_t)&aligned[0] / CACHE_LINE_SIZE ==
            (uintptr_t)&aligned[1] / CACHE_LINE_SIZE) ? "YES (BAD!)" : "NO (GOOD!)");

    printf("\nFalse sharing occurs when:\n");
    printf("  - Multiple threads access different variables\n");
    printf("  - Variables are on the same cache line\n");
    printf("  - At least one thread writes to its variable\n");
    printf("  - Causes cache line bouncing between cores\n");

    free(unaligned);
    free(aligned);
}

/* Example 4: Row-major vs Column-major access */
static void benchmark_access_patterns(void)
{
    const int SIZE = 1024;
    int **matrix;
    uint64_t start, end;
    double row_time, col_time;
    long long sum;

    printf("\n=== Memory Access Pattern Benchmark ===\n");
    printf("Matrix size: %dx%d\n", SIZE, SIZE);

    /* Allocate matrix */
    matrix = malloc(SIZE * sizeof(int *));
    for (int i = 0; i < SIZE; i++) {
        matrix[i] = aligned_alloc(CACHE_LINE_SIZE, SIZE * sizeof(int));
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
    }

    /* Row-major access (cache-friendly) */
    sum = 0;
    start = get_time_us();
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            sum += matrix[i][j];
        }
    }
    end = get_time_us();
    row_time = (end - start) / 1000.0;

    printf("\nRow-major access (i,j):\n");
    printf("  Time: %.2f ms\n", row_time);
    printf("  Sum: %lld\n", sum);

    /* Column-major access (cache-unfriendly) */
    sum = 0;
    start = get_time_us();
    for (int j = 0; j < SIZE; j++) {
        for (int i = 0; i < SIZE; i++) {
            sum += matrix[i][j];
        }
    }
    end = get_time_us();
    col_time = (end - start) / 1000.0;

    printf("\nColumn-major access (j,i):\n");
    printf("  Time: %.2f ms\n", col_time);
    printf("  Sum: %lld\n", sum);

    printf("\nSlowdown: %.2fx\n", col_time / row_time);
    printf("Reason: Column-major causes cache line thrashing\n");

    /* Cleanup */
    for (int i = 0; i < SIZE; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int main(void)
{
    printf("Cache-Friendly Programming Examples\n");
    printf("====================================\n");

    /* Run benchmarks */
    benchmark_aos_vs_soa();
    demonstrate_false_sharing();
    benchmark_access_patterns();

    printf("\n=== Key Takeaways ===\n");
    printf("1. SoA is usually faster than AoS for array processing\n");
    printf("2. Align data to cache line boundaries to avoid false sharing\n");
    printf("3. Access memory sequentially when possible\n");
    printf("4. Understand your data access patterns\n");
    printf("5. Measure cache performance with perf/vtune\n");

    return 0;
}

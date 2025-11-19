/*
 * Arena Allocator (Region-Based Memory Management)
 *
 * Demonstrates:
 * - Fast O(1) allocation
 * - Bulk deallocation
 * - Cache-friendly allocation
 * - No fragmentation
 *
 * Build: gcc -O2 -o arena_allocator arena_allocator.c
 * Run: ./arena_allocator
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>

#define ARENA_SIZE (16 * 1024 * 1024)  /* 16 MB */
#define ALIGNMENT 16

/* Arena structure */
struct arena {
    void *base;          /* Base address */
    size_t size;         /* Total size */
    size_t offset;       /* Current offset */
    size_t peak_usage;   /* Peak memory usage */
};

/* Align address to boundary */
static inline size_t align_up(size_t n, size_t align)
{
    return (n + align - 1) & ~(align - 1);
}

/* Create arena */
static struct arena *arena_create(size_t size)
{
    struct arena *arena = malloc(sizeof(struct arena));
    if (!arena)
        return NULL;

    /* Use mmap for large allocations */
    arena->base = mmap(NULL, size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (arena->base == MAP_FAILED) {
        free(arena);
        return NULL;
    }

    arena->size = size;
    arena->offset = 0;
    arena->peak_usage = 0;

    return arena;
}

/* Allocate from arena */
static void *arena_alloc(struct arena *arena, size_t size)
{
    /* Align allocation */
    size_t aligned_offset = align_up(arena->offset, ALIGNMENT);
    size_t aligned_size = align_up(size, ALIGNMENT);

    /* Check if we have space */
    if (aligned_offset + aligned_size > arena->size)
        return NULL;

    void *ptr = (char *)arena->base + aligned_offset;
    arena->offset = aligned_offset + aligned_size;

    /* Track peak usage */
    if (arena->offset > arena->peak_usage)
        arena->peak_usage = arena->offset;

    return ptr;
}

/* Reset arena (fast deallocation) */
static void arena_reset(struct arena *arena)
{
    arena->offset = 0;
    /* Memory remains allocated, just reuse it */
}

/* Destroy arena */
static void arena_destroy(struct arena *arena)
{
    if (arena) {
        munmap(arena->base, arena->size);
        free(arena);
    }
}

/* Get current usage */
static size_t arena_usage(struct arena *arena)
{
    return arena->offset;
}

/* Get time in microseconds */
static uint64_t get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

/* Benchmark: Arena allocator vs malloc */
static void benchmark_allocation(void)
{
    const int NUM_ALLOCS = 1000000;
    const size_t ALLOC_SIZE = 64;
    uint64_t start, end;
    double arena_time, malloc_time;

    printf("\n=== Allocation Benchmark ===\n");
    printf("Number of allocations: %d\n", NUM_ALLOCS);
    printf("Allocation size: %zu bytes\n", ALLOC_SIZE);

    /* Benchmark arena allocator */
    struct arena *arena = arena_create(ARENA_SIZE);
    start = get_time_us();

    for (int i = 0; i < NUM_ALLOCS; i++) {
        void *ptr = arena_alloc(arena, ALLOC_SIZE);
        if (!ptr) {
            printf("Arena full at iteration %d\n", i);
            break;
        }
    }

    end = get_time_us();
    arena_time = (end - start) / 1000.0;

    printf("\nArena Allocator:\n");
    printf("  Time: %.2f ms\n", arena_time);
    printf("  Memory used: %.2f MB\n", arena_usage(arena) / (1024.0 * 1024.0));
    printf("  Allocations/sec: %.0f\n", NUM_ALLOCS / (arena_time / 1000.0));

    arena_destroy(arena);

    /* Benchmark malloc/free */
    void **ptrs = malloc(NUM_ALLOCS * sizeof(void *));
    start = get_time_us();

    for (int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = malloc(ALLOC_SIZE);
    }
    for (int i = 0; i < NUM_ALLOCS; i++) {
        free(ptrs[i]);
    }

    end = get_time_us();
    malloc_time = (end - start) / 1000.0;

    printf("\nmalloc/free:\n");
    printf("  Time: %.2f ms\n", malloc_time);
    printf("  Allocations/sec: %.0f\n", NUM_ALLOCS / (malloc_time / 1000.0));

    printf("\nSpeedup: %.2fx faster\n", malloc_time / arena_time);

    free(ptrs);
}

/* Example: String processing with arena */
struct string {
    char *data;
    size_t len;
};

static struct string *string_create(struct arena *arena, const char *str)
{
    struct string *s = arena_alloc(arena, sizeof(struct string));
    if (!s)
        return NULL;

    s->len = strlen(str);
    s->data = arena_alloc(arena, s->len + 1);
    if (!s->data)
        return NULL;

    strcpy(s->data, str);
    return s;
}

static void demonstrate_usage(void)
{
    printf("\n=== Usage Example ===\n");

    struct arena *arena = arena_create(1024 * 1024);  /* 1 MB */

    /* Allocate various objects */
    int *numbers = arena_alloc(arena, 100 * sizeof(int));
    for (int i = 0; i < 100; i++)
        numbers[i] = i;

    struct string *s1 = string_create(arena, "Hello, Arena!");
    struct string *s2 = string_create(arena, "Fast allocation");

    printf("String 1: %s (len=%zu)\n", s1->data, s1->len);
    printf("String 2: %s (len=%zu)\n", s2->data, s2->len);
    printf("Arena usage: %zu bytes\n", arena_usage(arena));

    /* Reset and reuse */
    arena_reset(arena);
    printf("\nAfter reset:\n");
    printf("Arena usage: %zu bytes\n", arena_usage(arena));

    /* Allocate again - memory is reused */
    struct string *s3 = string_create(arena, "Reused memory!");
    printf("String 3: %s\n", s3->data);

    arena_destroy(arena);
}

int main(void)
{
    printf("Arena Allocator Example\n");
    printf("=======================\n");

    demonstrate_usage();
    benchmark_allocation();

    printf("\n=== Key Advantages ===\n");
    printf("1. Very fast O(1) allocation (just pointer bump)\n");
    printf("2. No per-allocation overhead\n");
    printf("3. Excellent cache locality\n");
    printf("4. Instant bulk deallocation\n");
    printf("5. No fragmentation\n");
    printf("6. Perfect for request/response or per-frame allocation\n");

    printf("\n=== Use Cases ===\n");
    printf("- Request processing in web servers\n");
    printf("- Per-frame allocation in games\n");
    printf("- Compiler temporary objects\n");
    printf("- Parsing and AST construction\n");
    printf("- Any scenario with bulk deallocation\n");

    return 0;
}

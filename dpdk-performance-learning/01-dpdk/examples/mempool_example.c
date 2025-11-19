/*
 * DPDK Mempool Example
 *
 * This example demonstrates:
 * - Creating memory pools
 * - Allocating and freeing objects
 * - Bulk operations
 * - NUMA-aware allocation
 * - Performance measurement
 *
 * Build: gcc -o mempool_example mempool_example.c $(pkg-config --cflags --libs libdpdk)
 * Run: sudo ./mempool_example -l 0-3 -n 4
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <rte_eal.h>
#include <rte_mempool.h>
#include <rte_cycles.h>
#include <rte_lcore.h>

#define MEMPOOL_CACHE_SIZE 256
#define MEMPOOL_SIZE 8192
#define ELEMENT_SIZE 64
#define BULK_SIZE 32
#define NUM_ITERATIONS 1000000

/* Custom data structure to store in mempool */
struct my_obj {
    uint64_t id;
    uint64_t timestamp;
    char data[48];
} __rte_cache_aligned;

/* Constructor callback - called when mempool is created */
static void my_obj_init(struct rte_mempool *mp,
                        __rte_unused void *arg,
                        void *obj,
                        unsigned i)
{
    struct my_obj *my = (struct my_obj *)obj;
    memset(my, 0, sizeof(*my));
    my->id = i;
}

/* Demonstrate basic mempool operations */
static void demonstrate_basic_operations(struct rte_mempool *mp)
{
    struct my_obj *obj = NULL;
    int ret;

    printf("\n=== Basic Mempool Operations ===\n");

    /* Get object from mempool */
    ret = rte_mempool_get(mp, (void **)&obj);
    if (ret < 0) {
        printf("Error: Cannot get object from mempool\n");
        return;
    }

    printf("Allocated object with ID: %lu\n", obj->id);
    obj->timestamp = rte_get_tsc_cycles();
    printf("Set timestamp: %lu\n", obj->timestamp);

    /* Return object to mempool */
    rte_mempool_put(mp, obj);
    printf("Object returned to mempool\n");

    /* Get multiple objects at once (bulk operation) */
    void *obj_table[BULK_SIZE];
    ret = rte_mempool_get_bulk(mp, obj_table, BULK_SIZE);
    if (ret < 0) {
        printf("Error: Cannot get bulk objects from mempool\n");
        return;
    }

    printf("\nAllocated %d objects in bulk:\n", BULK_SIZE);
    for (int i = 0; i < 5; i++) {  /* Print first 5 */
        struct my_obj *o = (struct my_obj *)obj_table[i];
        printf("  Object[%d] ID: %lu\n", i, o->id);
    }

    /* Return multiple objects at once */
    rte_mempool_put_bulk(mp, obj_table, BULK_SIZE);
    printf("Returned %d objects to mempool\n", BULK_SIZE);
}

/* Benchmark single allocation/deallocation */
static void benchmark_single_ops(struct rte_mempool *mp)
{
    struct my_obj *obj;
    uint64_t start, end;
    double cycles_per_op;

    printf("\n=== Single Operation Benchmark ===\n");

    start = rte_get_tsc_cycles();

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        rte_mempool_get(mp, (void **)&obj);
        obj->timestamp = i;
        rte_mempool_put(mp, obj);
    }

    end = rte_get_tsc_cycles();
    cycles_per_op = (double)(end - start) / NUM_ITERATIONS;

    printf("Iterations: %d\n", NUM_ITERATIONS);
    printf("Total cycles: %lu\n", end - start);
    printf("Cycles per allocation+free: %.2f\n", cycles_per_op);
    printf("CPU frequency: ~%.2f GHz (estimated)\n",
           rte_get_tsc_hz() / 1e9);
}

/* Benchmark bulk operations */
static void benchmark_bulk_ops(struct rte_mempool *mp)
{
    void *obj_table[BULK_SIZE];
    uint64_t start, end;
    double cycles_per_op;
    int num_bulk_ops = NUM_ITERATIONS / BULK_SIZE;

    printf("\n=== Bulk Operation Benchmark ===\n");

    start = rte_get_tsc_cycles();

    for (int i = 0; i < num_bulk_ops; i++) {
        rte_mempool_get_bulk(mp, obj_table, BULK_SIZE);

        /* Do some work */
        for (int j = 0; j < BULK_SIZE; j++) {
            struct my_obj *obj = (struct my_obj *)obj_table[j];
            obj->timestamp = i * BULK_SIZE + j;
        }

        rte_mempool_put_bulk(mp, obj_table, BULK_SIZE);
    }

    end = rte_get_tsc_cycles();
    cycles_per_op = (double)(end - start) / (num_bulk_ops * BULK_SIZE);

    printf("Bulk size: %d\n", BULK_SIZE);
    printf("Number of bulk operations: %d\n", num_bulk_ops);
    printf("Total objects processed: %d\n", num_bulk_ops * BULK_SIZE);
    printf("Total cycles: %lu\n", end - start);
    printf("Cycles per allocation+free: %.2f\n", cycles_per_op);
    printf("Speedup vs single: %.2fx\n",
           cycles_per_op > 0 ? 100.0 / cycles_per_op : 0);
}

/* Display mempool statistics */
static void display_mempool_stats(struct rte_mempool *mp)
{
    struct rte_mempool_ops_table *ops_table = rte_mempool_get_ops_table();

    printf("\n=== Mempool Statistics ===\n");
    printf("Name: %s\n", mp->name);
    printf("Total elements: %u\n", mp->size);
    printf("Available elements: %u\n", rte_mempool_avail_count(mp));
    printf("In use elements: %u\n", rte_mempool_in_use_count(mp));
    printf("Element size: %u bytes\n", mp->elt_size);
    printf("Header size: %u bytes\n", mp->header_size);
    printf("Trailer size: %u bytes\n", mp->trailer_size);
    printf("Cache size: %u\n", mp->cache_size);
    printf("Private data size: %u\n", mp->private_data_size);
    printf("Socket ID: %d\n", mp->socket_id);
    printf("Flags: 0x%x\n", mp->flags);
}

/* Main function */
int main(int argc, char *argv[])
{
    struct rte_mempool *mp;
    int ret;
    unsigned socket_id;

    printf("DPDK Mempool Example\n");
    printf("====================\n");

    /* Initialize EAL */
    ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        fprintf(stderr, "Error: Cannot init EAL\n");
        return -1;
    }

    /* Get socket ID of current lcore for NUMA-aware allocation */
    socket_id = rte_socket_id();
    printf("\nRunning on socket: %u\n", socket_id);

    /* Create mempool
     * Arguments:
     * - name: unique name
     * - n: number of elements
     * - elt_size: size of each element
     * - cache_size: per-lcore cache size (0 = no cache)
     * - private_data_size: size of private data
     * - mp_init: mempool constructor (can be NULL)
     * - mp_init_arg: argument to mp_init
     * - obj_init: object constructor
     * - obj_init_arg: argument to obj_init
     * - socket_id: NUMA socket
     * - flags: flags
     */
    printf("\nCreating mempool with %u elements of %lu bytes each\n",
           MEMPOOL_SIZE, sizeof(struct my_obj));

    mp = rte_mempool_create("my_mempool",
                            MEMPOOL_SIZE,
                            sizeof(struct my_obj),
                            MEMPOOL_CACHE_SIZE,
                            0,
                            NULL, NULL,
                            my_obj_init, NULL,
                            socket_id,
                            0);

    if (mp == NULL) {
        fprintf(stderr, "Error: Cannot create mempool: %s\n",
                rte_strerror(rte_errno));
        rte_eal_cleanup();
        return -1;
    }

    printf("Mempool created successfully\n");

    /* Display mempool information */
    display_mempool_stats(mp);

    /* Demonstrate operations */
    demonstrate_basic_operations(mp);

    /* Run benchmarks */
    benchmark_single_ops(mp);
    benchmark_bulk_ops(mp);

    /* Show final stats */
    display_mempool_stats(mp);

    /* Key Takeaways */
    printf("\n=== Key Takeaways ===\n");
    printf("1. Mempools provide fast, lock-free object allocation\n");
    printf("2. Per-core caching reduces contention\n");
    printf("3. Bulk operations are much faster than single ops\n");
    printf("4. NUMA-aware allocation improves performance\n");
    printf("5. Mempools are ideal for packet buffers (mbufs)\n");

    /* Cleanup */
    rte_mempool_free(mp);
    rte_eal_cleanup();

    return 0;
}

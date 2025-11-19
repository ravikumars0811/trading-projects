/*
 * DPDK Ring Buffer Example
 *
 * This example demonstrates:
 * - Creating lock-free ring buffers
 * - Single/Multi producer and consumer modes
 * - Bulk enqueue/dequeue operations
 * - Performance comparison
 * - Inter-core communication pattern
 *
 * Build: gcc -o ring_example ring_example.c $(pkg-config --cflags --libs libdpdk)
 * Run: sudo ./ring_example -l 0-3 -n 4
 */

#include <stdio.h>
#include <stdint.h>
#include <rte_eal.h>
#include <rte_ring.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_malloc.h>

#define RING_SIZE 1024
#define BULK_SIZE 32
#define NUM_ITERATIONS 1000000

/* Message structure for inter-core communication */
struct message {
    uint64_t seq_num;
    uint64_t timestamp;
    uint32_t src_lcore;
    uint32_t dst_lcore;
    char payload[48];
} __rte_cache_aligned;

/* Demonstrate basic ring operations */
static void demonstrate_basic_operations(struct rte_ring *ring)
{
    struct message *msg;
    int ret;

    printf("\n=== Basic Ring Operations ===\n");

    /* Allocate a message */
    msg = rte_zmalloc(NULL, sizeof(struct message), 0);
    if (msg == NULL) {
        printf("Error: Cannot allocate message\n");
        return;
    }

    /* Fill message */
    msg->seq_num = 1;
    msg->timestamp = rte_get_tsc_cycles();
    msg->src_lcore = rte_lcore_id();
    msg->dst_lcore = 0;
    snprintf(msg->payload, sizeof(msg->payload), "Hello from core %u",
             msg->src_lcore);

    /* Enqueue single element */
    ret = rte_ring_enqueue(ring, msg);
    if (ret < 0) {
        printf("Error: Cannot enqueue message\n");
        rte_free(msg);
        return;
    }
    printf("Enqueued message (seq=%lu) to ring\n", msg->seq_num);

    /* Check ring count */
    printf("Ring count: %u / %u\n",
           rte_ring_count(ring), rte_ring_get_capacity(ring));
    printf("Ring free count: %u\n", rte_ring_free_count(ring));

    /* Dequeue single element */
    struct message *recv_msg;
    ret = rte_ring_dequeue(ring, (void **)&recv_msg);
    if (ret < 0) {
        printf("Error: Cannot dequeue message\n");
        return;
    }

    printf("Dequeued message:\n");
    printf("  Seq: %lu\n", recv_msg->seq_num);
    printf("  Timestamp: %lu\n", recv_msg->timestamp);
    printf("  Source: core %u\n", recv_msg->src_lcore);
    printf("  Payload: %s\n", recv_msg->payload);

    rte_free(recv_msg);
}

/* Demonstrate bulk operations */
static void demonstrate_bulk_operations(struct rte_ring *ring)
{
    struct message *msgs[BULK_SIZE];
    struct message *recv_msgs[BULK_SIZE];
    unsigned int ret;

    printf("\n=== Bulk Ring Operations ===\n");

    /* Allocate bulk messages */
    for (int i = 0; i < BULK_SIZE; i++) {
        msgs[i] = rte_zmalloc(NULL, sizeof(struct message), 0);
        if (msgs[i] == NULL) {
            printf("Error: Cannot allocate message %d\n", i);
            /* Free already allocated */
            for (int j = 0; j < i; j++)
                rte_free(msgs[j]);
            return;
        }
        msgs[i]->seq_num = i;
        msgs[i]->timestamp = rte_get_tsc_cycles();
        msgs[i]->src_lcore = rte_lcore_id();
    }

    /* Bulk enqueue */
    ret = rte_ring_enqueue_bulk(ring, (void **)msgs, BULK_SIZE, NULL);
    if (ret == 0) {
        printf("Error: Cannot enqueue bulk messages\n");
        for (int i = 0; i < BULK_SIZE; i++)
            rte_free(msgs[i]);
        return;
    }
    printf("Enqueued %u messages in bulk\n", ret);
    printf("Ring count: %u\n", rte_ring_count(ring));

    /* Bulk dequeue */
    ret = rte_ring_dequeue_bulk(ring, (void **)recv_msgs, BULK_SIZE, NULL);
    if (ret == 0) {
        printf("Error: Cannot dequeue bulk messages\n");
        return;
    }
    printf("Dequeued %u messages in bulk\n", ret);

    printf("First 5 messages:\n");
    for (int i = 0; i < 5; i++) {
        printf("  [%d] seq=%lu, ts=%lu\n",
               i, recv_msgs[i]->seq_num, recv_msgs[i]->timestamp);
    }

    /* Free messages */
    for (int i = 0; i < BULK_SIZE; i++)
        rte_free(recv_msgs[i]);
}

/* Benchmark single operations */
static void benchmark_single_ops(struct rte_ring *ring)
{
    uint64_t start, end;
    void *obj = (void *)0x1234;  /* Dummy pointer */
    double cycles_per_op;

    printf("\n=== Single Operation Benchmark ===\n");

    start = rte_get_tsc_cycles();

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        rte_ring_enqueue(ring, obj);
        rte_ring_dequeue(ring, &obj);
    }

    end = rte_get_tsc_cycles();
    cycles_per_op = (double)(end - start) / NUM_ITERATIONS;

    printf("Iterations: %d\n", NUM_ITERATIONS);
    printf("Total cycles: %lu\n", end - start);
    printf("Cycles per enqueue+dequeue: %.2f\n", cycles_per_op);
}

/* Benchmark bulk operations */
static void benchmark_bulk_ops(struct rte_ring *ring)
{
    void *obj_table[BULK_SIZE];
    uint64_t start, end;
    double cycles_per_op;
    int num_bulk_ops = NUM_ITERATIONS / BULK_SIZE;

    printf("\n=== Bulk Operation Benchmark ===\n");

    /* Initialize dummy pointers */
    for (int i = 0; i < BULK_SIZE; i++)
        obj_table[i] = (void *)(uintptr_t)(0x1000 + i);

    start = rte_get_tsc_cycles();

    for (int i = 0; i < num_bulk_ops; i++) {
        rte_ring_enqueue_bulk(ring, obj_table, BULK_SIZE, NULL);
        rte_ring_dequeue_bulk(ring, obj_table, BULK_SIZE, NULL);
    }

    end = rte_get_tsc_cycles();
    cycles_per_op = (double)(end - start) / (num_bulk_ops * BULK_SIZE);

    printf("Bulk size: %d\n", BULK_SIZE);
    printf("Number of bulk operations: %d\n", num_bulk_ops);
    printf("Total operations: %d\n", num_bulk_ops * BULK_SIZE);
    printf("Total cycles: %lu\n", end - start);
    printf("Cycles per enqueue+dequeue: %.2f\n", cycles_per_op);
}

/* Display ring statistics */
static void display_ring_stats(struct rte_ring *ring)
{
    printf("\n=== Ring Statistics ===\n");
    printf("Name: %s\n", ring->name);
    printf("Capacity: %u\n", rte_ring_get_capacity(ring));
    printf("Used count: %u\n", rte_ring_count(ring));
    printf("Free count: %u\n", rte_ring_free_count(ring));
    printf("Size: %u bytes\n", rte_ring_get_size(ring));
    printf("Flags: 0x%x\n", ring->flags);

    /* Check ring type */
    if (ring->flags & RTE_RING_F_SP_ENQ)
        printf("Type: Single Producer\n");
    else
        printf("Type: Multi Producer\n");

    if (ring->flags & RTE_RING_F_SC_DEQ)
        printf("Consumer: Single Consumer\n");
    else
        printf("Consumer: Multi Consumer\n");
}

/* Main function */
int main(int argc, char *argv[])
{
    struct rte_ring *sp_sc_ring;
    struct rte_ring *mp_mc_ring;
    int ret;

    printf("DPDK Ring Buffer Example\n");
    printf("========================\n");

    /* Initialize EAL */
    ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        fprintf(stderr, "Error: Cannot init EAL\n");
        return -1;
    }

    printf("\nRunning on lcore: %u (socket %u)\n",
           rte_lcore_id(), rte_socket_id());

    /* Create Single Producer Single Consumer ring */
    printf("\nCreating SP/SC ring with %u elements\n", RING_SIZE);
    sp_sc_ring = rte_ring_create("sp_sc_ring",
                                 RING_SIZE,
                                 rte_socket_id(),
                                 RTE_RING_F_SP_ENQ | RTE_RING_F_SC_DEQ);
    if (sp_sc_ring == NULL) {
        fprintf(stderr, "Error: Cannot create SP/SC ring\n");
        rte_eal_cleanup();
        return -1;
    }

    /* Create Multi Producer Multi Consumer ring */
    printf("Creating MP/MC ring with %u elements\n", RING_SIZE);
    mp_mc_ring = rte_ring_create("mp_mc_ring",
                                 RING_SIZE,
                                 rte_socket_id(),
                                 0);  /* Default is MP/MC */
    if (mp_mc_ring == NULL) {
        fprintf(stderr, "Error: Cannot create MP/MC ring\n");
        rte_ring_free(sp_sc_ring);
        rte_eal_cleanup();
        return -1;
    }

    /* Display ring information */
    display_ring_stats(sp_sc_ring);
    display_ring_stats(mp_mc_ring);

    /* Demonstrate operations on SP/SC ring */
    printf("\n>>> Testing SP/SC Ring <<<\n");
    demonstrate_basic_operations(sp_sc_ring);
    demonstrate_bulk_operations(sp_sc_ring);

    /* Benchmark SP/SC ring */
    printf("\n>>> Benchmarking SP/SC Ring <<<\n");
    benchmark_single_ops(sp_sc_ring);
    benchmark_bulk_ops(sp_sc_ring);

    /* Benchmark MP/MC ring */
    printf("\n>>> Benchmarking MP/MC Ring <<<\n");
    printf("Note: MP/MC has higher overhead due to atomic operations\n");
    benchmark_single_ops(mp_mc_ring);
    benchmark_bulk_ops(mp_mc_ring);

    /* Key Takeaways */
    printf("\n=== Key Takeaways ===\n");
    printf("1. Rings are lock-free, bounded FIFO queues\n");
    printf("2. SP/SC rings are faster than MP/MC (no atomics)\n");
    printf("3. Bulk operations are much more efficient\n");
    printf("4. Rings are power-of-2 sized for fast modulo\n");
    printf("5. Perfect for inter-core communication in DPDK\n");
    printf("6. Used internally by DPDK for packet queuing\n");

    /* Cleanup */
    rte_ring_free(sp_sc_ring);
    rte_ring_free(mp_mc_ring);
    rte_eal_cleanup();

    return 0;
}

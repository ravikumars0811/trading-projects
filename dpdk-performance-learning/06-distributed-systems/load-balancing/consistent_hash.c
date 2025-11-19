/*
 * Consistent Hashing Implementation
 *
 * Used for:
 * - Load balancing
 * - Distributed caching
 * - Data partitioning
 * - Minimal key remapping on node changes
 *
 * Build: gcc -O2 -o consistent_hash consistent_hash.c
 * Run: ./consistent_hash
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_SERVERS 100
#define VIRTUAL_NODES 150  /* Virtual nodes per server */
#define HASH_RING_SIZE (MAX_SERVERS * VIRTUAL_NODES)

/* MurmurHash3 (32-bit) - Fast non-cryptographic hash */
static uint32_t murmur3_32(const void *key, size_t len, uint32_t seed)
{
    const uint8_t *data = (const uint8_t *)key;
    const int nblocks = len / 4;
    uint32_t h1 = seed;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    /* Body */
    const uint32_t *blocks = (const uint32_t *)(data + nblocks * 4);
    for (int i = -nblocks; i; i++) {
        uint32_t k1 = blocks[i];
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> (32 - 15));
        k1 *= c2;
        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> (32 - 13));
        h1 = h1 * 5 + 0xe6546b64;
    }

    /* Tail */
    const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);
    uint32_t k1 = 0;
    switch (len & 3) {
        case 3: k1 ^= tail[2] << 16; /* fallthrough */
        case 2: k1 ^= tail[1] << 8;  /* fallthrough */
        case 1: k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << 15) | (k1 >> (32 - 15));
                k1 *= c2;
                h1 ^= k1;
    }

    /* Finalization */
    h1 ^= len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    return h1;
}

/* Hash ring node */
struct ring_node {
    uint32_t hash;
    int server_id;
};

/* Consistent hash ring */
struct hash_ring {
    struct ring_node nodes[HASH_RING_SIZE];
    int num_nodes;
    int num_servers;
};

/* Compare function for qsort */
static int compare_nodes(const void *a, const void *b)
{
    const struct ring_node *na = (const struct ring_node *)a;
    const struct ring_node *nb = (const struct ring_node *)b;
    if (na->hash < nb->hash) return -1;
    if (na->hash > nb->hash) return 1;
    return 0;
}

/* Initialize hash ring */
static void hash_ring_init(struct hash_ring *ring)
{
    memset(ring, 0, sizeof(*ring));
}

/* Add server to hash ring */
static void hash_ring_add_server(struct hash_ring *ring, int server_id, const char *server_name)
{
    char vnode_key[256];

    /* Add virtual nodes for this server */
    for (int i = 0; i < VIRTUAL_NODES; i++) {
        /* Create virtual node key */
        snprintf(vnode_key, sizeof(vnode_key), "%s:%d", server_name, i);

        /* Hash the key */
        uint32_t hash = murmur3_32(vnode_key, strlen(vnode_key), 0);

        /* Add to ring */
        ring->nodes[ring->num_nodes].hash = hash;
        ring->nodes[ring->num_nodes].server_id = server_id;
        ring->num_nodes++;
    }

    ring->num_servers++;

    /* Sort ring by hash value */
    qsort(ring->nodes, ring->num_nodes, sizeof(struct ring_node), compare_nodes);

    printf("Added server %d (%s) with %d virtual nodes\n",
           server_id, server_name, VIRTUAL_NODES);
}

/* Remove server from hash ring */
static void hash_ring_remove_server(struct hash_ring *ring, int server_id)
{
    int new_count = 0;

    /* Remove all nodes belonging to this server */
    for (int i = 0; i < ring->num_nodes; i++) {
        if (ring->nodes[i].server_id != server_id) {
            ring->nodes[new_count++] = ring->nodes[i];
        }
    }

    int removed = ring->num_nodes - new_count;
    ring->num_nodes = new_count;
    ring->num_servers--;

    printf("Removed server %d (%d virtual nodes removed)\n", server_id, removed);
}

/* Find server for key using binary search */
static int hash_ring_get_server(struct hash_ring *ring, const char *key)
{
    if (ring->num_nodes == 0)
        return -1;

    /* Hash the key */
    uint32_t hash = murmur3_32(key, strlen(key), 0);

    /* Binary search for first node >= hash */
    int left = 0;
    int right = ring->num_nodes - 1;

    while (left < right) {
        int mid = (left + right) / 2;
        if (ring->nodes[mid].hash < hash) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }

    /* Wrap around to first node if needed */
    if (left >= ring->num_nodes || ring->nodes[left].hash < hash) {
        left = 0;
    }

    return ring->nodes[left].server_id;
}

/* Test distribution */
static void test_distribution(struct hash_ring *ring, int num_keys)
{
    int *counts = calloc(ring->num_servers, sizeof(int));
    char key[64];

    printf("\n=== Distribution Test ===\n");
    printf("Testing with %d keys\n", num_keys);

    /* Generate keys and count distribution */
    for (int i = 0; i < num_keys; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        int server = hash_ring_get_server(ring, key);
        if (server >= 0) {
            counts[server]++;
        }
    }

    /* Print distribution */
    printf("\nKey distribution:\n");
    for (int i = 0; i < ring->num_servers; i++) {
        float percentage = (counts[i] * 100.0) / num_keys;
        printf("Server %d: %d keys (%.2f%%)\n", i, counts[i], percentage);
    }

    /* Calculate standard deviation */
    float mean = (float)num_keys / ring->num_servers;
    float variance = 0;
    for (int i = 0; i < ring->num_servers; i++) {
        float diff = counts[i] - mean;
        variance += diff * diff;
    }
    variance /= ring->num_servers;
    float stddev = variance > 0 ? sqrtf(variance) : 0;

    printf("\nStatistics:\n");
    printf("  Mean: %.2f keys per server\n", mean);
    printf("  Std deviation: %.2f\n", stddev);
    printf("  Coefficient of variation: %.2f%%\n", (stddev / mean) * 100);

    free(counts);
}

/* Test key remapping when server is removed */
static void test_key_remapping(struct hash_ring *ring, int num_keys)
{
    char key[64];
    int *before = malloc(num_keys * sizeof(int));
    int remapped = 0;

    printf("\n=== Key Remapping Test ===\n");

    /* Record initial mapping */
    for (int i = 0; i < num_keys; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        before[i] = hash_ring_get_server(ring, key);
    }

    /* Remove server 1 */
    int original_servers = ring->num_servers;
    hash_ring_remove_server(ring, 1);

    /* Check remapping */
    for (int i = 0; i < num_keys; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        int after = hash_ring_get_server(ring, key);

        if (before[i] != after) {
            remapped++;
        }
    }

    float remapped_pct = (remapped * 100.0) / num_keys;
    float expected_pct = 100.0 / original_servers;

    printf("\nKeys remapped: %d / %d (%.2f%%)\n", remapped, num_keys, remapped_pct);
    printf("Expected (1/%d servers): %.2f%%\n", original_servers, expected_pct);
    printf("Overhead: %.2f%%\n", remapped_pct - expected_pct);

    free(before);
}

int main(void)
{
    struct hash_ring ring;
    const int NUM_SERVERS = 5;
    const int NUM_KEYS = 10000;

    printf("Consistent Hashing Implementation\n");
    printf("==================================\n\n");

    /* Initialize hash ring */
    hash_ring_init(&ring);

    /* Add servers */
    printf("=== Adding Servers ===\n");
    for (int i = 0; i < NUM_SERVERS; i++) {
        char server_name[64];
        snprintf(server_name, sizeof(server_name), "server_%d", i);
        hash_ring_add_server(&ring, i, server_name);
    }

    /* Test distribution */
    test_distribution(&ring, NUM_KEYS);

    /* Test key remapping */
    test_key_remapping(&ring, NUM_KEYS);

    printf("\n=== Key Advantages ===\n");
    printf("1. Minimal key remapping when nodes change (~1/N)\n");
    printf("2. Even load distribution with virtual nodes\n");
    printf("3. O(log N) lookup time with binary search\n");
    printf("4. Scales well to many nodes\n");
    printf("5. No need for global coordination\n");

    printf("\n=== Use Cases ===\n");
    printf("- Distributed caching (Memcached, Redis)\n");
    printf("- Load balancing\n");
    printf("- Data partitioning in distributed databases\n");
    printf("- CDN request routing\n");
    printf("- Distributed hash tables (DHT)\n");

    return 0;
}

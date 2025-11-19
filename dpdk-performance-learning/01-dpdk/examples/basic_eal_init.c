/*
 * Basic DPDK EAL Initialization Example
 *
 * This example demonstrates:
 * - EAL initialization
 * - CPU core detection
 * - Memory information
 * - Device enumeration
 *
 * Build: gcc -o basic_eal_init basic_eal_init.c $(pkg-config --cflags --libs libdpdk)
 * Run: sudo ./basic_eal_init -l 0-3 -n 4
 */

#include <stdio.h>
#include <stdint.h>
#include <rte_eal.h>
#include <rte_lcore.h>
#include <rte_mempool.h>
#include <rte_ethdev.h>
#include <rte_version.h>

#define APP_NAME "BasicEAL"

/* Display system information */
static void display_system_info(void)
{
    unsigned lcore_id;
    unsigned socket_id;

    printf("\n=== System Information ===\n");
    printf("DPDK Version: %s\n", rte_version());
    printf("Number of logical cores: %u\n", rte_lcore_count());
    printf("Number of sockets: %u\n", rte_socket_count());

    printf("\n=== CPU Core Information ===\n");
    printf("Main lcore ID: %u\n", rte_get_main_lcore());

    printf("\nLogical cores enabled:\n");
    RTE_LCORE_FOREACH(lcore_id) {
        socket_id = rte_lcore_to_socket_id(lcore_id);
        printf("  Core %u (Socket %u)\n", lcore_id, socket_id);
    }
}

/* Display memory information */
static void display_memory_info(void)
{
    printf("\n=== Memory Information ===\n");

    const struct rte_memzone *mz;
    const struct rte_memseg_list *msl;

    printf("Huge page size: %lu KB\n",
           rte_mem_page_size() / 1024);

    /* Display memory segments */
    msl = rte_mem_virt2memseg_list(NULL);
    if (msl != NULL) {
        printf("Memory segment page size: %lu KB\n",
               msl->page_sz / 1024);
    }
}

/* Display network device information */
static void display_device_info(void)
{
    uint16_t port_id;
    uint16_t nb_ports;
    struct rte_eth_dev_info dev_info;
    char name[RTE_ETH_NAME_MAX_LEN];

    nb_ports = rte_eth_dev_count_avail();

    printf("\n=== Network Device Information ===\n");
    printf("Number of available ports: %u\n", nb_ports);

    if (nb_ports == 0) {
        printf("No network ports available.\n");
        printf("Make sure to bind devices with dpdk-devbind.py\n");
        return;
    }

    RTE_ETH_FOREACH_DEV(port_id) {
        if (rte_eth_dev_get_name_by_port(port_id, name) != 0) {
            snprintf(name, sizeof(name), "Unknown");
        }

        if (rte_eth_dev_info_get(port_id, &dev_info) != 0) {
            printf("\nPort %u (%s): Error getting device info\n",
                   port_id, name);
            continue;
        }

        printf("\nPort %u (%s):\n", port_id, name);
        printf("  Driver: %s\n", dev_info.driver_name);
        printf("  Socket: %d\n", dev_info.device->numa_node);
        printf("  Max RX queues: %u\n", dev_info.max_rx_queues);
        printf("  Max TX queues: %u\n", dev_info.max_tx_queues);
        printf("  Max MAC addresses: %u\n", dev_info.max_mac_addrs);
        printf("  Max RX packet length: %u\n", dev_info.max_rx_pktlen);
        printf("  Min RX buffer size: %u\n", dev_info.min_rx_bufsize);
    }
}

/* Main function */
int main(int argc, char *argv[])
{
    int ret;

    printf("%s: DPDK EAL Initialization Example\n", APP_NAME);
    printf("==================================\n");

    /* Initialize EAL */
    printf("\nInitializing DPDK EAL...\n");
    ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        fprintf(stderr, "Error: Cannot init EAL: %d\n", ret);
        return -1;
    }

    /* Adjust argc/argv to skip EAL parameters */
    argc -= ret;
    argv += ret;

    printf("EAL initialization successful!\n");

    /* Display various system information */
    display_system_info();
    display_memory_info();
    display_device_info();

    printf("\n=== EAL Parameters ===\n");
    printf("Common EAL parameters:\n");
    printf("  -l CORELIST  : List of cores to run on\n");
    printf("  -n CHANNELS  : Number of memory channels\n");
    printf("  --main-lcore : Designate main lcore\n");
    printf("  --socket-mem : Memory to allocate on each socket\n");
    printf("  --proc-type  : primary|secondary|auto\n");
    printf("  --log-level  : Set log level (0-8)\n");
    printf("  -m MB        : Memory to allocate in MB\n");

    printf("\nExample usage:\n");
    printf("  sudo ./basic_eal_init -l 0-3 -n 4\n");
    printf("  sudo ./basic_eal_init -l 0,2,4,6 -n 4 --main-lcore 0\n");
    printf("  sudo ./basic_eal_init -l 0-7 --socket-mem=1024,1024\n");

    /* Cleanup */
    printf("\nCleaning up and exiting...\n");
    rte_eal_cleanup();

    return 0;
}

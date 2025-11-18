# Main Makefile for DPDK Performance Learning Project

.PHONY: all clean help performance-opt system-prog memory-mgmt cpu-opt distributed

# Compiler settings
CC := gcc
CFLAGS := -O2 -Wall -Wextra -std=c11
LDFLAGS := -lm -lpthread

# Directories
PERF_OPT_DIR := 02-performance-optimization
SYS_PROG_DIR := 03-system-programming
MEM_MGMT_DIR := 04-memory-management
CPU_OPT_DIR := 05-cpu-optimization
DIST_SYS_DIR := 06-distributed-systems

all: performance-opt system-prog memory-mgmt cpu-opt distributed
	@echo "==================================="
	@echo "Build complete!"
	@echo "==================================="
	@echo ""
	@echo "Run examples from their respective directories:"
	@echo "  cd 02-performance-optimization/cache && ./cache_friendly"
	@echo "  cd 03-system-programming/threads && ./thread_affinity"
	@echo "  cd 04-memory-management/allocators && ./arena_allocator"
	@echo "  cd 05-cpu-optimization/simd && ./simd_basics"
	@echo "  cd 06-distributed-systems/networking && ./tcp_server_epoll 8080"
	@echo ""

help:
	@echo "DPDK Performance Learning Project - Build System"
	@echo "================================================"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build all modules"
	@echo "  performance-opt  - Build performance optimization examples"
	@echo "  system-prog      - Build system programming examples"
	@echo "  memory-mgmt      - Build memory management examples"
	@echo "  cpu-opt          - Build CPU optimization examples"
	@echo "  distributed      - Build distributed systems examples"
	@echo "  clean            - Clean all build artifacts"
	@echo ""
	@echo "Individual module builds:"
	@echo "  make -C 02-performance-optimization"
	@echo "  make -C 03-system-programming"
	@echo "  etc."
	@echo ""

# Performance Optimization
performance-opt:
	@echo "Building Performance Optimization examples..."
	$(CC) $(CFLAGS) $(PERF_OPT_DIR)/cache/cache_friendly.c -o $(PERF_OPT_DIR)/cache/cache_friendly $(LDFLAGS)

# System Programming
system-prog:
	@echo "Building System Programming examples..."
	$(CC) $(CFLAGS) $(SYS_PROG_DIR)/threads/thread_affinity.c -o $(SYS_PROG_DIR)/threads/thread_affinity -lpthread -lnuma

# Memory Management
memory-mgmt:
	@echo "Building Memory Management examples..."
	$(CC) $(CFLAGS) $(MEM_MGMT_DIR)/allocators/arena_allocator.c -o $(MEM_MGMT_DIR)/allocators/arena_allocator $(LDFLAGS)

# CPU Optimization
cpu-opt:
	@echo "Building CPU Optimization examples..."
	$(CC) -O3 -march=native -mavx2 $(CPU_OPT_DIR)/simd/simd_basics.c -o $(CPU_OPT_DIR)/simd/simd_basics $(LDFLAGS)

# Distributed Systems
distributed:
	@echo "Building Distributed Systems examples..."
	$(CC) $(CFLAGS) $(DIST_SYS_DIR)/networking/tcp_server_epoll.c -o $(DIST_SYS_DIR)/networking/tcp_server_epoll
	$(CC) $(CFLAGS) $(DIST_SYS_DIR)/load-balancing/consistent_hash.c -o $(DIST_SYS_DIR)/load-balancing/consistent_hash $(LDFLAGS)

clean:
	@echo "Cleaning build artifacts..."
	find . -type f -executable -not -path "./.git/*" -delete
	find . -name "*.o" -delete
	find . -name "*.so" -delete
	find . -name "*.a" -delete
	@echo "Clean complete!"

# DPDK examples require special handling
# Note: DPDK examples need DPDK installed and configured
# See 01-dpdk/docs/setup.md for instructions
dpdk:
	@echo "Building DPDK examples requires DPDK installation"
	@echo "See 01-dpdk/docs/setup.md for setup instructions"
	@echo ""
	@if [ -n "$$RTE_SDK" ]; then \
		echo "RTE_SDK is set to: $$RTE_SDK"; \
		echo "Building DPDK examples..."; \
		cd 01-dpdk/examples && \
		gcc -O2 basic_eal_init.c -o basic_eal_init $$(pkg-config --cflags --libs libdpdk) && \
		gcc -O2 mempool_example.c -o mempool_example $$(pkg-config --cflags --libs libdpdk) && \
		gcc -O2 ring_example.c -o ring_example $$(pkg-config --cflags --libs libdpdk) && \
		echo "DPDK examples built successfully!"; \
	else \
		echo "RTE_SDK not set. Please install and configure DPDK first."; \
		exit 1; \
	fi

#!/bin/bash
################################################################################
# Memory and Hugepages Optimization for Trading Systems
#
# This script configures hugepages and memory optimizations
# Run with: sudo ./03-memory-optimization.sh
################################################################################

set -e

echo "=========================================="
echo "Memory & Hugepages Optimization"
echo "=========================================="

# Get total memory in GB
TOTAL_MEM_KB=$(grep MemTotal /proc/meminfo | awk '{print $2}')
TOTAL_MEM_GB=$((TOTAL_MEM_KB / 1024 / 1024))

echo "Total System Memory: ${TOTAL_MEM_GB}GB"

################################################################################
# HUGEPAGES CONFIGURATION
################################################################################

echo ""
echo "Configuring Hugepages..."
echo ""

# Calculate hugepage allocation
# Hugepage size is typically 2MB on x86_64
HUGEPAGE_SIZE_KB=$(grep Hugepagesize /proc/meminfo | awk '{print $2}')
HUGEPAGE_SIZE_MB=$((HUGEPAGE_SIZE_KB / 1024))

echo "Hugepage size: ${HUGEPAGE_SIZE_MB}MB"

# Allocate hugepages (adjust based on your application needs)
# For trading applications, allocate ~20-30% of total memory to hugepages
HUGEPAGES_PERCENT=25
HUGEPAGES_TO_ALLOCATE=$(( (TOTAL_MEM_KB * HUGEPAGES_PERCENT / 100) / HUGEPAGE_SIZE_KB ))

echo "Allocating $HUGEPAGES_TO_ALLOCATE hugepages (${HUGEPAGES_PERCENT}% of total memory)"

# Set hugepages
echo $HUGEPAGES_TO_ALLOCATE > /proc/sys/vm/nr_hugepages

# Verify allocation
ALLOCATED=$(cat /proc/sys/vm/nr_hugepages)
echo "Hugepages allocated: $ALLOCATED"

# Make hugepages persistent
cat >> /etc/sysctl.d/99-trading-optimization.conf << EOF

################################################################################
# HUGEPAGES
################################################################################
vm.nr_hugepages = $HUGEPAGES_TO_ALLOCATE
EOF

################################################################################
# CONFIGURE HUGETLBFS
################################################################################

echo ""
echo "Configuring hugetlbfs mount..."

# Create mount point
mkdir -p /mnt/huge

# Check if already mounted
if ! grep -q "hugetlbfs" /proc/mounts; then
    mount -t hugetlbfs nodev /mnt/huge
    echo "Mounted hugetlbfs at /mnt/huge"

    # Make persistent in /etc/fstab
    if ! grep -q "hugetlbfs" /etc/fstab; then
        echo "nodev /mnt/huge hugetlbfs defaults 0 0" >> /etc/fstab
        echo "Added hugetlbfs to /etc/fstab"
    fi
else
    echo "hugetlbfs already mounted"
fi

################################################################################
# MEMORY LOCKING
################################################################################

echo ""
echo "Configuring memory locking..."

# Allow users to lock memory (already done in 01-kernel-optimization.sh)
# This prevents swapping of critical memory pages

cat >> /etc/security/limits.d/99-trading.conf << 'EOF'

# Memory locking for hugepages
*    soft    memlock    unlimited
*    hard    memlock    unlimited
EOF

################################################################################
# NUMA MEMORY POLICY
################################################################################

echo ""
echo "NUMA Memory Configuration:"
echo ""

if command -v numactl &> /dev/null; then
    numactl --hardware

    echo ""
    echo "NUMA policy recommendations:"
    echo "  - Bind application to NUMA node 0 for best latency"
    echo "  - Example: numactl --membind=0 --cpunodebind=0 ./hft_system"
    echo ""

    # Create script to check NUMA memory allocation
    cat > /home/user/trading-projects/check-numa-allocation.sh << 'EOFNUMA'
#!/bin/bash
################################################################################
# Check NUMA Memory Allocation
################################################################################

echo "NUMA Memory Allocation:"
echo "======================="
echo ""

for pid in $(pgrep -f "hft_system"); do
    echo "Process: $pid ($(ps -p $pid -o comm=))"
    echo "Memory distribution:"
    numastat -p $pid
    echo ""
done

echo "System-wide NUMA stats:"
numastat
EOFNUMA

    chmod +x /home/user/trading-projects/check-numa-allocation.sh
    echo "Created NUMA monitoring script: /home/user/trading-projects/check-numa-allocation.sh"
else
    echo "numactl not installed. Install with: apt-get install numactl"
fi

################################################################################
# MEMORY POOL CONFIGURATION
################################################################################

echo ""
echo "Creating memory pool configuration..."

cat > /home/user/trading-projects/HFT-System-CPP/config/memory_config.txt << 'EOF'
# Memory Pool Configuration for HFT System
#
# Pre-allocate memory pools to eliminate runtime allocation overhead

[memory_pools]
# Order pool: Number of pre-allocated order objects
# Each order ~128 bytes, 100K orders = ~12MB
order_pool_size = 100000

# Market data message pool
# Each message ~256 bytes, 1M messages = ~256MB
market_data_pool_size = 1000000

# Trade pool
trade_pool_size = 50000

# Use hugepages for memory pools (0=no, 1=yes)
use_hugepages = 1

# Hugepage path (mounted hugetlbfs)
hugepage_path = /mnt/huge

[memory_allocation]
# Pre-fault all memory at startup (touch all pages)
prefault_memory = 1

# Lock memory to prevent swapping (mlockall)
lock_memory = 1

# NUMA node to allocate memory from (-1 = default)
numa_node = 0

[cache_optimization]
# Align data structures to cache line size (64 bytes)
cache_line_size = 64

# Prefetch distance for critical paths (cache lines)
prefetch_distance = 8
EOF

echo "Created: /home/user/trading-projects/HFT-System-CPP/config/memory_config.txt"

################################################################################
# CREATE MEMORY MONITORING SCRIPT
################################################################################

cat > /home/user/trading-projects/monitor-memory.sh << 'EOFMEM'
#!/bin/bash
################################################################################
# Memory Monitoring for Trading Applications
################################################################################

echo "Memory Performance Monitor"
echo "==========================="
echo ""

# Hugepages status
echo "=== Hugepages Status ==="
grep -E "HugePages|Hugepagesize" /proc/meminfo
echo ""

# Memory usage
echo "=== Memory Usage ==="
free -h
echo ""

# Swap usage (should be near zero)
echo "=== Swap Usage (should be minimal) ==="
swapon --show
echo ""

# Per-process memory for HFT
echo "=== HFT Process Memory ==="
for pid in $(pgrep -f "hft_system"); do
    echo "PID $pid:"
    pmap -x $pid | tail -1
    echo "  RSS: $(ps -p $pid -o rss= | awk '{print $1/1024 "MB"}')"
    echo "  Virtual: $(ps -p $pid -o vsz= | awk '{print $1/1024 "MB"}')"
    echo ""
done

# Page faults (should be zero during steady state)
echo "=== Page Faults ==="
for pid in $(pgrep -f "hft_system"); do
    echo "PID $pid:"
    ps -p $pid -o min_flt,maj_flt,cmd
done
echo ""

# NUMA memory distribution
if command -v numastat &> /dev/null; then
    echo "=== NUMA Distribution ==="
    numastat -c hft_system 2>/dev/null || echo "No HFT process running"
fi
EOFMEM

chmod +x /home/user/trading-projects/monitor-memory.sh

echo ""
echo "Created monitoring script: /home/user/trading-projects/monitor-memory.sh"

################################################################################
# CREATE C++ HUGEPAGE ALLOCATOR HEADER
################################################################################

echo ""
echo "Creating C++ hugepage allocator..."

mkdir -p /home/user/trading-projects/HFT-System-CPP/include/core

cat > /home/user/trading-projects/HFT-System-CPP/include/core/hugepage_allocator.hpp << 'EOFHPP'
#pragma once

#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

namespace hft {
namespace core {

/**
 * Allocator that uses hugepages for improved performance
 * Reduces TLB misses and improves cache efficiency
 */
template<typename T>
class HugepageAllocator {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    HugepageAllocator() noexcept = default;

    template<typename U>
    HugepageAllocator(const HugepageAllocator<U>&) noexcept {}

    T* allocate(size_type n) {
        if (n == 0) return nullptr;

        size_type bytes = n * sizeof(T);

        // Align to hugepage size (2MB)
        constexpr size_type HUGEPAGE_SIZE = 2 * 1024 * 1024;
        size_type aligned_bytes = ((bytes + HUGEPAGE_SIZE - 1) / HUGEPAGE_SIZE) * HUGEPAGE_SIZE;

        // Allocate using mmap with MAP_HUGETLB
        void* ptr = mmap(nullptr, aligned_bytes,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                        -1, 0);

        if (ptr == MAP_FAILED) {
            // Fallback to regular allocation
            ptr = mmap(nullptr, aligned_bytes,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1, 0);

            if (ptr == MAP_FAILED) {
                throw std::bad_alloc();
            }
        }

        // Lock memory to prevent swapping
        mlock(ptr, aligned_bytes);

        return static_cast<T*>(ptr);
    }

    void deallocate(T* ptr, size_type n) noexcept {
        if (ptr == nullptr || n == 0) return;

        size_type bytes = n * sizeof(T);
        constexpr size_type HUGEPAGE_SIZE = 2 * 1024 * 1024;
        size_type aligned_bytes = ((bytes + HUGEPAGE_SIZE - 1) / HUGEPAGE_SIZE) * HUGEPAGE_SIZE;

        munlock(ptr, aligned_bytes);
        munmap(ptr, aligned_bytes);
    }
};

template<typename T, typename U>
bool operator==(const HugepageAllocator<T>&, const HugepageAllocator<U>&) {
    return true;
}

template<typename T, typename U>
bool operator!=(const HugepageAllocator<T>&, const HugepageAllocator<U>&) {
    return false;
}

} // namespace core
} // namespace hft
EOFHPP

echo "Created: /home/user/trading-projects/HFT-System-CPP/include/core/hugepage_allocator.hpp"

################################################################################
# VERIFICATION
################################################################################

echo ""
echo "=========================================="
echo "Memory Optimization Complete!"
echo "=========================================="
echo ""
echo "Current Configuration:"
echo "  Hugepages allocated: $(cat /proc/sys/vm/nr_hugepages)"
echo "  Hugepage size: ${HUGEPAGE_SIZE_MB}MB"
echo "  Total hugepage memory: $(($(cat /proc/sys/vm/nr_hugepages) * HUGEPAGE_SIZE_MB))MB"
echo ""
echo "Hugepage usage:"
grep -E "HugePages" /proc/meminfo
echo ""
echo "Next Steps:"
echo "1. Monitor memory with: /home/user/trading-projects/monitor-memory.sh"
echo "2. Check NUMA with: /home/user/trading-projects/check-numa-allocation.sh"
echo "3. Update HFT code to use HugepageAllocator for critical data structures"
echo ""
echo "Example usage in C++:"
echo "  std::vector<Order, HugepageAllocator<Order>> orders;"
echo ""

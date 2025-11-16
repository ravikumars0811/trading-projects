#!/bin/bash

###############################################################################
# Linux HFT Server Optimization Script
###############################################################################
#
# This script configures a Linux server for optimal HFT performance:
# - CPU isolation and frequency scaling
# - Network stack optimization
# - Memory management (huge pages, swap)
# - IRQ affinity
# - Filesystem and I/O optimization
#
# Usage: sudo ./optimize_linux_hft.sh
#
# IMPORTANT: Review and customize before running in production!
#
###############################################################################

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}ERROR: Please run as root (sudo)${NC}"
    exit 1
fi

echo "======================================================================"
echo "HFT Linux Server Optimization Script"
echo "======================================================================"
echo ""

# Configuration variables
ISOLATED_CPUS="2-7"  # CPUs to isolate for HFT (adjust based on your system)
HOUSEKEEPING_CPUS="0,1"  # CPUs for OS tasks
HUGEPAGE_2MB_COUNT=1024  # Number of 2MB huge pages (2GB total)
HUGEPAGE_1GB_COUNT=4     # Number of 1GB huge pages (4GB total)

###############################################################################
# 1. CPU OPTIMIZATION
###############################################################################

echo -e "${GREEN}[1] CPU Optimization${NC}"
echo "----------------------------------------------------------------------"

# Set CPU governor to performance mode
echo "Setting CPU governor to 'performance' mode..."
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    echo "performance" > $cpu 2>/dev/null || true
done

# Verify
governor=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo "N/A")
echo "  CPU Governor: $governor"

# Disable CPU turbo boost (reduces jitter)
if [ -f /sys/devices/system/cpu/intel_pstate/no_turbo ]; then
    echo "Disabling Intel Turbo Boost..."
    echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo
    echo "  Intel Turbo Boost: Disabled"
elif [ -f /sys/devices/system/cpu/cpufreq/boost ]; then
    echo "Disabling CPU Boost..."
    echo 0 > /sys/devices/system/cpu/cpufreq/boost
    echo "  CPU Boost: Disabled"
fi

# Disable C-states (prevents CPU from sleeping)
echo "Disabling C-states..."
for state in /sys/devices/system/cpu/cpu*/cpuidle/state*/disable; do
    echo 1 > $state 2>/dev/null || true
done

echo -e "${YELLOW}Note: For CPU isolation (isolcpus, nohz_full), edit /etc/default/grub:${NC}"
echo "  GRUB_CMDLINE_LINUX=\"isolcpus=$ISOLATED_CPUS nohz_full=$ISOLATED_CPUS rcu_nocbs=$ISOLATED_CPUS\""
echo "  Then run: sudo update-grub && sudo reboot"
echo ""

###############################################################################
# 2. IRQ AFFINITY
###############################################################################

echo -e "${GREEN}[2] IRQ Affinity Configuration${NC}"
echo "----------------------------------------------------------------------"

# Move all IRQs to housekeeping CPUs
echo "Moving IRQs to housekeeping CPUs ($HOUSEKEEPING_CPUS)..."

# Convert CPU list to hex mask (0,1 -> 0x03)
if [ "$HOUSEKEEPING_CPUS" == "0,1" ]; then
    IRQ_MASK="03"  # Binary: 0000 0011
elif [ "$HOUSEKEEPING_CPUS" == "0" ]; then
    IRQ_MASK="01"
else
    IRQ_MASK="03"  # Default
fi

irq_count=0
for irq in /proc/irq/*/smp_affinity; do
    echo "$IRQ_MASK" > $irq 2>/dev/null || true
    ((irq_count++))
done

echo "  Configured $irq_count IRQs to CPUs $HOUSEKEEPING_CPUS"
echo ""

###############################################################################
# 3. MEMORY OPTIMIZATION
###############################################################################

echo -e "${GREEN}[3] Memory Optimization${NC}"
echo "----------------------------------------------------------------------"

# Configure 2MB huge pages
echo "Configuring 2MB huge pages ($HUGEPAGE_2MB_COUNT pages = $((HUGEPAGE_2MB_COUNT * 2))MB)..."
echo $HUGEPAGE_2MB_COUNT > /proc/sys/vm/nr_hugepages

actual_2mb=$(cat /proc/sys/vm/nr_hugepages)
echo "  2MB Huge Pages: $actual_2mb allocated"

# Configure 1GB huge pages (if supported)
if [ -d /sys/kernel/mm/hugepages/hugepages-1048576kB ]; then
    echo "Configuring 1GB huge pages ($HUGEPAGE_1GB_COUNT pages = ${HUGEPAGE_1GB_COUNT}GB)..."
    echo $HUGEPAGE_1GB_COUNT > /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages

    actual_1gb=$(cat /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages)
    echo "  1GB Huge Pages: $actual_1gb allocated"
fi

# Enable transparent huge pages
echo "Enabling transparent huge pages..."
echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo always > /sys/kernel/mm/transparent_hugepage/defrag

thp_status=$(cat /sys/kernel/mm/transparent_hugepage/enabled)
echo "  Transparent Huge Pages: $thp_status"

# Disable swap (critical for low latency)
echo "Disabling swap..."
swapoff -a
echo "  Swap: Disabled"

# VM tuning
echo "Tuning VM parameters..."
sysctl -w vm.swappiness=0                    # Never swap
sysctl -w vm.dirty_ratio=80                  # Allow more dirty pages in memory
sysctl -w vm.dirty_background_ratio=5
sysctl -w vm.dirty_expire_centisecs=12000
sysctl -w vm.zone_reclaim_mode=0             # Disable NUMA zone reclaim

echo ""

###############################################################################
# 4. NETWORK OPTIMIZATION
###############################################################################

echo -e "${GREEN}[4] Network Optimization${NC}"
echo "----------------------------------------------------------------------"

# TCP/IP stack tuning
echo "Tuning network stack for low latency..."

# TCP optimizations
sysctl -w net.ipv4.tcp_low_latency=1
sysctl -w net.ipv4.tcp_timestamps=0          # Reduce overhead
sysctl -w net.ipv4.tcp_sack=0                # Disable selective ACK
sysctl -w net.ipv4.tcp_window_scaling=1

# Increase buffer sizes
sysctl -w net.core.rmem_max=134217728        # 128MB
sysctl -w net.core.wmem_max=134217728
sysctl -w net.core.rmem_default=16777216     # 16MB
sysctl -w net.core.wmem_default=16777216
sysctl -w net.ipv4.tcp_rmem="4096 87380 134217728"
sysctl -w net.ipv4.tcp_wmem="4096 65536 134217728"

# Increase max connections
sysctl -w net.core.somaxconn=4096
sysctl -w net.ipv4.tcp_max_syn_backlog=8192

# Reduce TIME_WAIT
sysctl -w net.ipv4.tcp_fin_timeout=15
sysctl -w net.ipv4.tcp_tw_reuse=1

# Network device queue
sysctl -w net.core.netdev_max_backlog=10000

# Busy polling (poll NIC without interrupts)
sysctl -w net.core.busy_poll=50              # 50 microseconds
sysctl -w net.core.busy_read=50

echo "  Network stack: Optimized for low latency"
echo ""

###############################################################################
# 5. FILESYSTEM AND I/O OPTIMIZATION
###############################################################################

echo -e "${GREEN}[5] Filesystem and I/O Optimization${NC}"
echo "----------------------------------------------------------------------"

# I/O scheduler (use 'none' or 'noop' for SSDs/NVMe)
echo "Setting I/O scheduler to 'none' (for NVMe/SSD)..."
for disk in /sys/block/nvme*/queue/scheduler; do
    echo none > $disk 2>/dev/null || true
done

for disk in /sys/block/sd*/queue/scheduler; do
    echo noop > $disk 2>/dev/null || echo mq-deadline > $disk || true
done

# Increase file descriptor limits
echo "Increasing file descriptor limits..."
sysctl -w fs.file-max=2097152

# Check current limit
ulimit -n 1048576 2>/dev/null || echo "  Note: Run 'ulimit -n 1048576' in your shell"

echo "  I/O Scheduler: Optimized"
echo ""

###############################################################################
# 6. KERNEL PARAMETERS
###############################################################################

echo -e "${GREEN}[6] Kernel Parameters${NC}"
echo "----------------------------------------------------------------------"

# Disable address space layout randomization (ASLR)
echo "Disabling ASLR for deterministic performance..."
echo 0 > /proc/sys/kernel/randomize_va_space

# Real-time throttling (allow RT threads to use 100% CPU)
echo "Configuring real-time scheduling..."
sysctl -w kernel.sched_rt_runtime_us=-1      # No throttling

# Reduce context switch overhead
sysctl -w kernel.sched_migration_cost_ns=5000000  # 5ms

# Watchdog (disable to reduce interrupts)
sysctl -w kernel.nmi_watchdog=0 2>/dev/null || true

echo "  Kernel parameters: Optimized"
echo ""

###############################################################################
# 7. NUMA OPTIMIZATION (if applicable)
###############################################################################

echo -e "${GREEN}[7] NUMA Configuration${NC}"
echo "----------------------------------------------------------------------"

if command -v numactl &> /dev/null; then
    numactl --hardware | head -n 5
    echo ""
    echo -e "${YELLOW}For NUMA systems, run your HFT application with:${NC}"
    echo "  numactl --cpunodebind=0 --membind=0 ./hft_server"
else
    echo "  numactl not installed (install with: apt-get install numactl)"
fi

echo ""

###############################################################################
# 8. VERIFICATION AND STATUS
###############################################################################

echo -e "${GREEN}[8] System Status Verification${NC}"
echo "----------------------------------------------------------------------"

echo "CPU Information:"
echo "  Total CPUs: $(nproc)"
echo "  CPU Governor: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo 'N/A')"
echo "  Turbo Boost: $([ -f /sys/devices/system/cpu/intel_pstate/no_turbo ] && cat /sys/devices/system/cpu/intel_pstate/no_turbo | sed 's/0/Enabled/;s/1/Disabled/' || echo 'N/A')"

echo ""
echo "Memory Information:"
grep -i huge /proc/meminfo | head -n 10

echo ""
echo "Network Buffers:"
echo "  rmem_max: $(sysctl net.core.rmem_max | awk '{print $3}') bytes"
echo "  wmem_max: $(sysctl net.core.wmem_max | awk '{print $3}') bytes"

echo ""
echo "Swap Status:"
free -h | grep -i swap

echo ""
echo "Ulimits (file descriptors):"
echo "  soft: $(ulimit -Sn)"
echo "  hard: $(ulimit -Hn)"

echo ""

###############################################################################
# 9. PERSISTENCE
###############################################################################

echo -e "${GREEN}[9] Making Changes Persistent${NC}"
echo "----------------------------------------------------------------------"

# Create/update sysctl.conf
SYSCTL_CONF="/etc/sysctl.d/99-hft-optimization.conf"

echo "Creating persistent sysctl configuration: $SYSCTL_CONF"

cat > $SYSCTL_CONF <<EOF
# HFT System Optimization
# Generated on $(date)

# VM tuning
vm.swappiness=0
vm.dirty_ratio=80
vm.dirty_background_ratio=5
vm.zone_reclaim_mode=0

# Network tuning
net.ipv4.tcp_low_latency=1
net.ipv4.tcp_timestamps=0
net.ipv4.tcp_sack=0
net.core.rmem_max=134217728
net.core.wmem_max=134217728
net.core.rmem_default=16777216
net.core.wmem_default=16777216
net.ipv4.tcp_rmem=4096 87380 134217728
net.ipv4.tcp_wmem=4096 65536 134217728
net.core.somaxconn=4096
net.ipv4.tcp_max_syn_backlog=8192
net.ipv4.tcp_fin_timeout=15
net.ipv4.tcp_tw_reuse=1
net.core.netdev_max_backlog=10000
net.core.busy_poll=50
net.core.busy_read=50

# Kernel parameters
kernel.randomize_va_space=0
kernel.sched_rt_runtime_us=-1
kernel.sched_migration_cost_ns=5000000

# File system
fs.file-max=2097152
EOF

# Create rc.local for boot-time configuration
RC_LOCAL="/etc/rc.local"

echo "Creating boot-time configuration: $RC_LOCAL"

cat > $RC_LOCAL <<'EOF'
#!/bin/bash
# HFT System Boot-time Optimization

# CPU governor
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    echo "performance" > $cpu 2>/dev/null || true
done

# Disable turbo boost
echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo 2>/dev/null || true

# Disable C-states
for state in /sys/devices/system/cpu/cpu*/cpuidle/state*/disable; do
    echo 1 > $state 2>/dev/null || true
done

# Huge pages
echo 1024 > /proc/sys/vm/nr_hugepages

# Transparent huge pages
echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo always > /sys/kernel/mm/transparent_hugepage/defrag

# Disable swap
swapoff -a

# I/O scheduler
for disk in /sys/block/nvme*/queue/scheduler; do
    echo none > $disk 2>/dev/null || true
done

exit 0
EOF

chmod +x $RC_LOCAL

echo "  Persistent configuration files created"
echo ""

###############################################################################
# 10. FINAL INSTRUCTIONS
###############################################################################

echo "======================================================================"
echo -e "${GREEN}Optimization Complete!${NC}"
echo "======================================================================"
echo ""
echo -e "${YELLOW}IMPORTANT: For CPU isolation, you must:${NC}"
echo "  1. Edit /etc/default/grub"
echo "  2. Add to GRUB_CMDLINE_LINUX:"
echo "     isolcpus=$ISOLATED_CPUS nohz_full=$ISOLATED_CPUS rcu_nocbs=$ISOLATED_CPUS"
echo "  3. Run: sudo update-grub"
echo "  4. Reboot the system"
echo ""
echo -e "${YELLOW}To run your HFT application optimally:${NC}"
echo "  # Pin to isolated CPUs with real-time priority"
echo "  sudo chrt -f 99 taskset -c $ISOLATED_CPUS ./hft_server"
echo ""
echo "  # Or with NUMA binding"
echo "  sudo chrt -f 99 numactl --cpunodebind=0 --membind=0 taskset -c $ISOLATED_CPUS ./hft_server"
echo ""
echo -e "${YELLOW}To verify optimizations:${NC}"
echo "  cat /proc/cmdline     # Check boot parameters"
echo "  cat /proc/meminfo | grep Huge   # Check huge pages"
echo "  sysctl -a | grep -i tcp         # Check network settings"
echo ""
echo "======================================================================"

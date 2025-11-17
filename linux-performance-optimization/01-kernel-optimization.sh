#!/bin/bash
################################################################################
# Linux Kernel Optimization for High-Performance Trading Systems
#
# This script optimizes Linux kernel parameters for ultra-low latency trading
# Run with: sudo ./01-kernel-optimization.sh
################################################################################

set -e

echo "=========================================="
echo "Linux Kernel Optimization for Trading"
echo "=========================================="

# Backup current sysctl configuration
BACKUP_FILE="/etc/sysctl.d/99-trading-optimization.conf.backup.$(date +%Y%m%d_%H%M%S)"
if [ -f /etc/sysctl.conf ]; then
    cp /etc/sysctl.conf "$BACKUP_FILE"
    echo "Backed up sysctl.conf to $BACKUP_FILE"
fi

# Create optimized sysctl configuration
cat > /etc/sysctl.d/99-trading-optimization.conf << 'EOF'
################################################################################
# KERNEL SCHEDULER OPTIMIZATIONS
################################################################################

# Reduce scheduler migration cost (lower values = better for low latency)
kernel.sched_migration_cost_ns = 500000

# Minimum task running time before being preempted (nanoseconds)
kernel.sched_min_granularity_ns = 1000000

# Wake-up granularity (lower = more aggressive wake-ups)
kernel.sched_wakeup_granularity_ns = 1500000

# Scheduler latency (target time for all tasks to run once)
kernel.sched_latency_ns = 6000000

################################################################################
# MEMORY MANAGEMENT
################################################################################

# Reduce swappiness (0-100, lower = less swap usage)
vm.swappiness = 1

# Dirty page cache settings (faster writes, lower latency)
vm.dirty_ratio = 10
vm.dirty_background_ratio = 5
vm.dirty_expire_centisecs = 1000
vm.dirty_writeback_centisecs = 100

# Overcommit memory (better memory allocation)
vm.overcommit_memory = 1

# Transparent Huge Pages (enabled for better performance)
# Note: This is set via /sys interface below

# Zone reclaim mode (0 = allocate from other nodes if local node is full)
vm.zone_reclaim_mode = 0

# Min free kbytes (keep more memory free for allocations)
vm.min_free_kbytes = 1048576

################################################################################
# NETWORK STACK TUNING
################################################################################

# Increase network buffer sizes
net.core.rmem_max = 134217728
net.core.wmem_max = 134217728
net.core.rmem_default = 16777216
net.core.wmem_default = 16777216

# TCP buffer sizes (min, default, max)
net.ipv4.tcp_rmem = 4096 87380 134217728
net.ipv4.tcp_wmem = 4096 65536 134217728

# Increase network device backlog
net.core.netdev_max_backlog = 30000

# Enable TCP window scaling
net.ipv4.tcp_window_scaling = 1

# Increase max number of connections
net.core.somaxconn = 8192
net.ipv4.tcp_max_syn_backlog = 8192

# TCP Fast Open
net.ipv4.tcp_fastopen = 3

# Reduce TCP keepalive time
net.ipv4.tcp_keepalive_time = 300
net.ipv4.tcp_keepalive_probes = 5
net.ipv4.tcp_keepalive_intvl = 15

# Enable TCP timestamps (helps with RTT calculation)
net.ipv4.tcp_timestamps = 1

# Disable TCP slow start after idle
net.ipv4.tcp_slow_start_after_idle = 0

# TCP congestion control (BBR is best for low latency)
net.core.default_qdisc = fq
net.ipv4.tcp_congestion_control = bbr

################################################################################
# FILE SYSTEM & I/O
################################################################################

# Maximum number of file descriptors
fs.file-max = 2097152

# Increase inotify limits
fs.inotify.max_user_instances = 8192
fs.inotify.max_user_watches = 524288

################################################################################
# KERNEL PARAMETERS
################################################################################

# Disable core dumps (saves time on crashes)
kernel.core_pattern = /dev/null

# Increase PID max
kernel.pid_max = 4194304

# Message queue limits
kernel.msgmnb = 65536
kernel.msgmax = 65536

################################################################################
# SECURITY (Relaxed for performance)
################################################################################

# Disable kernel pointer restriction (for profiling)
kernel.kptr_restrict = 0

# Enable performance events
kernel.perf_event_paranoid = -1

EOF

# Apply sysctl settings
echo "Applying sysctl settings..."
sysctl -p /etc/sysctl.d/99-trading-optimization.conf

################################################################################
# TRANSPARENT HUGE PAGES
################################################################################

echo "Configuring Transparent Huge Pages..."

# Enable THP
echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo always > /sys/kernel/mm/transparent_hugepage/defrag

# Or for better latency, use madvise mode:
# echo madvise > /sys/kernel/mm/transparent_hugepage/enabled
# echo defer+madvise > /sys/kernel/mm/transparent_hugepage/defrag

echo "THP enabled: $(cat /sys/kernel/mm/transparent_hugepage/enabled)"

################################################################################
# CPU FREQUENCY SCALING
################################################################################

echo "Setting CPU governor to performance mode..."

# Install cpufrequtils if not present
if ! command -v cpupower &> /dev/null; then
    echo "cpupower not found. Install with: apt-get install linux-tools-common linux-tools-generic"
else
    # Set all CPUs to performance governor
    cpupower frequency-set -g performance

    # Disable turbo boost for consistent latency (optional)
    # echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo
fi

################################################################################
# DISABLE UNNECESSARY SERVICES
################################################################################

echo "Disabling unnecessary services for performance..."

# List of services that can impact latency
SERVICES_TO_DISABLE=(
    "bluetooth"
    "cups"
    "avahi-daemon"
)

for service in "${SERVICES_TO_DISABLE[@]}"; do
    if systemctl is-active --quiet "$service"; then
        echo "Disabling $service..."
        systemctl stop "$service" 2>/dev/null || true
        systemctl disable "$service" 2>/dev/null || true
    fi
done

################################################################################
# IRQ BALANCING
################################################################################

echo "Configuring IRQ balancing..."

# Disable irqbalance for manual CPU pinning
if systemctl is-active --quiet irqbalance; then
    echo "Stopping irqbalance (you'll manually pin IRQs)..."
    systemctl stop irqbalance
    systemctl disable irqbalance
fi

################################################################################
# GRUB CONFIGURATION
################################################################################

echo ""
echo "=========================================="
echo "GRUB Configuration Recommendations"
echo "=========================================="
echo ""
echo "Add the following to /etc/default/grub GRUB_CMDLINE_LINUX:"
echo ""
echo "GRUB_CMDLINE_LINUX=\"isolcpus=1-7 nohz_full=1-7 rcu_nocbs=1-7 intel_pstate=disable processor.max_cstate=1 intel_idle.max_cstate=0 idle=poll\""
echo ""
echo "Explanation:"
echo "  - isolcpus=1-7         : Isolate CPUs 1-7 from kernel scheduler (CPU 0 for OS)"
echo "  - nohz_full=1-7        : Disable timer ticks on isolated CPUs"
echo "  - rcu_nocbs=1-7        : Offload RCU callbacks from isolated CPUs"
echo "  - intel_pstate=disable : Disable Intel P-State driver (use acpi-cpufreq)"
echo "  - processor.max_cstate=1 : Limit C-states (prevent deep sleep)"
echo "  - intel_idle.max_cstate=0 : Disable intel_idle driver"
echo "  - idle=poll            : Poll instead of halt when idle (lowest latency)"
echo ""
echo "After editing /etc/default/grub, run:"
echo "  sudo update-grub"
echo "  sudo reboot"
echo ""

################################################################################
# ULIMITS
################################################################################

echo "Configuring user limits..."

cat > /etc/security/limits.d/99-trading.conf << 'EOF'
# User limits for trading applications
*    soft    nofile    1048576
*    hard    nofile    1048576
*    soft    nproc     unlimited
*    hard    nproc     unlimited
*    soft    memlock   unlimited
*    hard    memlock   unlimited
*    soft    stack     unlimited
*    hard    stack     unlimited
*    soft    cpu       unlimited
*    hard    cpu       unlimited
*    soft    rtprio    99
*    hard    rtprio    99
EOF

echo "User limits configured in /etc/security/limits.d/99-trading.conf"

################################################################################
# VERIFICATION
################################################################################

echo ""
echo "=========================================="
echo "Optimization Complete!"
echo "=========================================="
echo ""
echo "Current Settings:"
echo "  Swappiness: $(cat /proc/sys/vm/swappiness)"
echo "  THP: $(cat /sys/kernel/mm/transparent_hugepage/enabled)"
echo "  TCP Congestion Control: $(cat /proc/sys/net/ipv4/tcp_congestion_control)"
echo ""
echo "Next Steps:"
echo "1. Update GRUB configuration (see above)"
echo "2. Reboot the system"
echo "3. Run 02-cpu-isolation.sh to pin processes to specific CPUs"
echo "4. Run 03-memory-optimization.sh for hugepages setup"
echo ""

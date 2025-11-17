#!/bin/bash
################################################################################
# CPU Isolation and Process Affinity for Trading Applications
#
# This script pins trading processes to specific CPU cores
# Run with: sudo ./02-cpu-isolation.sh
################################################################################

set -e

echo "=========================================="
echo "CPU Isolation & Process Affinity Setup"
echo "=========================================="

# Get number of CPUs
NUM_CPUS=$(nproc)
echo "Total CPU cores: $NUM_CPUS"

# CPU allocation strategy:
# CPU 0: Operating System and background tasks
# CPU 1: Market Data Handler (critical path)
# CPU 2: Order Manager (critical path)
# CPU 3: Risk Manager
# CPU 4-N: Strategy threads, network IRQs

################################################################################
# FUNCTION: Set CPU affinity for a process
################################################################################

set_cpu_affinity() {
    local process_name=$1
    local cpu_list=$2

    echo "Setting CPU affinity for $process_name to CPUs: $cpu_list"

    # Find PIDs matching the process name
    pids=$(pgrep -f "$process_name" || true)

    if [ -z "$pids" ]; then
        echo "  WARNING: No process found matching '$process_name'"
        return 1
    fi

    for pid in $pids; do
        taskset -apc $cpu_list $pid
        echo "  Set CPU affinity for PID $pid"

        # Set scheduling priority to real-time FIFO
        chrt -f -p 95 $pid
        echo "  Set real-time priority (FIFO 95) for PID $pid"
    done
}

################################################################################
# FUNCTION: Set IRQ affinity
################################################################################

set_irq_affinity() {
    local irq=$1
    local cpu_mask=$2

    echo "Setting IRQ $irq to CPU mask $cpu_mask"
    echo "$cpu_mask" > /proc/irq/$irq/smp_affinity
}

################################################################################
# PIN TRADING PROCESSES
################################################################################

echo ""
echo "Pinning trading processes to CPUs..."
echo ""

# HFT System processes (adjust process names based on your binary names)
# CPU 1: Market Data Handler
set_cpu_affinity "hft_system.*market" "1" || echo "  (Process not running yet)"

# CPU 2: Order Manager
set_cpu_affinity "hft_system.*order" "2" || echo "  (Process not running yet)"

# CPU 3: Risk Manager
set_cpu_affinity "hft_system.*risk" "3" || echo "  (Process not running yet)"

# CPU 4-5: Trading Strategy
set_cpu_affinity "hft_system.*strategy" "4-5" || echo "  (Process not running yet)"

# Main HFT binary (if single process)
set_cpu_affinity "hft_system" "1-5" || echo "  (Main HFT process not running yet)"

################################################################################
# PIN NETWORK IRQs
################################################################################

echo ""
echo "Pinning network IRQs to dedicated CPUs..."
echo ""

# Find network interface IRQs
NETWORK_INTERFACE="eth0"  # Change to your network interface (eth0, ens3, etc.)

# Get IRQs for the network interface
irqs=$(grep "$NETWORK_INTERFACE" /proc/interrupts | awk '{print $1}' | sed 's/://' || true)

if [ -z "$irqs" ]; then
    echo "No IRQs found for interface $NETWORK_INTERFACE"
    echo "Available interfaces:"
    ip link show
else
    cpu_idx=6
    for irq in $irqs; do
        if [ $cpu_idx -ge $NUM_CPUS ]; then
            cpu_idx=4  # Wrap around if we run out of CPUs
        fi

        # Calculate CPU mask (2^cpu_idx in hex)
        cpu_mask=$(printf "%x" $((1 << cpu_idx)))

        set_irq_affinity "$irq" "$cpu_mask"
        cpu_idx=$((cpu_idx + 1))
    done
fi

################################################################################
# NUMA OPTIMIZATION
################################################################################

echo ""
echo "NUMA Configuration:"
echo ""

if command -v numactl &> /dev/null; then
    numactl --hardware

    echo ""
    echo "To run HFT system with NUMA binding:"
    echo "  numactl --cpunodebind=0 --membind=0 ./hft_system"
    echo ""
else
    echo "numactl not installed. Install with: apt-get install numactl"
fi

################################################################################
# CREATE HELPER SCRIPT
################################################################################

echo ""
echo "Creating helper script for starting HFT with optimal CPU pinning..."

cat > /home/user/trading-projects/start-hft-optimized.sh << 'EOFSCRIPT'
#!/bin/bash
################################################################################
# Start HFT System with Optimal CPU and Memory Binding
################################################################################

set -e

echo "Starting HFT System with Performance Optimizations..."

# Set CPU affinity and NUMA binding
# CPU 1-5: Trading application
# NUMA node 0: Memory binding

cd /home/user/trading-projects/HFT-System-CPP

# Option 1: Run with taskset (CPU affinity only)
# taskset -c 1-5 ./build/hft_system

# Option 2: Run with numactl (CPU + memory affinity)
if command -v numactl &> /dev/null; then
    numactl --physcpubind=1-5 --membind=0 ./build/hft_system
else
    echo "WARNING: numactl not found, using taskset only"
    taskset -c 1-5 ./build/hft_system
fi

EOFSCRIPT

chmod +x /home/user/trading-projects/start-hft-optimized.sh

echo "Created: /home/user/trading-projects/start-hft-optimized.sh"

################################################################################
# SYSTEMD SERVICE (Optional)
################################################################################

cat > /tmp/hft-trading.service << 'EOFSVC'
[Unit]
Description=HFT Trading System
After=network.target

[Service]
Type=simple
User=root
WorkingDirectory=/home/user/trading-projects/HFT-System-CPP
ExecStart=/bin/bash -c 'numactl --physcpubind=1-5 --membind=0 /home/user/trading-projects/HFT-System-CPP/build/hft_system'
Restart=always
RestartSec=5

# Resource limits
LimitNOFILE=1048576
LimitNPROC=unlimited
LimitMEMLOCK=unlimited

# Real-time scheduling
CPUSchedulingPolicy=fifo
CPUSchedulingPriority=95

# CPU affinity
CPUAffinity=1 2 3 4 5

[Install]
WantedBy=multi-user.target
EOFSVC

echo ""
echo "Systemd service template created at /tmp/hft-trading.service"
echo "To install:"
echo "  sudo cp /tmp/hft-trading.service /etc/systemd/system/"
echo "  sudo systemctl daemon-reload"
echo "  sudo systemctl enable hft-trading"
echo "  sudo systemctl start hft-trading"
echo ""

################################################################################
# MONITORING SCRIPT
################################################################################

cat > /home/user/trading-projects/monitor-cpu-affinity.sh << 'EOFMON'
#!/bin/bash
################################################################################
# Monitor CPU Affinity of Trading Processes
################################################################################

echo "Trading Process CPU Affinity Monitor"
echo "======================================"
echo ""

watch -n 1 '
echo "HFT Processes:"
ps aux | grep -E "hft_system|market_data|order_manager" | grep -v grep

echo ""
echo "CPU Affinity:"
for pid in $(pgrep -f "hft_system"); do
    echo "PID $pid: $(taskset -cp $pid 2>/dev/null | cut -d: -f2)"
done

echo ""
echo "CPU Usage per Core:"
mpstat -P ALL 1 1 | grep -E "CPU|Average"

echo ""
echo "Network IRQ Distribution:"
cat /proc/interrupts | grep -E "CPU|eth0|ens"
'
EOFMON

chmod +x /home/user/trading-projects/monitor-cpu-affinity.sh

echo "Created monitoring script: /home/user/trading-projects/monitor-cpu-affinity.sh"

################################################################################
# SUMMARY
################################################################################

echo ""
echo "=========================================="
echo "CPU Isolation Setup Complete!"
echo "=========================================="
echo ""
echo "Summary:"
echo "  - Trading processes pinned to isolated CPUs"
echo "  - Real-time scheduling priority set (FIFO 95)"
echo "  - Network IRQs distributed across CPUs"
echo "  - Helper scripts created"
echo ""
echo "Recommended CPU allocation:"
echo "  CPU 0: Operating System"
echo "  CPU 1: Market Data Handler"
echo "  CPU 2: Order Manager"
echo "  CPU 3: Risk Manager"
echo "  CPU 4-5: Trading Strategy"
echo "  CPU 6+: Network IRQs"
echo ""
echo "Start HFT with:"
echo "  /home/user/trading-projects/start-hft-optimized.sh"
echo ""
echo "Monitor with:"
echo "  /home/user/trading-projects/monitor-cpu-affinity.sh"
echo ""

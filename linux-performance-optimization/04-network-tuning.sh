#!/bin/bash
################################################################################
# Network Performance Tuning for Low-Latency Trading
#
# This script optimizes network stack for minimal latency
# Run with: sudo ./04-network-tuning.sh
################################################################################

set -e

echo "=========================================="
echo "Network Performance Optimization"
echo "=========================================="

# Detect primary network interface
INTERFACE=$(ip route | grep default | awk '{print $5}' | head -1)

if [ -z "$INTERFACE" ]; then
    echo "ERROR: Could not detect network interface"
    exit 1
fi

echo "Optimizing network interface: $INTERFACE"

################################################################################
# NETWORK INTERFACE CONFIGURATION
################################################################################

echo ""
echo "Configuring network interface parameters..."

# Check if ethtool is available
if ! command -v ethtool &> /dev/null; then
    echo "Installing ethtool..."
    apt-get install -y ethtool
fi

# Increase ring buffer sizes (RX/TX queues)
echo "Setting ring buffer sizes..."
ethtool -G $INTERFACE rx 4096 tx 4096 2>/dev/null || echo "  (Not supported on this NIC)"

# Enable hardware timestamping if available
echo "Configuring hardware timestamping..."
ethtool -T $INTERFACE

# Disable TCP segmentation offload for lower latency
echo "Disabling offload features for lower latency..."
ethtool -K $INTERFACE tso off gso off gro off 2>/dev/null || echo "  (Some features not supported)"

# Enable receive flow steering
echo "Configuring receive flow steering..."
ethtool -K $INTERFACE ntuple on rxhash on 2>/dev/null || echo "  (Not supported)"

# Display current settings
echo ""
echo "Current NIC settings:"
ethtool -g $INTERFACE
ethtool -k $INTERFACE | grep -E "tcp-segmentation-offload|generic-segmentation-offload|generic-receive-offload"

################################################################################
# NETWORK QUEUE CONFIGURATION
################################################################################

echo ""
echo "Configuring network queues..."

# Get number of RX queues
NUM_QUEUES=$(ls -d /sys/class/net/$INTERFACE/queues/rx-* 2>/dev/null | wc -l)
echo "Number of RX queues: $NUM_QUEUES"

# Set RX queue sizes
for queue in /sys/class/net/$INTERFACE/queues/rx-*; do
    queue_num=$(basename $queue | sed 's/rx-//')
    echo "Configuring RX queue $queue_num"

    # XPS (Transmit Packet Steering) - pin TX queue to specific CPU
    echo 1 > $queue/xps_cpus 2>/dev/null || true
done

################################################################################
# INTERRUPT COALESCING
################################################################################

echo ""
echo "Configuring interrupt coalescing..."

# Reduce interrupt coalescing for lower latency
# (Trade throughput for latency)
ethtool -C $INTERFACE rx-usecs 0 tx-usecs 0 2>/dev/null || echo "  (Not supported on this NIC)"

# Display current coalescing settings
echo ""
echo "Current interrupt coalescing:"
ethtool -c $INTERFACE 2>/dev/null || echo "  (Not supported)"

################################################################################
# RSS (RECEIVE SIDE SCALING)
################################################################################

echo ""
echo "Configuring RSS (Receive Side Scaling)..."

# Get RSS hash function
echo "RSS configuration:"
ethtool -x $INTERFACE 2>/dev/null || echo "  (Not supported on this NIC)"

# Set RSS hash key (optional - for advanced users)
# ethtool -X $INTERFACE equal 4

################################################################################
# NETWORK SYSCTL TUNING
################################################################################

echo ""
echo "Applying network sysctl optimizations..."

cat >> /etc/sysctl.d/99-trading-network.conf << 'EOF'
################################################################################
# NETWORK PERFORMANCE TUNING
################################################################################

# TCP buffer optimization (already in main config, but reinforced here)
net.core.rmem_max = 134217728
net.core.wmem_max = 134217728
net.core.rmem_default = 16777216
net.core.wmem_default = 16777216

net.ipv4.tcp_rmem = 4096 87380 134217728
net.ipv4.tcp_wmem = 4096 65536 134217728

# Increase netdev backlog
net.core.netdev_max_backlog = 100000

# Increase socket listen backlog
net.core.somaxconn = 8192
net.ipv4.tcp_max_syn_backlog = 8192

# TCP Fast Open
net.ipv4.tcp_fastopen = 3

# Disable TCP slow start after idle
net.ipv4.tcp_slow_start_after_idle = 0

# TCP timestamps (for accurate RTT)
net.ipv4.tcp_timestamps = 1

# Enable TCP window scaling
net.ipv4.tcp_window_scaling = 1

# BBR congestion control (best for low latency)
net.core.default_qdisc = fq
net.ipv4.tcp_congestion_control = bbr

# Reduce TIME_WAIT sockets
net.ipv4.tcp_fin_timeout = 15
net.ipv4.tcp_max_tw_buckets = 1440000
net.ipv4.tcp_tw_reuse = 1

# TCP keepalive
net.ipv4.tcp_keepalive_time = 300
net.ipv4.tcp_keepalive_probes = 5
net.ipv4.tcp_keepalive_intvl = 15

# Disable TCP selective acknowledgments (SACK) for lower latency
net.ipv4.tcp_sack = 0
net.ipv4.tcp_dsack = 0
net.ipv4.tcp_fack = 0

# Reduce TCP retries
net.ipv4.tcp_syn_retries = 3
net.ipv4.tcp_synack_retries = 3

# Enable TCP low latency mode
net.ipv4.tcp_low_latency = 1

# Increase local port range
net.ipv4.ip_local_port_range = 10000 65535

# Increase max number of connections
net.netfilter.nf_conntrack_max = 1000000

# UDP optimizations
net.ipv4.udp_rmem_min = 8192
net.ipv4.udp_wmem_min = 8192

# Disable IPv6 if not needed (reduces overhead)
net.ipv6.conf.all.disable_ipv6 = 1
net.ipv6.conf.default.disable_ipv6 = 1

# Reduce ARP cache timeout
net.ipv4.neigh.default.gc_thresh1 = 8192
net.ipv4.neigh.default.gc_thresh2 = 32768
net.ipv4.neigh.default.gc_thresh3 = 65536

# Enable IP forwarding (if running multiple network interfaces)
# net.ipv4.ip_forward = 1

# Disable ICMP redirects (security)
net.ipv4.conf.all.accept_redirects = 0
net.ipv4.conf.default.accept_redirects = 0

# Enable TCP MTU probing
net.ipv4.tcp_mtu_probing = 1

# Increase TCP max orphans
net.ipv4.tcp_max_orphans = 262144

# Network packet steering (RPS/RFS)
net.core.rps_sock_flow_entries = 32768

EOF

# Apply sysctl settings
sysctl -p /etc/sysctl.d/99-trading-network.conf

################################################################################
# RPS/RFS CONFIGURATION
################################################################################

echo ""
echo "Configuring RPS (Receive Packet Steering)..."

# Configure RPS for each RX queue
for queue in /sys/class/net/$INTERFACE/queues/rx-*/rps_cpus; do
    # Set CPU mask to use all CPUs except 0
    # Example: for 8 CPUs, mask = 0xFE (11111110 in binary = CPUs 1-7)
    NUM_CPUS=$(nproc)
    CPU_MASK=$(printf "0x%X" $((2**NUM_CPUS - 2)))

    echo $CPU_MASK > $queue
    echo "  Set RPS CPU mask to $CPU_MASK for $(dirname $queue)"
done

# Configure RFS (Receive Flow Steering)
echo "32768" > /proc/sys/net/core/rps_sock_flow_entries

for queue in /sys/class/net/$INTERFACE/queues/rx-*/rps_flow_cnt; do
    echo "2048" > $queue
done

################################################################################
# MULTICAST OPTIMIZATION
################################################################################

echo ""
echo "Configuring multicast (for market data feeds)..."

# Increase multicast membership limits
sysctl -w net.ipv4.igmp_max_memberships=512

# Add to sysctl config
cat >> /etc/sysctl.d/99-trading-network.conf << 'EOF'

# Multicast optimization
net.ipv4.igmp_max_memberships = 512
EOF

################################################################################
# CREATE NETWORK MONITORING SCRIPT
################################################################################

cat > /home/user/trading-projects/monitor-network.sh << 'EOFNET'
#!/bin/bash
################################################################################
# Network Performance Monitoring
################################################################################

INTERFACE=$(ip route | grep default | awk '{print $5}' | head -1)

echo "Network Performance Monitor - Interface: $INTERFACE"
echo "====================================================="
echo ""

# Network statistics
echo "=== Network Statistics ==="
ip -s link show $INTERFACE
echo ""

# Packet drops
echo "=== Packet Drops (should be zero) ==="
ethtool -S $INTERFACE | grep -i drop
echo ""

# Ring buffer utilization
echo "=== Ring Buffer Utilization ==="
ethtool -g $INTERFACE
echo ""

# Interrupt coalescing
echo "=== Interrupt Coalescing ==="
ethtool -c $INTERFACE
echo ""

# TCP statistics
echo "=== TCP Statistics ==="
ss -s
echo ""

# Connection tracking
echo "=== Active Connections ==="
ss -tan | grep ESTAB | wc -l
echo ""

# Network errors
echo "=== Network Errors ==="
netstat -i
echo ""

# IRQ counts
echo "=== Network IRQ Counts ==="
cat /proc/interrupts | grep -E "CPU|$INTERFACE"
echo ""

# Bandwidth usage
echo "=== Bandwidth Usage (real-time) ==="
echo "Press Ctrl+C to exit"
iftop -i $INTERFACE -t -s 1 2>/dev/null || echo "iftop not installed (apt-get install iftop)"
EOFNET

chmod +x /home/user/trading-projects/monitor-network.sh

echo ""
echo "Created monitoring script: /home/user/trading-projects/monitor-network.sh"

################################################################################
# LATENCY TESTING SCRIPT
################################################################################

cat > /home/user/trading-projects/test-network-latency.sh << 'EOFLAT'
#!/bin/bash
################################################################################
# Network Latency Testing
################################################################################

TARGET_HOST=${1:-"8.8.8.8"}

echo "Network Latency Test to $TARGET_HOST"
echo "======================================"
echo ""

# Ping test
echo "=== Ping Test (ICMP) ==="
ping -c 100 -i 0.2 $TARGET_HOST | tail -2
echo ""

# TCP latency (using hping3 if available)
if command -v hping3 &> /dev/null; then
    echo "=== TCP SYN Latency ==="
    sudo hping3 -S -c 100 -p 80 $TARGET_HOST 2>&1 | tail -2
    echo ""
fi

# Application-level latency (HTTP)
echo "=== HTTP Latency ==="
for i in {1..10}; do
    curl -w "Time: %{time_total}s\n" -o /dev/null -s http://$TARGET_HOST
done
echo ""

# DNS latency
echo "=== DNS Resolution Latency ==="
for i in {1..10}; do
    /usr/bin/time -f "Time: %E" dig @8.8.8.8 google.com > /dev/null 2>&1
done
echo ""

echo "Recommendation: Latency should be <1ms for local network, <10ms for exchange connectivity"
EOFLAT

chmod +x /home/user/trading-projects/test-network-latency.sh

echo ""
echo "Created latency testing script: /home/user/trading-projects/test-network-latency.sh"

################################################################################
# VERIFICATION
################################################################################

echo ""
echo "=========================================="
echo "Network Optimization Complete!"
echo "=========================================="
echo ""
echo "Current Configuration:"
echo "  Interface: $INTERFACE"
echo "  Ring buffer size: $(ethtool -g $INTERFACE 2>/dev/null | grep -A1 "^RX:" | tail -1 | awk '{print $2}' || echo 'N/A')"
echo "  TCP Congestion Control: $(cat /proc/sys/net/ipv4/tcp_congestion_control)"
echo "  Network backlog: $(cat /proc/sys/net/core/netdev_max_backlog)"
echo ""
echo "Next Steps:"
echo "1. Monitor network with: /home/user/trading-projects/monitor-network.sh"
echo "2. Test latency with: /home/user/trading-projects/test-network-latency.sh <host>"
echo "3. Verify packet drops are zero"
echo "4. Pin network IRQs to specific CPUs (done in 02-cpu-isolation.sh)"
echo ""
echo "For exchange connectivity, ensure:"
echo "  - Direct network connection (no Wi-Fi)"
echo "  - Jumbo frames enabled if supported (MTU 9000)"
echo "  - Hardware timestamping enabled"
echo "  - Kernel bypass (DPDK) for <1μs latency (advanced)"
echo ""

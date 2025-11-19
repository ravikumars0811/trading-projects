#!/bin/bash
################################################################################
# Real-Time Performance Monitoring Dashboard
#
# Displays comprehensive system and application metrics
# Run with: ./performance-dashboard.sh
################################################################################

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print section headers
print_header() {
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
}

# Function to check if value is in acceptable range
check_threshold() {
    local value=$1
    local threshold=$2
    local operator=$3

    if [ "$operator" = "lt" ]; then
        if (( $(echo "$value < $threshold" | bc -l) )); then
            echo -e "${GREEN}✓${NC}"
        else
            echo -e "${RED}✗${NC}"
        fi
    elif [ "$operator" = "gt" ]; then
        if (( $(echo "$value > $threshold" | bc -l) )); then
            echo -e "${GREEN}✓${NC}"
        else
            echo -e "${RED}✗${NC}"
        fi
    fi
}

# Main monitoring loop
while true; do
    clear

    echo -e "${GREEN}╔═══════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║     HFT TRADING SYSTEM - PERFORMANCE DASHBOARD           ║${NC}"
    echo -e "${GREEN}║     $(date '+%Y-%m-%d %H:%M:%S')                              ║${NC}"
    echo -e "${GREEN}╚═══════════════════════════════════════════════════════════╝${NC}"
    echo ""

    # ========================================
    # CPU PERFORMANCE
    # ========================================
    print_header "CPU PERFORMANCE"

    # Overall CPU usage
    cpu_usage=$(top -bn1 | grep "Cpu(s)" | awk '{print $2}' | cut -d'%' -f1)
    echo -e "Overall CPU Usage:     ${YELLOW}${cpu_usage}%${NC}"

    # Per-core usage
    echo ""
    echo "Per-Core CPU Usage:"
    mpstat -P ALL 1 1 | grep -v "Linux\|^$\|Average" | awk '{
        if (NR==1) {
            printf "%-8s %10s %10s %10s %10s\n", "CPU", "User%", "System%", "Idle%", "Status"
            printf "%-8s %10s %10s %10s %10s\n", "---", "-----", "-------", "-----", "------"
        } else if (NR>2) {
            idle = $NF
            status = (idle < 20) ? "BUSY" : (idle < 50) ? "MODERATE" : "IDLE"
            color = (idle < 20) ? "\033[0;31m" : (idle < 50) ? "\033[1;33m" : "\033[0;32m"
            printf "%-8s %9s%% %9s%% %9s%% %s%10s\033[0m\n", $2, $3, $5, $NF, color, status
        }
    }'

    # CPU frequency
    echo ""
    echo "CPU Frequency Scaling:"
    if command -v cpupower &> /dev/null; then
        cpupower frequency-info | grep "current CPU frequency" | head -1
    else
        cat /proc/cpuinfo | grep "cpu MHz" | head -1
    fi

    # Context switches
    context_switches=$(vmstat 1 2 | tail -1 | awk '{print $12}')
    echo "Context Switches/sec:  ${context_switches}"

    echo ""

    # ========================================
    # MEMORY PERFORMANCE
    # ========================================
    print_header "MEMORY PERFORMANCE"

    # Memory usage
    free -h | awk 'NR==1{print "               "$2"    "$3"    "$4"    "$6}
                   NR==2{printf "Memory:        %-7s %-7s %-7s %-7s\n", $2, $3, $4, $6}'

    # Swap usage (should be minimal)
    swap_used=$(free | grep Swap | awk '{print $3}')
    swap_total=$(free | grep Swap | awk '{print $2}')

    if [ "$swap_total" -gt 0 ]; then
        swap_percent=$(echo "scale=2; $swap_used * 100 / $swap_total" | bc)
        if (( $(echo "$swap_percent > 1" | bc -l) )); then
            echo -e "Swap Usage:        ${RED}${swap_percent}% (WARNING: Swap is being used!)${NC}"
        else
            echo -e "Swap Usage:        ${GREEN}${swap_percent}% (Good)${NC}"
        fi
    fi

    # Hugepages
    echo ""
    echo "Hugepages:"
    grep -E "HugePages_Total|HugePages_Free|Hugepagesize" /proc/meminfo | \
        awk '{printf "  %-20s %s %s\n", $1, $2, $3}'

    # Page faults for trading processes
    if pgrep -f "hft_system" > /dev/null; then
        echo ""
        echo "HFT Process Memory:"
        for pid in $(pgrep -f "hft_system"); do
            ps -p $pid -o pid,rss,vsz,min_flt,maj_flt,comm | \
                awk 'NR==1{printf "  %-8s %10s %10s %10s %10s %s\n", $1,$2,$3,$4,$5,$6}
                     NR>1{printf "  %-8s %9sK %9sK %10s %10s %s\n", $1,$2,$3,$4,$5,$6}'
        done
    fi

    echo ""

    # ========================================
    # NETWORK PERFORMANCE
    # ========================================
    print_header "NETWORK PERFORMANCE"

    # Detect network interface
    INTERFACE=$(ip route | grep default | awk '{print $5}' | head -1)

    if [ -n "$INTERFACE" ]; then
        echo "Interface: $INTERFACE"
        echo ""

        # Network statistics
        rx_packets_before=$(cat /sys/class/net/$INTERFACE/statistics/rx_packets)
        tx_packets_before=$(cat /sys/class/net/$INTERFACE/statistics/tx_packets)
        rx_bytes_before=$(cat /sys/class/net/$INTERFACE/statistics/rx_bytes)
        tx_bytes_before=$(cat /sys/class/net/$INTERFACE/statistics/tx_bytes)

        sleep 1

        rx_packets_after=$(cat /sys/class/net/$INTERFACE/statistics/rx_packets)
        tx_packets_after=$(cat /sys/class/net/$INTERFACE/statistics/tx_packets)
        rx_bytes_after=$(cat /sys/class/net/$INTERFACE/statistics/rx_bytes)
        tx_bytes_after=$(cat /sys/class/net/$INTERFACE/statistics/tx_bytes)

        rx_pps=$((rx_packets_after - rx_packets_before))
        tx_pps=$((tx_packets_after - tx_packets_before))
        rx_bps=$(((rx_bytes_after - rx_bytes_before) * 8 / 1000000))  # Mbps
        tx_bps=$(((tx_bytes_after - tx_bytes_before) * 8 / 1000000))  # Mbps

        echo "RX: ${rx_pps} packets/sec, ${rx_bps} Mbps"
        echo "TX: ${tx_pps} packets/sec, ${tx_bps} Mbps"

        # Packet drops
        echo ""
        echo "Packet Drops:"
        if command -v ethtool &> /dev/null; then
            ethtool -S $INTERFACE 2>/dev/null | grep -i drop | head -5 || echo "  No drops detected"
        fi

        # Connection stats
        echo ""
        established=$(ss -tan | grep ESTAB | wc -l)
        time_wait=$(ss -tan | grep TIME-WAIT | wc -l)
        echo "Established connections: $established"
        echo "TIME_WAIT connections:   $time_wait"
    fi

    echo ""

    # ========================================
    # DISK I/O PERFORMANCE
    # ========================================
    print_header "DISK I/O PERFORMANCE"

    # Disk I/O statistics
    iostat -x 1 2 | tail -n +4 | awk '
        NR==1 {
            printf "%-12s %10s %10s %10s %10s\n", "Device", "r/s", "w/s", "rMB/s", "wMB/s"
            printf "%-12s %10s %10s %10s %10s\n", "------", "---", "---", "-----", "-----"
        }
        NR>1 && NF>0 {
            printf "%-12s %10.2f %10.2f %10.2f %10.2f\n", $1, $4, $5, $6/1024, $7/1024
        }
    ' 2>/dev/null || echo "iostat not available (apt-get install sysstat)"

    echo ""

    # ========================================
    # TRADING APPLICATION METRICS
    # ========================================
    print_header "TRADING APPLICATION METRICS"

    if pgrep -f "hft_system" > /dev/null; then
        echo "HFT System Status: ${GREEN}RUNNING${NC}"
        echo ""

        # Process info
        for pid in $(pgrep -f "hft_system"); do
            echo "Process $pid:"
            ps -p $pid -o pid,pri,nice,rtprio,cls,psr,comm | \
                awk 'NR==1{print "  "$0} NR>1{print "  "$0}'

            # Thread count
            threads=$(ps -p $pid -L | wc -l)
            echo "  Threads: $((threads - 1))"

            # CPU affinity
            affinity=$(taskset -cp $pid 2>/dev/null | cut -d: -f2)
            echo "  CPU Affinity:$affinity"
        done

        # Application logs (if available)
        if [ -f "/var/log/hft_system.log" ]; then
            echo ""
            echo "Recent Log Entries:"
            tail -5 /var/log/hft_system.log | sed 's/^/  /'
        fi
    else
        echo -e "HFT System Status: ${RED}NOT RUNNING${NC}"
    fi

    echo ""

    # ========================================
    # LATENCY METRICS
    # ========================================
    print_header "LATENCY METRICS"

    # System latency
    if command -v cyclictest &> /dev/null; then
        echo "Running cyclictest (5 sec sample)..."
        cyclictest -t1 -n -p95 -D 5 2>/dev/null | grep -E "Min|Avg|Max" || \
            echo "Run: apt-get install rt-tests"
    else
        echo "cyclictest not installed (apt-get install rt-tests)"
    fi

    # Network latency
    echo ""
    echo "Network Latency (ping to 8.8.8.8):"
    ping -c 3 8.8.8.8 2>/dev/null | tail -1 || echo "  Network unreachable"

    echo ""

    # ========================================
    # OPTIMIZATION CHECKLIST
    # ========================================
    print_header "OPTIMIZATION CHECKLIST"

    echo -n "CPU Governor (performance):      "
    gov=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo "unknown")
    if [ "$gov" = "performance" ]; then
        echo -e "${GREEN}✓ $gov${NC}"
    else
        echo -e "${RED}✗ $gov (should be 'performance')${NC}"
    fi

    echo -n "Transparent Huge Pages:          "
    thp=$(cat /sys/kernel/mm/transparent_hugepage/enabled 2>/dev/null | grep -o "\[.*\]" | tr -d '[]')
    if [ "$thp" = "always" ] || [ "$thp" = "madvise" ]; then
        echo -e "${GREEN}✓ $thp${NC}"
    else
        echo -e "${YELLOW}⚠ $thp${NC}"
    fi

    echo -n "Swappiness:                      "
    swappiness=$(cat /proc/sys/vm/swappiness)
    if [ "$swappiness" -le 10 ]; then
        echo -e "${GREEN}✓ $swappiness${NC}"
    else
        echo -e "${YELLOW}⚠ $swappiness (should be ≤10)${NC}"
    fi

    echo -n "IRQ Balance:                     "
    if systemctl is-active --quiet irqbalance; then
        echo -e "${YELLOW}⚠ ENABLED (should be disabled for manual pinning)${NC}"
    else
        echo -e "${GREEN}✓ DISABLED${NC}"
    fi

    echo -n "TCP Congestion Control:          "
    tcp_cc=$(cat /proc/sys/net/ipv4/tcp_congestion_control)
    if [ "$tcp_cc" = "bbr" ]; then
        echo -e "${GREEN}✓ $tcp_cc${NC}"
    else
        echo -e "${YELLOW}⚠ $tcp_cc (bbr recommended)${NC}"
    fi

    echo ""
    echo -e "${BLUE}Press Ctrl+C to exit${NC}"
    echo ""

    # Refresh every 5 seconds
    sleep 5
done

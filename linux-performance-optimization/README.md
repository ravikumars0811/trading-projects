# Linux Performance Optimization Scripts

This directory contains comprehensive performance optimization scripts for high-frequency trading systems on Linux.

## Quick Start

Run all scripts in order:

```bash
cd /home/user/trading-projects/linux-performance-optimization

# 1. Kernel optimization (run as root)
sudo ./01-kernel-optimization.sh

# 2. CPU isolation and process pinning
sudo ./02-cpu-isolation.sh

# 3. Memory and hugepages
sudo ./03-memory-optimization.sh

# 4. Network tuning
sudo ./04-network-tuning.sh

# 5. Real-time monitoring dashboard
./performance-dashboard.sh
```

## Scripts Overview

### 01-kernel-optimization.sh
Optimizes Linux kernel parameters for low-latency trading:
- Scheduler tuning (reduced migration cost, lower granularity)
- Memory management (minimal swap, optimized dirty pages)
- Network stack (BBR congestion control, larger buffers)
- File system limits
- User limits (ulimits)

**Run as**: `sudo`
**Duration**: 1-2 minutes
**Requires reboot**: No (but GRUB update recommended)

### 02-cpu-isolation.sh
Configures CPU isolation and process affinity:
- Isolates CPUs from kernel scheduler
- Pins trading processes to specific cores
- Sets real-time scheduling priorities
- Distributes network IRQs
- Creates helper scripts for optimal startup

**Run as**: `sudo`
**Duration**: 1 minute
**Requires reboot**: After GRUB update

### 03-memory-optimization.sh
Configures hugepages and memory optimizations:
- Allocates 2MB hugepages (reduces TLB misses)
- Mounts hugetlbfs
- Creates hugepage allocator for C++
- Configures NUMA memory policy
- Memory pool configuration

**Run as**: `sudo`
**Duration**: 1 minute
**Requires reboot**: No

### 04-network-tuning.sh
Optimizes network stack for minimal latency:
- Increases NIC ring buffers
- Disables offloading features (TSO, GSO, GRO)
- Configures interrupt coalescing
- Enables RPS/RFS (Receive Packet Steering)
- Tunes TCP/UDP parameters
- Configures multicast

**Run as**: `sudo`
**Duration**: 1-2 minutes
**Requires reboot**: No

### performance-dashboard.sh
Real-time performance monitoring dashboard:
- CPU usage per core
- Memory and hugepage statistics
- Network throughput and latency
- Disk I/O performance
- Trading application metrics
- Optimization checklist

**Run as**: Regular user
**Duration**: Continuous (Ctrl+C to exit)
**Requires reboot**: No

## Expected Performance Improvements

| Metric | Before | After | Improvement |
|--------|--------|-------|-------------|
| Order processing latency | 500ns | 100-150ns | 3-5x |
| Market data throughput | 10K msg/s | 100K+ msg/s | 10x |
| Options pricing | 5K/s | 50K+/s | 10x |
| Network latency | Variable | <100μs | Consistent |
| Context switches | High | Minimal | Isolated CPUs |

## Prerequisites

### Required Packages

```bash
sudo apt-get update
sudo apt-get install -y \
    sysstat \
    cpufrequtils \
    linux-tools-common \
    linux-tools-generic \
    ethtool \
    numactl \
    net-tools \
    iproute2 \
    bc
```

### System Requirements

- Linux kernel 4.14+ (5.x recommended)
- Root/sudo access
- x86_64 architecture
- 8+ CPU cores (16+ recommended)
- 16GB+ RAM (32GB+ recommended)
- 10Gbps network interface (for HFT)

## Important Notes

### CPU Isolation

After running the optimization scripts, you **must** update GRUB configuration:

1. Edit `/etc/default/grub`:
   ```bash
   sudo nano /etc/default/grub
   ```

2. Add to `GRUB_CMDLINE_LINUX`:
   ```
   isolcpus=1-7 nohz_full=1-7 rcu_nocbs=1-7 intel_pstate=disable processor.max_cstate=1 intel_idle.max_cstate=0 idle=poll
   ```

3. Update GRUB and reboot:
   ```bash
   sudo update-grub
   sudo reboot
   ```

### Verification

After reboot, verify optimizations:

```bash
# Check CPU isolation
cat /proc/cmdline | grep isolcpus

# Check hugepages
grep HugePages /proc/meminfo

# Check TCP congestion control
cat /proc/sys/net/ipv4/tcp_congestion_control

# Check swappiness
cat /proc/sys/vm/swappiness

# Run dashboard
./performance-dashboard.sh
```

## Monitoring Scripts

Additional monitoring scripts created by the optimization process:

- `/home/user/trading-projects/monitor-memory.sh` - Memory monitoring
- `/home/user/trading-projects/monitor-network.sh` - Network monitoring
- `/home/user/trading-projects/monitor-cpu-affinity.sh` - CPU affinity monitoring
- `/home/user/trading-projects/check-numa-allocation.sh` - NUMA monitoring
- `/home/user/trading-projects/test-network-latency.sh` - Network latency testing

## Helper Scripts

- `/home/user/trading-projects/start-hft-optimized.sh` - Start HFT with optimal settings

## Troubleshooting

### Scripts fail with permission errors
Run with `sudo`:
```bash
sudo ./01-kernel-optimization.sh
```

### GRUB update fails
Ensure GRUB is installed:
```bash
sudo apt-get install grub2-common
```

### Network script can't find interface
Manually set interface in script:
```bash
INTERFACE="eth0"  # Or your interface name
```

### Hugepages not allocating
Check available memory:
```bash
free -h
```

Reduce hugepage count in `03-memory-optimization.sh` if needed.

### CPU isolation not working
Verify GRUB configuration:
```bash
cat /proc/cmdline
```

Ensure you've rebooted after GRUB update.

## Rollback

To restore original settings:

```bash
# Restore sysctl
sudo cp /etc/sysctl.d/99-trading-optimization.conf.backup.* /etc/sysctl.conf
sudo sysctl -p

# Remove GRUB parameters
sudo nano /etc/default/grub  # Remove added parameters
sudo update-grub
sudo reboot

# Clear hugepages
echo 0 | sudo tee /proc/sys/vm/nr_hugepages
```

## Performance Testing

After optimization, benchmark your system:

```bash
# Build HFT system with optimizations
cd /home/user/trading-projects/HFT-System-CPP
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native" ..
make -j$(nproc)

# Run with monitoring
/home/user/trading-projects/start-hft-optimized.sh &
/home/user/trading-projects/linux-performance-optimization/performance-dashboard.sh
```

## Support

For detailed guidance, see:
- [Performance Optimization Guide](/home/user/trading-projects/PERFORMANCE_OPTIMIZATION_GUIDE.md)
- Individual script comments (extensive documentation in each script)

## License

These scripts are provided for educational and production use.

## Changelog

### Version 1.0 (2025-11-17)
- Initial release
- Kernel, CPU, memory, and network optimizations
- Real-time monitoring dashboard
- Helper scripts for deployment

---

**Last Updated**: 2025-11-17

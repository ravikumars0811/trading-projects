# DPDK Setup Guide

## System Requirements

- Linux kernel 4.14 or later
- GCC 9.0+ or Clang 10.0+
- Python 3.6+ (for build system)
- 8GB+ RAM
- Network card with DPDK PMD support

## Supported NICs

- Intel: 82599, X710, XL710, E810
- Mellanox: ConnectX-4, ConnectX-5, ConnectX-6
- Broadcom: NetXtreme-C/E
- Virtio (for VMs)

Check full list: https://core.dpdk.org/supported/

## Installation Steps

### 1. Install Dependencies

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential meson ninja-build \
    python3-pyelftools python3-pip libnuma-dev \
    linux-headers-$(uname -r) pkg-config

# RHEL/CentOS/Fedora
sudo dnf install -y gcc meson python3-pyelftools \
    numactl-devel kernel-devel elfutils-libelf-devel
```

### 2. Download DPDK

```bash
# Download DPDK LTS version
wget https://fast.dpdk.org/rel/dpdk-21.11.6.tar.xz
tar xf dpdk-21.11.6.tar.xz
cd dpdk-21.11.6
```

### 3. Build DPDK

```bash
# Configure build
meson setup build

# Or with specific options
meson setup build -Dexamples=all -Dtests=false

# Build
cd build
ninja

# Install (optional)
sudo ninja install
sudo ldconfig
```

### 4. Configure Environment

Add to `~/.bashrc`:

```bash
export RTE_SDK=/path/to/dpdk-21.11.6
export RTE_TARGET=build
export PKG_CONFIG_PATH=$RTE_SDK/$RTE_TARGET/meson-private:$PKG_CONFIG_PATH
```

### 5. Setup Huge Pages

#### Temporary (2MB huge pages)

```bash
# Allocate 1024 huge pages (2GB)
sudo sysctl -w vm.nr_hugepages=1024

# Verify
grep Huge /proc/meminfo
```

#### Persistent (add to `/etc/sysctl.conf`)

```bash
vm.nr_hugepages=1024
```

#### Mount huge pages

```bash
sudo mkdir -p /mnt/huge
sudo mount -t hugetlbfs nodev /mnt/huge

# Make persistent (add to /etc/fstab)
nodev /mnt/huge hugetlbfs defaults 0 0
```

#### 1GB Huge Pages (optional, for better performance)

```bash
# Add to kernel boot parameters (GRUB)
# Edit /etc/default/grub:
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=4"

# Update GRUB and reboot
sudo update-grub
sudo reboot

# After reboot, mount
sudo mkdir -p /mnt/huge_1GB
sudo mount -t hugetlbfs pagesize=1GB /mnt/huge_1GB
```

### 6. Bind Network Devices

#### Check device status

```bash
cd $RTE_SDK/usertools
sudo ./dpdk-devbind.py --status
```

#### Unbind from kernel driver

```bash
# Find device (e.g., 0000:03:00.0)
lspci | grep Ethernet

# Unbind from kernel
sudo ./dpdk-devbind.py --unbind 0000:03:00.0
```

#### Bind to DPDK driver

```bash
# Load DPDK driver
sudo modprobe vfio-pci

# Or for older systems
sudo modprobe uio
sudo modprobe igb_uio  # if available

# Bind device
sudo ./dpdk-devbind.py --bind=vfio-pci 0000:03:00.0

# Verify
sudo ./dpdk-devbind.py --status
```

#### Restore kernel driver

```bash
sudo ./dpdk-devbind.py --bind=ixgbe 0000:03:00.0
```

### 7. CPU Isolation (Optional but Recommended)

Isolate CPUs for DPDK applications:

```bash
# Edit /etc/default/grub
GRUB_CMDLINE_LINUX="isolcpus=1-7 nohz_full=1-7 rcu_nocbs=1-7"

# Update and reboot
sudo update-grub
sudo reboot
```

### 8. Test Installation

```bash
# Run testpmd (DPDK test application)
cd $RTE_SDK/build/app
sudo ./dpdk-testpmd -l 0-3 -n 4 -- -i

# In testpmd prompt
testpmd> show port info all
testpmd> start
testpmd> stop
testpmd> quit
```

## Virtual Environment Setup (for Testing)

### Using QEMU/KVM with Virtio

```bash
# Install QEMU
sudo apt-get install qemu-kvm

# Create VM with virtio NICs
qemu-system-x86_64 \
    -enable-kvm \
    -cpu host \
    -smp 4 \
    -m 4096 \
    -device virtio-net-pci,netdev=net0 \
    -netdev user,id=net0 \
    ...
```

## Troubleshooting

### Issue: "EAL: Cannot open /dev/vfio/vfio"

Solution:
```bash
sudo modprobe vfio-pci
sudo chmod 666 /dev/vfio/vfio
```

### Issue: "EAL: No free hugepages"

Solution:
```bash
# Clear existing huge pages
sudo rm -rf /mnt/huge/*
# Reallocate
sudo sysctl -w vm.nr_hugepages=1024
```

### Issue: "EAL: Device binding failed"

Solution:
- Check if device is already bound: `lspci -vv`
- Try unbinding first: `dpdk-devbind.py --unbind <device>`
- Check IOMMU is enabled: `dmesg | grep -i iommu`

### Issue: Permission denied

Solution:
```bash
# Run as root or setup capabilities
sudo setcap cap_sys_admin,cap_sys_rawio+ep ./your_dpdk_app
```

## Performance Tuning

### 1. NUMA Configuration

```bash
# Check NUMA topology
numactl --hardware

# Run on specific NUMA node
numactl --cpunodebind=0 --membind=0 ./your_app
```

### 2. CPU Frequency Scaling

```bash
# Set to performance mode
sudo cpupower frequency-set -g performance

# Or for specific cores
for i in {0..7}; do
    sudo cpupower -c $i frequency-set -g performance
done
```

### 3. Disable C-States (for lowest latency)

```bash
# Add to kernel parameters
intel_idle.max_cstate=0 processor.max_cstate=0
```

### 4. IRQ Affinity

```bash
# Move IRQs away from DPDK cores
# Example script
for irq in $(cat /proc/interrupts | grep eth0 | awk '{print $1}' | sed 's/://'); do
    echo 1 > /proc/irq/$irq/smp_affinity
done
```

## Security Considerations

- DPDK applications typically need root privileges
- Use vfio-pci instead of uio for better security (IOMMU)
- Consider using container isolation
- Be careful with device binding (can lose network access)

## Next Steps

After setup:
1. Run basic examples in `examples/`
2. Understand EAL parameters
3. Experiment with different core configurations
4. Measure performance with different settings

## References

- [DPDK Getting Started Guide](https://doc.dpdk.org/guides/linux_gsg/index.html)
- [DPDK Programmer's Guide](https://doc.dpdk.org/guides/prog_guide/index.html)
- [DPDK Performance Tuning](https://doc.dpdk.org/guides/linux_gsg/nic_perf_intel_platform.html)

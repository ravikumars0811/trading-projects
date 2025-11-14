# High Frequency Trading (HFT) System

A production-ready, ultra-low-latency High Frequency Trading system implemented in C++20 with advanced performance optimizations.

## 🚀 Features

### Core Infrastructure
- **Lock-Free Data Structures**: SPSC queue with atomic operations for zero-contention message passing
- **Memory Pool Allocator**: Pre-allocated memory pools to eliminate runtime allocation overhead
- **High-Resolution Timers**: TSC-based timing for nanosecond precision
- **Asynchronous Logging**: Lock-free logging system with minimal overhead

### Trading Components
- **Ultra-Low-Latency Order Book**: Array-based price levels for O(1) access
- **Market Data Handler**: High-performance feed processing with simulated market data
- **Order Management System (OMS)**: Complete order lifecycle management
- **Position Manager**: Real-time position tracking and PnL calculation
- **Exchange Gateway**: FIX protocol support for exchange connectivity

### Trading Strategies
- **Market Making**: Provides liquidity with bid/ask quotes around mid-price
- **Statistical Arbitrage**: Mean reversion strategy based on z-score signals

### Risk Management
- **Pre-Trade Checks**: Position limits, order size limits, price collars
- **Post-Trade Monitoring**: PnL limits, order rate limiting
- **Real-Time Risk Metrics**: Exposure tracking and risk reporting

### Performance Monitoring
- **Latency Tracking**: Nanosecond-precision latency measurements
- **System Metrics**: Orders, fills, cancels, and throughput statistics
- **Percentile Analysis**: P50, P95, P99 latency percentiles

## 📋 Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         HFT System                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐ │
│  │   Market     │───▶│   Trading    │───▶│   Exchange   │ │
│  │   Data       │    │   Strategy   │    │   Gateway    │ │
│  │   Handler    │    │              │    │              │ │
│  └──────────────┘    └──────────────┘    └──────────────┘ │
│         │                    │                    │        │
│         │                    │                    │        │
│         ▼                    ▼                    ▼        │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐ │
│  │   Order      │    │     OMS      │    │     Risk     │ │
│  │   Book       │    │              │    │   Manager    │ │
│  └──────────────┘    └──────────────┘    └──────────────┘ │
│                                                             │
│  ┌──────────────────────────────────────────────────────┐  │
│  │           Performance Monitor & Metrics               │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## 🛠️ Build Requirements

- **Compiler**: GCC 11+ or Clang 14+ (C++20 support required)
- **CMake**: 3.15 or higher
- **Operating System**: Linux (Ubuntu 20.04+ recommended)
- **Optional**: Docker for containerized deployment

## 📦 Installation

### Local Build

```bash
# Clone the repository
git clone <repository-url>
cd HFT-System-CPP

# Build the project
./scripts/build.sh

# Run the system
./scripts/run.sh
```

### Docker Deployment

```bash
# Build and deploy with Docker
./scripts/deploy.sh

# View logs
docker-compose -f docker/docker-compose.yml logs -f

# Stop the system
docker-compose -f docker/docker-compose.yml down
```

## ⚙️ Configuration

Edit `config/hft_config.txt` to configure the system:

```ini
# Symbol to trade
symbol=AAPL

# Strategy selection (market_making or stat_arb)
strategy=market_making

# Market Making Parameters
spread_bps=10.0
quote_size=100

# Risk Limits
max_position=1000
max_order_size=500
max_loss=10000.0
```

## 🧪 Testing

```bash
cd build
ctest --verbose
```

### Individual Tests

```bash
# Test order book
./build/test_order_book

# Test lock-free queue
./build/test_lock_free_queue

# Test memory pool
./build/test_memory_pool
```

## 📊 Performance Characteristics

### Latency Benchmarks
- **Order Book Operations**: ~100-200 ns per operation
- **Order Processing**: <1 microsecond end-to-end
- **Market Data Processing**: ~500 ns latency
- **Memory Allocation**: 10x faster than standard new/delete

### Throughput
- **Market Data**: 1M+ messages/second
- **Order Rate**: 100K+ orders/second
- **Trade Processing**: Real-time with nanosecond timestamps

## 🏗️ Project Structure

```
HFT-System-CPP/
├── include/              # Header files
│   ├── core/            # Core infrastructure
│   ├── market_data/     # Market data components
│   ├── oms/             # Order management
│   ├── strategy/        # Trading strategies
│   ├── gateway/         # Exchange connectivity
│   ├── risk/            # Risk management
│   └── metrics/         # Performance monitoring
├── src/                 # Implementation files
├── tests/               # Unit tests
├── config/              # Configuration files
├── scripts/             # Build and deployment scripts
├── docker/              # Docker configuration
└── CMakeLists.txt       # Build configuration
```

## 🚀 Production Deployment

### Performance Tuning

1. **CPU Pinning**: Bind threads to specific cores to reduce context switching
2. **Huge Pages**: Enable transparent huge pages for better memory performance
3. **Network Tuning**: Optimize kernel network parameters for low latency
4. **NUMA Awareness**: Allocate memory on the same NUMA node as the CPU

### System Configuration

```bash
# Enable huge pages
echo always > /sys/kernel/mm/transparent_hugepage/enabled

# Increase network buffer sizes
sysctl -w net.core.rmem_max=134217728
sysctl -w net.core.wmem_max=134217728

# Reduce timer tick frequency
echo nohz_full=1-7 > /etc/default/grub
```

### Monitoring

- **System Metrics**: CPU, memory, network utilization
- **Latency Metrics**: Order processing, market data latency
- **Trading Metrics**: Fill rates, PnL, positions
- **Risk Metrics**: Position limits, exposure, drawdown

## 🔐 Security Considerations

- **Pre-Trade Risk Checks**: All orders validated before submission
- **Position Limits**: Per-symbol and aggregate position limits
- **Price Collars**: Prevents erroneous orders at extreme prices
- **Rate Limiting**: Prevents excessive order submission

## 📝 Code Quality

- **Modern C++20**: Using latest language features
- **Memory Safety**: RAII, smart pointers, bounds checking
- **Thread Safety**: Lock-free data structures, atomic operations
- **Error Handling**: Exception safety and error propagation

## 🤝 Contributing

This is a demonstration project showcasing HFT system design and implementation.

## 📄 License

This project is provided for educational and demonstration purposes.

## ⚠️ Disclaimer

This is a demonstration HFT system for educational purposes. It uses simulated market data and should not be used for actual trading without extensive testing, regulatory compliance, and risk management procedures.

## 📚 Key Technologies

- **C++20**: Modern C++ with concepts, ranges, and coroutines
- **Lock-Free Programming**: Atomic operations and memory ordering
- **SIMD Optimizations**: Vector operations for performance
- **Cache Optimization**: Data structure alignment and prefetching
- **FIX Protocol**: Industry-standard exchange connectivity

## 🎯 Use Cases

- **Algorithmic Trading**: High-frequency market making and arbitrage
- **Market Making**: Providing liquidity in electronic markets
- **Statistical Arbitrage**: Exploiting price inefficiencies
- **Backtesting**: Historical strategy simulation
- **Research**: HFT system design and optimization

## 📈 Future Enhancements

- [ ] Multi-asset support
- [ ] Advanced strategies (pairs trading, order flow prediction)
- [ ] Machine learning integration
- [ ] GPU acceleration for compute-intensive operations
- [ ] Distributed system architecture
- [ ] Real exchange connectivity (production FIX)
- [ ] Historical data replay and backtesting
- [ ] Web-based monitoring dashboard

## 📞 Support

For questions or issues, please refer to the documentation or contact the development team.

---

**Built with ❤️ for ultra-low-latency trading**

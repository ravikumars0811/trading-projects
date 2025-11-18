# System Architecture & Detailed Functionality Guide

## Table of Contents
1. [System Overview](#system-overview)
2. [Component Details](#component-details)
3. [Data Flow](#data-flow)
4. [Performance Optimizations](#performance-optimizations)
5. [Deployment Architecture](#deployment-architecture)

## System Overview

The Algorithmic Trading System is designed as a modular, high-performance platform capable of handling High-Frequency Trading (HFT) workloads while integrating advanced AI/ML capabilities.

### Design Principles

1. **Low Latency**: Every component is optimized for minimal latency
2. **High Throughput**: Capable of processing millions of events per second
3. **Fault Tolerance**: Graceful degradation and automatic recovery
4. **Scalability**: Horizontal and vertical scaling capabilities
5. **Modularity**: Pluggable components for easy customization

## Component Details

### 1. Market Data Handler

#### Functionality
The Market Data Handler is responsible for receiving, processing, and distributing real-time market data to the rest of the system.

#### Key Features

**Lock-Free Ring Buffer**
- Custom implementation using atomic operations
- Size: 100,000 elements (configurable)
- Zero-copy design for maximum performance
- Supports both market data and tick data

```cpp
template<typename T, size_t Size>
class LockFreeRingBuffer {
    // Uses atomic operations for thread-safe access
    // No mutex locks in the hot path
    // Circular buffer with separate read/write pointers
};
```

**WebSocket Client**
- Asynchronous I/O for non-blocking communication
- Automatic reconnection with exponential backoff
- Heartbeat mechanism to detect connection issues
- Message batching for efficiency

**Data Processing**
- Multi-threaded processing (one thread per CPU core)
- Processes both Level 1 (quotes) and Level 2 (order book) data
- Real-time statistics calculation (updates/sec, latency)
- Callback system for downstream components

**Supported Data Providers**
1. **Alpaca**: US stocks and crypto
2. **Interactive Brokers**: Global markets
3. **Polygon**: Real-time and historical data
4. **Custom feeds**: Easy to add new providers

#### Performance Metrics
- **Throughput**: 1M+ updates/second
- **Latency**: < 10 microseconds (from network to callback)
- **Memory**: Constant memory usage (no dynamic allocation in hot path)

---

### 2. Order Management System (OMS)

#### Functionality
The OMS manages the complete order lifecycle from creation to execution, maintaining portfolio state and ensuring data consistency.

#### Key Features

**Order Types**
1. **Market Orders**: Execute immediately at best available price
2. **Limit Orders**: Execute only at specified price or better
3. **Stop Orders**: Trigger when price reaches stop level
4. **Stop-Limit Orders**: Combine stop and limit functionality
5. **Iceberg Orders**: Hide large order quantity
6. **TWAP**: Time-Weighted Average Price execution
7. **VWAP**: Volume-Weighted Average Price execution

**Order State Machine**
```
PENDING → SUBMITTED → (PARTIALLY_FILLED)* → FILLED
                  ↓
              CANCELLED / REJECTED / EXPIRED
```

**Portfolio Management**
- Real-time position tracking
- Automatic P&L calculation (realized and unrealized)
- Multi-symbol support
- Long and short positions
- Average price calculation using FIFO

**Position Tracking**
```cpp
struct Position {
    Symbol symbol;
    Quantity quantity;      // +ve for long, -ve for short
    Price avg_price;        // Average entry price
    double realized_pnl;    // Closed trades P&L
    double unrealized_pnl;  // Open position P&L
};
```

#### Thread Safety
- Reader-writer locks for concurrent access
- Lock-free order ID generation
- Atomic counters for statistics
- Thread-safe callbacks

---

### 3. Strategy Engine

#### Functionality
The Strategy Engine generates trading signals based on various algorithms and executes them through the Order Manager.

#### Built-in Strategies

**1. Moving Average Crossover**
- **Logic**: Buy when short MA crosses above long MA, sell when opposite
- **Parameters**: Short period (default: 20), Long period (default: 50)
- **Use Case**: Trend-following in trending markets
- **Performance**: Best in strongly trending markets

**2. Mean Reversion**
- **Logic**: Buy when price deviates below mean, sell when above
- **Parameters**: Lookback period, Entry threshold (Z-score), Exit threshold
- **Use Case**: Range-bound markets
- **Performance**: Best in sideways markets with clear support/resistance

**3. Momentum**
- **Logic**: Buy strong performers, sell weak performers
- **Parameters**: Lookback period for momentum calculation
- **Use Case**: Capturing short-term price momentum
- **Performance**: Best in volatile markets

**4. ML Strategy (LSTM/Transformer)**
- **Logic**: Deep learning models predict price direction
- **Features**: 50+ technical indicators
- **Model Types**: LSTM, Transformer, GRU
- **Performance**: Adapts to changing market conditions

#### Signal Generation
```cpp
struct Signal {
    Symbol symbol;
    double strength;     // -1.0 (strong sell) to 1.0 (strong buy)
    double confidence;   // 0.0 to 1.0
    Timestamp timestamp;
    std::string reason;  // Human-readable explanation
};
```

#### Strategy Manager
- Manages multiple strategies simultaneously
- Aggregates signals from different strategies
- Position sizing based on signal strength and confidence
- Performance tracking per strategy

---

### 4. Risk Management System

#### Functionality
The Risk Manager ensures that all trading activities comply with predefined risk limits, protecting capital and preventing catastrophic losses.

#### Pre-Trade Checks

**Position Limits**
- Maximum position size per symbol
- Maximum concentration (% of portfolio)
- Prevents over-exposure to single instrument

**Portfolio Limits**
- Maximum total portfolio value
- Maximum leverage ratio
- Cash requirements

**Loss Limits**
- Maximum daily loss threshold
- Maximum drawdown from peak
- Triggers circuit breaker when breached

**Rate Limiting**
- Maximum orders per second
- Prevents fat-finger errors
- Protects against system bugs

#### Post-Trade Monitoring

**Real-Time Risk Metrics**
```cpp
struct RiskMetrics {
    double current_drawdown;     // Current drawdown from peak
    double daily_pnl;            // Today's P&L
    double peak_equity;          // All-time high equity
    double current_leverage;     // Current leverage ratio
    double portfolio_var;        // Value at Risk
    double sharpe_ratio;         // Risk-adjusted returns
    double max_concentration;    // Largest position %
};
```

**Value at Risk (VaR)**
- Historical VaR calculation
- Parametric VaR (assuming normal distribution)
- Monte Carlo VaR (for complex portfolios)
- Confidence levels: 95%, 99%

**Conditional VaR (CVaR)**
- Expected shortfall beyond VaR
- More conservative risk measure
- Captures tail risk

#### Circuit Breakers

**Automatic Trading Halt When**:
1. Daily loss exceeds limit
2. Drawdown exceeds maximum
3. System error detected
4. Manual trigger by operator

**Actions Taken**:
- Cancel all active orders
- Notify administrators
- Log incident for review
- Prevent new order submission

#### Stop Loss Management

**Types**:
1. **Fixed Stop**: Stop at specific price
2. **Trailing Stop**: Follow price with fixed distance
3. **Percentage Stop**: Stop when loss exceeds %

**Features**:
- Automatic execution
- Per-symbol configuration
- Real-time price monitoring

---

### 5. Execution Engine

#### Simulated Execution (Backtesting)

**Features**:
- Realistic fill simulation
- Configurable slippage (basis points)
- Latency simulation (microseconds)
- Fill probability (0-1)
- Partial fills support

**Order Matching**:
```cpp
// Market orders fill at best price + slippage
// Limit orders fill only when price reaches limit
// Stop orders trigger at stop price, then execute as market
```

#### Live Execution (Alpaca Integration)

**Features**:
- Direct market access
- Order status updates via WebSocket
- Position synchronization
- Account information retrieval

**API Integration**:
- RESTful API for order submission
- WebSocket for real-time updates
- OAuth2 authentication
- Rate limiting compliance

#### Smart Order Router (SOR)

**Routing Strategies**:
1. **Round Robin**: Distribute orders evenly
2. **Lowest Latency**: Route to fastest venue
3. **Best Price**: Route to best bid/ask
4. **Liquidity Seeking**: Route to deepest market

---

### 6. Backtesting Framework

#### Functionality
The Backtesting Framework allows testing strategies on historical data before deploying them with real capital.

#### Features

**Historical Simulation**
- Event-driven architecture
- Bar-by-bar processing
- Realistic order fills
- Commission and slippage modeling

**Performance Metrics**

**Return Metrics**:
- Total return
- Annualized return
- Compound annual growth rate (CAGR)

**Risk-Adjusted Metrics**:
- Sharpe Ratio: (Return - Risk-free rate) / Volatility
- Sortino Ratio: Like Sharpe but only downside volatility
- Calmar Ratio: CAGR / Max Drawdown

**Trading Metrics**:
- Win rate (% winning trades)
- Average win vs average loss
- Profit factor (gross profit / gross loss)
- Maximum consecutive losses

**Drawdown Analysis**:
- Maximum drawdown
- Drawdown duration
- Recovery time

**Walk-Forward Optimization**
- In-sample optimization
- Out-of-sample validation
- Rolling windows
- Prevents overfitting

**Monte Carlo Simulation**
- Test strategy robustness
- Random price perturbations
- Bootstrap resampling
- Risk of ruin calculation

---

### 7. Machine Learning Pipeline

#### Functionality
The ML Pipeline trains deep learning models on historical data and integrates them into the trading system.

#### Architecture

**Data Pipeline**:
1. **Data Collection**: Historical OHLCV data
2. **Feature Engineering**: Calculate technical indicators
3. **Normalization**: Standardize features
4. **Sequence Creation**: Create time-series sequences
5. **Train/Test Split**: Time-based splitting

**Feature Engineering**

**Technical Indicators** (50+ features):
- Price-based: Returns, log returns, momentum
- Moving averages: SMA, EMA (multiple periods)
- Oscillators: RSI, Stochastic, CCI
- Trend: MACD, ADX, Aroon
- Volatility: Bollinger Bands, ATR, Keltner Channels
- Volume: Volume ratios, OBV, MFI

**Model Architectures**

**1. LSTM (Long Short-Term Memory)**
```
Input Layer (features)
    → LSTM Layer 1 (128 units)
    → LSTM Layer 2 (128 units)
    → Dense Layer 1 (64 units)
    → Dense Layer 2 (32 units)
    → Output Layer (1 unit, sigmoid)
```
- Captures long-term dependencies
- Remembers important patterns
- Forgets irrelevant information

**2. Transformer**
```
Input Projection (features → d_model)
    → Transformer Encoder (8 heads, 3 layers)
    → Mean Pooling
    → Dense Layers
    → Output (probability)
```
- Attention mechanism
- Parallel processing
- Better long-range dependencies

**Training Process**

**Hyperparameters**:
- Batch size: 64
- Learning rate: 0.001
- Optimizer: Adam
- Loss function: Binary Cross-Entropy
- Sequence length: 50 timesteps

**Regularization**:
- Dropout (0.2)
- Early stopping (patience: 10)
- Learning rate scheduling
- Gradient clipping

**Validation**:
- Time-series cross-validation
- Out-of-sample testing
- Walk-forward validation

**Model Deployment**:
1. Train in Python (PyTorch/TensorFlow)
2. Export to ONNX or TorchScript
3. Load in C++ for inference
4. Real-time prediction (< 1ms)

---

### 8. Database Layer

#### TimescaleDB (Time-Series Data)

**Schema**:
```sql
-- Market data table
CREATE TABLE market_data (
    time TIMESTAMPTZ NOT NULL,
    symbol TEXT NOT NULL,
    open DOUBLE PRECISION,
    high DOUBLE PRECISION,
    low DOUBLE PRECISION,
    close DOUBLE PRECISION,
    volume BIGINT
);

-- Create hypertable for automatic partitioning
SELECT create_hypertable('market_data', 'time');

-- Orders table
CREATE TABLE orders (
    order_id BIGINT PRIMARY KEY,
    symbol TEXT NOT NULL,
    side TEXT NOT NULL,
    type TEXT NOT NULL,
    quantity DOUBLE PRECISION,
    price DOUBLE PRECISION,
    status TEXT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL,
    updated_at TIMESTAMPTZ NOT NULL
);

-- Trades table
CREATE TABLE trades (
    trade_id BIGINT PRIMARY KEY,
    order_id BIGINT REFERENCES orders(order_id),
    symbol TEXT NOT NULL,
    side TEXT NOT NULL,
    quantity DOUBLE PRECISION,
    price DOUBLE PRECISION,
    commission DOUBLE PRECISION,
    timestamp TIMESTAMPTZ NOT NULL
);

-- Positions table
CREATE TABLE positions (
    symbol TEXT PRIMARY KEY,
    quantity DOUBLE PRECISION,
    avg_price DOUBLE PRECISION,
    realized_pnl DOUBLE PRECISION,
    unrealized_pnl DOUBLE PRECISION,
    last_update TIMESTAMPTZ NOT NULL
);
```

**Continuous Aggregates**:
```sql
-- 1-minute OHLCV aggregates
CREATE MATERIALIZED VIEW ohlcv_1min
WITH (timescaledb.continuous) AS
SELECT
    time_bucket('1 minute', time) AS bucket,
    symbol,
    first(close, time) AS open,
    max(close) AS high,
    min(close) AS low,
    last(close, time) AS close,
    sum(volume) AS volume
FROM market_data
GROUP BY bucket, symbol;
```

#### Redis (Caching)

**Use Cases**:
1. **Real-time prices**: Latest market data
2. **Active orders**: Fast lookup
3. **Position cache**: Reduce database queries
4. **Session data**: User sessions

**Data Structures**:
```
HSET market:AAPL bid 150.10 ask 150.15 last 150.12
ZADD active_orders 1234567890 order:12345
HSET position:AAPL quantity 100 avg_price 148.50
```

---

### 9. Monitoring & Observability

#### Prometheus Metrics

**Trading Metrics**:
- `trading_orders_total`: Total orders submitted
- `trading_orders_filled`: Total orders filled
- `trading_orders_rejected`: Total orders rejected
- `trading_pnl_total`: Total P&L
- `trading_positions_count`: Number of open positions
- `trading_equity_total`: Current equity

**System Metrics**:
- `system_cpu_usage`: CPU utilization
- `system_memory_usage`: Memory usage
- `system_latency_seconds`: Processing latency
- `market_data_updates_per_second`: Market data rate
- `order_processing_latency_seconds`: Order latency

**Risk Metrics**:
- `risk_drawdown_current`: Current drawdown
- `risk_leverage_current`: Current leverage
- `risk_var_current`: Current VaR
- `risk_violations_total`: Risk violations count

#### Grafana Dashboards

**Dashboard 1: Trading Overview**
- Equity curve
- Daily P&L
- Open positions
- Recent trades

**Dashboard 2: Performance**
- Sharpe ratio
- Win rate
- Profit factor
- Max drawdown

**Dashboard 3: System Health**
- CPU/Memory usage
- Network latency
- Database queries/sec
- Error rates

**Dashboard 4: Risk**
- Current drawdown
- Leverage ratio
- Position concentration
- VaR/CVaR

---

## Data Flow

### Live Trading Flow

```
1. Market Data Arrives
   ↓
2. WebSocket Client receives data
   ↓
3. Lock-free buffer enqueues data
   ↓
4. Worker threads process data
   ↓
5. Market Data Handler updates internal state
   ↓
6. Callbacks notify Strategy Engine
   ↓
7. Strategy Engine generates signals
   ↓
8. Risk Manager validates signal
   ↓
9. Order Manager creates order
   ↓
10. Execution Engine submits order
    ↓
11. Order status updates received
    ↓
12. Portfolio updated
    ↓
13. Metrics exported to Prometheus
    ↓
14. Dashboard updated in real-time
```

### Backtesting Flow

```
1. Load historical data from database
   ↓
2. Initialize portfolio with initial capital
   ↓
3. For each time bar:
   a. Update market data
   b. Strategy generates signals
   c. Risk checks signals
   d. Orders executed with simulated fills
   e. Portfolio updated
   f. Metrics calculated
   ↓
4. Generate final report
   ↓
5. Save results and equity curve
```

---

## Performance Optimizations

### 1. Lock-Free Data Structures
- Ring buffers for market data
- Atomic operations instead of mutexes
- Single-producer-single-consumer queues

### 2. Memory Management
- Pre-allocated buffers
- Object pools for orders/trades
- Arena allocators for temporary objects
- Avoid dynamic allocation in hot paths

### 3. CPU Optimization
- Cache-friendly data layout
- Branch prediction hints
- SIMD instructions for calculations
- Thread affinity for consistent latency

### 4. Network Optimization
- Keep-alive connections
- Message batching
- Compression for non-critical data
- Multiple connections for redundancy

### 5. Compiler Optimizations
```cmake
-O3                  # Maximum optimization
-march=native        # CPU-specific instructions
-flto               # Link-time optimization
-ffast-math         # Faster math (careful!)
```

---

## Deployment Architecture

### Single Server Deployment

```
┌─────────────────────────────────────┐
│     Trading Server (High-End)       │
├─────────────────────────────────────┤
│  - Trading System Process           │
│  - TimescaleDB                      │
│  - Redis                            │
│  - Prometheus                       │
│  - Grafana                          │
└─────────────────────────────────────┘
```

### High-Availability Deployment

```
┌──────────────┐      ┌──────────────┐
│   Primary    │◄────►│   Secondary  │
│   Trading    │      │   Trading    │
│   Server     │      │   Server     │
└──────────────┘      └──────────────┘
       │                     │
       └──────────┬──────────┘
                  │
        ┌─────────▼─────────┐
        │  TimescaleDB      │
        │  (Replication)    │
        └───────────────────┘
```

### Cloud Deployment (AWS)

```
┌─────────────────────────────────────────┐
│               AWS Cloud                 │
├─────────────────────────────────────────┤
│  ECS/EKS Cluster                        │
│  ├─ Trading System Containers           │
│  ├─ Load Balancer                       │
│  └─ Auto Scaling                        │
│                                          │
│  RDS PostgreSQL (TimescaleDB)           │
│  ElastiCache (Redis)                    │
│  CloudWatch (Monitoring)                │
│  S3 (Data Storage)                      │
└─────────────────────────────────────────┘
```

---

## Conclusion

This trading system represents a production-grade implementation combining:
- **High performance**: HFT-capable latencies
- **Advanced AI/ML**: Deep learning integration
- **Robust risk management**: Comprehensive protection
- **Enterprise features**: Monitoring, logging, deployment

The modular architecture allows easy customization and extension while maintaining performance and reliability standards required for live trading.

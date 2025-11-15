# High-Frequency Trading System Design

## System Overview

Design a complete HFT system capable of processing millions of orders per second with sub-microsecond latency.

### Requirements

**Functional Requirements:**
- Receive market data from multiple exchanges
- Process orders (limit, market, stop)
- Match orders in real-time
- Execute trades with exchanges
- Manage positions and P&L
- Risk management and compliance
- Historical data storage and analysis

**Non-Functional Requirements:**
- Latency: < 10 microseconds order processing
- Throughput: 10M+ messages/second
- Availability: 99.99% uptime
- Data integrity: No trade duplicates or losses
- Scalability: Support 1000+ trading symbols
- Monitoring: Real-time metrics and alerts

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         HFT TRADING SYSTEM                       │
└─────────────────────────────────────────────────────────────────┘

┌──────────────────┐         ┌──────────────────┐
│   EXCHANGES      │         │   MARKET DATA    │
│   - NYSE         │────────▶│   FEED HANDLER   │
│   - NASDAQ       │         │   (UDP Multicast) │
│   - CBOE         │         └────────┬─────────┘
└──────────────────┘                  │
                                      │
                         ┌────────────▼──────────────┐
                         │   MARKET DATA NORMALIZER   │
                         │   - Protocol translation   │
                         │   - Data validation        │
                         └────────────┬───────────────┘
                                      │
                    ┌─────────────────┼─────────────────┐
                    │                 │                 │
        ┌───────────▼────────┐  ┌────▼─────────┐  ┌───▼──────────┐
        │   STRATEGY ENGINE   │  │  ORDER BOOK  │  │ RISK ENGINE  │
        │   - Alpha signals   │  │  - Price     │  │ - Position   │
        │   - Market making   │  │    levels    │  │   limits     │
        │   - Arbitrage       │  │  - Matching  │  │ - Capital    │
        └─────────┬───────────┘  └──────────────┘  │   checks     │
                  │                                 └───┬──────────┘
                  │                                     │
                  └──────────────┬──────────────────────┘
                                 │
                         ┌───────▼────────────┐
                         │  ORDER MANAGEMENT  │
                         │  SYSTEM (OMS)      │
                         │  - Order routing   │
                         │  - State tracking  │
                         └──────────┬─────────┘
                                    │
                         ┌──────────▼─────────┐
                         │  EXECUTION ENGINE  │
                         │  - Smart routing   │
                         │  - FIX protocol    │
                         └──────────┬─────────┘
                                    │
                         ┌──────────▼─────────┐
                         │    EXCHANGES       │
                         └────────────────────┘

        ┌────────────────────────────────────────────┐
        │         SUPPORTING SERVICES                │
        ├────────────────────────────────────────────┤
        │  - Position Keeper                         │
        │  - P&L Calculator                          │
        │  - Trade Repository                        │
        │  - Monitoring & Alerting                   │
        │  - Configuration Management                │
        └────────────────────────────────────────────┘
```

## Component Design

### 1. Market Data Feed Handler

**Purpose:** Receive and normalize market data from exchanges

**Key Features:**
- UDP multicast for low latency
- Lock-free processing
- Sequence number gap detection
- A/B feed handling for reliability

```cpp
class MarketDataFeedHandler {
private:
    // Lock-free SPSC queue per symbol
    std::unordered_map<std::string, SPSCQueue<MarketData>> queues_;

    // Separate thread per feed
    std::thread receiver_thread_;
    std::thread processor_thread_;

    // UDP socket
    int socket_fd_;

    // Statistics
    std::atomic<uint64_t> messages_received_{0};
    std::atomic<uint64_t> gaps_detected_{0};

public:
    void start() {
        // 1. Bind to multicast group
        // 2. Set socket options (SO_RCVBUF, SO_TIMESTAMP)
        // 3. Pin thread to CPU core
        // 4. Start receiver thread
    }

    void process() {
        // 1. Read from socket (zero-copy if possible)
        // 2. Parse message (fixed-size structs)
        // 3. Check sequence number
        // 4. Enqueue to symbol-specific queue
        // 5. Notify subscribers
    }
};
```

**Optimizations:**
- Pin threads to specific CPU cores
- Use huge pages for memory
- SO_RCVBUF sized for burst handling
- Hardware timestamping
- Kernel bypass (DPDK)

**Latency:** 200-500 nanoseconds

### 2. Strategy Engine

**Purpose:** Generate trading signals based on market data

**Architecture:**

```
┌────────────────────────────────────────────┐
│          Strategy Engine                   │
├────────────────────────────────────────────┤
│                                            │
│  ┌──────────────────────────────────────┐ │
│  │  Strategy Manager                    │ │
│  │  - Load strategies dynamically       │ │
│  │  - Distribute market data            │ │
│  └──────────────┬───────────────────────┘ │
│                 │                          │
│    ┌────────────┼────────────┐            │
│    │            │            │            │
│  ┌─▼──────┐  ┌─▼──────┐  ┌─▼──────┐      │
│  │ MM     │  │ Arb    │  │ Trend  │      │
│  │ Strategy│  │ Strategy│  │ Strategy│      │
│  └────────┘  └────────┘  └────────┘      │
│                                            │
│  ┌──────────────────────────────────────┐ │
│  │  Signal Aggregator                   │ │
│  │  - Combine signals                   │ │
│  │  - Portfolio allocation              │ │
│  └──────────────┬───────────────────────┘ │
└─────────────────┼──────────────────────────┘
                  │
                  ▼
         ┌────────────────┐
         │  Order Generator│
         └────────────────┘
```

**Key Strategies:**

1. **Market Making:**
   ```cpp
   class MarketMakingStrategy {
       void onMarketData(const MarketData& md) {
           double mid = (md.bid + md.ask) / 2.0;
           double spread = config_.target_spread;

           // Quote both sides
           quotePrice(mid - spread/2, md.bid_size);
           quotePrice(mid + spread/2, md.ask_size);
       }
   };
   ```

2. **Statistical Arbitrage:**
   ```cpp
   class StatArbStrategy {
       void onMarketData(const MarketData& md) {
           double spread = calculateSpread(stock_a, stock_b);
           if (spread > mean_ + 2*stddev_) {
               // Short spread
               sell(stock_a);
               buy(stock_b);
           }
       }
   };
   ```

**Latency:** 1-5 microseconds

### 3. Order Book Implementation

**Purpose:** Maintain real-time view of market liquidity

**Data Structure:**

```cpp
class OrderBook {
private:
    // Price levels using sorted containers
    std::map<double, PriceLevel, std::greater<>> bids_;  // Descending
    std::map<double, PriceLevel, std::less<>> asks_;     // Ascending

    // Fast order lookup
    std::unordered_map<OrderId, Order*> orders_;

    // Cache best prices
    double best_bid_{0};
    double best_ask_{0};

    // Memory pool for orders
    ObjectPool<Order> order_pool_;

public:
    // O(1) - cached
    double getBestBid() const { return best_bid_; }
    double getBestAsk() const { return best_ask_; }

    // O(log n) - map insertion
    void addOrder(Order* order);

    // O(log n) - map deletion
    void cancelOrder(OrderId id);

    // O(k log n) - k matches
    void matchOrder(Order* aggressor);
};
```

**Memory Layout:**
```
Cache Line 0:  [best_bid | best_ask | total_bid_volume | total_ask_volume]
Cache Line 1:  [top_5_bid_prices...]
Cache Line 2:  [top_5_ask_prices...]
Cache Line 3:  [order_count | last_trade_price | ...]
```

**Optimizations:**
- Intrusive containers (no allocations)
- Memory pool for orders
- Cache line alignment
- Prefetch next price level

**Latency:** 100-300 nanoseconds per operation

### 4. Risk Management System

**Purpose:** Enforce trading limits and compliance

**Risk Checks (in order):**

```cpp
class RiskEngine {
public:
    bool checkOrder(const Order& order) {
        // 1. Pre-trade checks (FAST - inline)
        if (!checkPositionLimit(order)) return false;
        if (!checkCapitalLimit(order)) return false;
        if (!checkPriceCollar(order)) return false;

        // 2. Symbol-level checks
        if (!checkMaxOrderSize(order)) return false;
        if (!checkMaxOrderRate(order)) return false;

        // 3. Account-level checks
        if (!checkDailyLossLimit()) return false;
        if (!checkNetExposure()) return false;

        return true;
    }

private:
    // Limits stored in cache-friendly arrays
    struct alignas(64) SymbolLimits {
        double max_position;
        double max_order_size;
        double price_min;
        double price_max;
    };

    std::unordered_map<Symbol, SymbolLimits> limits_;
};
```

**Kill Switch:**
```cpp
class KillSwitch {
    std::atomic<bool> enabled_{false};

    void activate(const std::string& reason) {
        enabled_.store(true);
        // 1. Stop all new orders
        // 2. Cancel all open orders
        // 3. Flatten all positions (optional)
        // 4. Alert operations team
    }
};
```

**Latency:** 50-200 nanoseconds

### 5. Order Management System (OMS)

**Purpose:** Route and track order lifecycle

**State Machine:**

```
    NEW ──────▶ PENDING ──────▶ ACKNOWLEDGED ──────▶ FILLED
     │            │                  │                  │
     │            │                  │                  │
     └────────────┴──────────────────┴──────▶ CANCELLED │
                                                        │
                                                        ▼
                                                    ARCHIVED
```

**Order Router:**

```cpp
class OrderRouter {
public:
    void routeOrder(Order* order) {
        // 1. Select venue
        Venue* venue = selectVenue(order);

        // 2. Risk check
        if (!risk_engine_->check(order)) {
            rejectOrder(order, "Risk check failed");
            return;
        }

        // 3. Convert to venue protocol
        VenueMessage msg = convertToVenueFormat(order);

        // 4. Send (via FIX or binary protocol)
        venue->send(msg);

        // 5. Update state
        order->state = OrderState::PENDING;

        // 6. Track in OMS
        pending_orders_[order->id] = order;
    }

private:
    Venue* selectVenue(Order* order) {
        // Smart order routing logic
        // - Best price
        // - Lowest fees
        // - Historical fill rate
        // - Current queue position
    }
};
```

### 6. Execution Engine

**Purpose:** Communicate with exchanges via FIX or binary protocols

**FIX Session Management:**

```cpp
class FIXSession {
private:
    // Outbound sequence number
    std::atomic<int> out_seq_num_{1};

    // Inbound sequence number
    std::atomic<int> in_seq_num_{1};

    // Socket
    int socket_fd_;

    // Message queue
    SPSCQueue<FIXMessage> outbound_queue_;

public:
    void sendOrder(const Order& order) {
        FIXMessage msg;
        msg.setMsgType("D");  // NewOrderSingle
        msg.setField(11, order.client_order_id);
        msg.setField(55, order.symbol);
        msg.setField(54, order.side);
        msg.setField(38, order.quantity);
        msg.setField(40, order.type);
        msg.setField(44, order.price);

        // Add header
        msg.setSeqNum(out_seq_num_++);
        msg.calculateChecksum();

        // Send
        send(socket_fd_, msg.toString());
    }
};
```

**Binary Protocol (Proprietary):**

```cpp
struct __attribute__((packed)) NewOrderMsg {
    uint8_t  msg_type;        // 'N'
    uint64_t order_id;
    char     symbol[8];
    uint8_t  side;            // 'B' or 'S'
    uint32_t quantity;
    uint64_t price;           // Fixed-point: price * 10000
    uint8_t  order_type;      // 'L', 'M', etc.
    uint64_t timestamp;
};
```

## Data Flow

### Market Data Path (Critical Path)

```
1. Network card receives packet
2. Kernel processes packet (or bypass via DPDK)
3. Feed handler reads from socket
4. Parse message (200-500ns)
5. Update order book (100-300ns)
6. Strategy processes (1-5μs)
7. Generate order (100ns)
8. Risk check (50-200ns)
9. Send to exchange (1-10μs network)

Total: 2-20 microseconds
```

### Order Execution Path

```
1. Strategy generates signal
2. Create order object (from pool)
3. Risk checks (pre-trade)
4. Queue order for routing
5. Convert to FIX/binary
6. Send via socket
7. Wait for acknowledgment
8. Update order state
9. Execute trade
10. Update positions and P&L

Total: 10-100 microseconds
```

## Performance Optimizations

### 1. CPU Optimizations

**Thread Affinity:**
```cpp
void pinThreadToCPU(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
}
```

**CPU Layout:**
- Core 0: Market data receiver
- Core 1: Market data processor
- Core 2: Strategy engine
- Core 3: Order routing
- Core 4-7: Risk checks, P&L, monitoring

### 2. Memory Optimizations

**Huge Pages:**
```bash
# Allocate 1GB huge pages
echo 512 > /proc/sys/vm/nr_hugepages

# In code:
void* ptr = mmap(nullptr, size,
                 PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                 -1, 0);
```

**Memory Pool:**
```cpp
template<typename T, size_t Size>
class MemoryPool {
    alignas(64) T objects_[Size];
    std::atomic<size_t> next_free_{0};

public:
    T* allocate() {
        size_t idx = next_free_.fetch_add(1);
        return idx < Size ? &objects_[idx] : nullptr;
    }
};
```

### 3. Network Optimizations

**Kernel Bypass (DPDK):**
- Direct access to NIC
- Zero-copy packet processing
- Polling instead of interrupts
- Latency: ~200ns vs ~2μs with kernel

**TCP/IP Optimization:**
```cpp
// Disable Nagle's algorithm
int flag = 1;
setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

// Increase socket buffer
int bufsize = 1024 * 1024;
setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

// Enable quickack
int quickack = 1;
setsockopt(sock, IPPROTO_TCP, TCP_QUICKACK, &quickack, sizeof(quickack));
```

## Monitoring and Observability

### Key Metrics

**Latency Metrics:**
- Market data latency (feed to strategy)
- Order placement latency (signal to wire)
- Round-trip latency (send to ack)
- Percentiles: p50, p90, p99, p99.9, p99.99

**Throughput Metrics:**
- Messages per second
- Orders per second
- Fills per second
- Data rate (MB/s)

**Business Metrics:**
- P&L (realized/unrealized)
- Position exposure
- Fill rate
- Slippage
- Win rate

### Monitoring System

```cpp
class LatencyMonitor {
private:
    struct Histogram {
        std::array<std::atomic<uint64_t>, 1000> buckets;
    };

    Histogram market_data_latency_;
    Histogram order_latency_;

public:
    void recordLatency(LatencyType type, uint64_t nanos) {
        // Lock-free histogram update
        size_t bucket = nanos / 100;  // 100ns buckets
        if (bucket < 1000) {
            auto& hist = (type == MARKET_DATA) ?
                market_data_latency_ : order_latency_;
            hist.buckets[bucket].fetch_add(1, std::memory_order_relaxed);
        }
    }

    void dumpStats() {
        // Calculate percentiles
    }
};
```

## Disaster Recovery

### Components

1. **Primary-Backup Failover:**
   - Hot standby receives same market data
   - Shared state via replicated memory
   - Sub-second failover time

2. **Data Replication:**
   - All trades logged to persistent storage
   - Replicate to backup datacenter
   - Event sourcing for state reconstruction

3. **Circuit Breakers:**
   - Automatic halt on abnormal conditions
   - Manual kill switch
   - Gradual resume with monitoring

## Capacity Planning

### Calculations

**Message Rate:**
- Market data: 1M msg/sec × 1000 symbols = 1B msg/sec
- Orders: 100K orders/sec
- Fills: 50K fills/sec

**Bandwidth:**
- Market data: 1M msg/sec × 100 bytes = 100 MB/sec
- Orders: 100K × 200 bytes = 20 MB/sec
- Total: ~150 MB/sec

**CPU:**
- Market data: 2 cores
- Strategies: 4 cores
- Risk/OMS: 2 cores
- Total: 8+ cores per server

**Memory:**
- Order book: 1000 symbols × 10MB = 10GB
- Orders cache: 1M orders × 1KB = 1GB
- Buffers: 5GB
- Total: 20GB+

## Interview Discussion Points

1. **Scalability:**
   - How to scale to 10,000 symbols?
   - How to handle 10x message rate?
   - Geographic distribution?

2. **Trade-offs:**
   - Latency vs throughput
   - Consistency vs availability
   - Cost vs performance

3. **Failure Scenarios:**
   - Network partition
   - Exchange outage
   - Data corruption
   - Infinite loop in strategy

4. **Optimizations:**
   - Where to optimize first?
   - How to measure improvement?
   - When is it fast enough?

5. **Future Improvements:**
   - FPGA-based order matching
   - GPU for parallel calculations
   - Machine learning for signals
   - Blockchain for settlement

## Key Takeaways

1. **Latency is Critical:** Every microsecond matters
2. **Lock-Free is Key:** Avoid contention in hot paths
3. **Cache is King:** Design for L1/L2 cache
4. **Measure Everything:** Can't optimize what you don't measure
5. **Fail Gracefully:** Have plan for every failure mode
6. **Keep It Simple:** Complexity kills performance
7. **Test Thoroughly:** Edge cases cause production issues
8. **Monitor Constantly:** Know system state at all times

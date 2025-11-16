# Module 7: HFT-Specific Topics

## Overview

This module focuses on techniques and implementations specific to High-Frequency Trading (HFT) and low-latency systems used in investment banking.

## Topics Covered

### 1. Low-Latency Fundamentals
- Understanding latency (network, processing, queuing)
- Latency budgets
- Jitter minimization
- Deterministic execution

### 2. Order Book Implementation
- Price-time priority
- Limit order book structure
- Fast order insertion/deletion/matching
- Book depth and aggregation

### 3. Market Data Processing
- Tick data handling
- Level 1 vs Level 2 data
- Book reconstruction
- Snapshot + incremental updates

### 4. FIX Protocol
- FIX message structure
- Parsing and serialization
- Tag-value pairs
- Session management

### 5. Lockless Data Structures
- Single-producer single-consumer (SPSC) queue
- Multi-producer single-consumer (MPSC) queue
- Ring buffers
- Sequence numbers

### 6. Time-Critical Operations
- RDTSC for timestamps
- Clock synchronization
- Avoiding system calls
- Busy-waiting vs sleeping

### 7. Kernel Bypass Techniques
- DPDK concepts
- User-space networking
- Reducing context switches
- Zero-copy I/O

### 8. Optimization Techniques
- Hot path optimization
- Branch prediction
- Inlining critical functions
- Template metaprogramming for zero overhead

## Code Examples

1. `01_order_book.cpp` - High-performance order book
2. `02_lockfree_queue.cpp` - SPSC lock-free queue
3. `03_market_data_feed.cpp` - Fast tick processing
4. `04_fix_parser.cpp` - Minimal FIX protocol parser
5. `05_timestamp_service.cpp` - High-precision timestamps
6. `06_matching_engine.cpp` - Simple matching engine
7. `07_strategy_backtester.cpp` - Performance-focused backtester

## Architecture Patterns

### Feed Handler Pattern
```
Market Data → Parser → Normalizer → Book Builder → Strategies
                                   → Analytics
```

### Order Management System (OMS)
```
Strategy → Risk Check → Order Router → Exchange
             ↓                ↓
          Position       Execution
           Manager       Reports
```

### Matching Engine
```
Incoming Orders → Validation → Order Book → Match
                                   ↓            ↓
                              Cancel/Modify  Execution
```

## Performance Targets

### Latency Budgets (typical)
- Tick-to-trade: 10-100 microseconds
- Order placement: 1-10 microseconds
- Book update: 100-1000 nanoseconds
- Risk check: < 1 microsecond

### Throughput Targets
- Market data: 1M+ messages/second
- Order processing: 100K+ orders/second
- Book updates: 10M+ updates/second

## Key Design Principles

### 1. Zero-Copy Architecture
```cpp
// BAD: Copying data
std::string msg = receiveMessage();
Order order = parseOrder(msg);

// GOOD: In-place processing
const char* msg_ptr = getMessagePtr();
processOrderInPlace(msg_ptr);
```

### 2. Pre-Allocation
```cpp
// BAD: Dynamic allocation in hot path
auto order = new Order(symbol, price, qty);

// GOOD: Memory pool
Order* order = order_pool_.allocate();
order->init(symbol, price, qty);
```

### 3. Minimize Branching
```cpp
// BAD: Unpredictable branch
if (side == 'B') {
    buyBook.insert(order);
} else {
    sellBook.insert(order);
}

// GOOD: Branchless
Book* books[2] = {&sellBook, &buyBook};
books[side == 'B'].insert(order);
```

### 4. Cache Locality
```cpp
// BAD: Pointer chasing
struct Order {
    Order* next;
    OrderData* data;  // Another indirection!
};

// GOOD: Flat structure
struct Order {
    Order* next;
    double price;
    int quantity;
    char symbol[8];
};
```

## Common Pitfalls

1. **Allocations in Hot Path**: Always pre-allocate
2. **Locks in Critical Section**: Use lock-free structures
3. **System Calls**: Cache data, avoid repeated calls
4. **Exceptions**: Avoid exceptions in hot paths
5. **Virtual Functions**: Minimize in latency-critical code
6. **Logging**: Use async logging, rate-limit
7. **STL Containers**: Beware of hidden allocations
8. **Shared Pointers**: Reference counting overhead

## Optimization Techniques

### Compiler Hints
```cpp
// Likely/unlikely for branch prediction
if (__builtin_expect(price > 0, 1)) {  // Likely
    // Hot path
}

// Force inlining
__attribute__((always_inline))
inline void criticalFunction() {
    // ...
}

// No inline
__attribute__((noinline))
void coldFunction() {
    // ...
}
```

### Memory Prefetching
```cpp
// Prefetch next order while processing current
__builtin_prefetch(next_order);
processOrder(current_order);
```

### Cache Line Alignment
```cpp
// Avoid false sharing
struct alignas(64) ThreadLocalData {
    std::atomic<uint64_t> counter;
    char padding[56];
};
```

## Testing and Validation

### Latency Testing
- Measure end-to-end latency
- Profile hot paths
- Test under load
- Measure jitter (99.9th percentile)

### Correctness Testing
- Order book consistency
- Price-time priority
- No order loss
- Proper risk checks

### Stress Testing
- Maximum message rate
- Burst handling
- Memory limits
- CPU saturation

## Real-World Considerations

### Market Microstructure
- Tick sizes
- Lot sizes
- Auction mechanisms
- Circuit breakers

### Risk Management
- Pre-trade risk checks
- Position limits
- Order rate limits
- Fat finger protection

### Regulatory Requirements
- Order audit trail
- Best execution
- Market access controls
- Drop copy

## Resources and References

### Books
- "Trading and Exchanges" by Larry Harris
- "Flash Boys" by Michael Lewis
- "Dark Pools" by Scott Patterson

### Standards
- FIX Protocol specification
- FIX/FAST for market data
- ITCH/OUCH protocols

### Tools
- Market data simulators
- Backtesting frameworks
- Performance profilers
- Network analyzers

## Next Steps

After mastering this module:
1. Implement a complete order book
2. Build a market data feed handler
3. Create a simple matching engine
4. Develop a basic trading strategy
5. Measure and optimize latency
6. Study real exchange protocols
7. Practice with historical data

# Module 8: Practical Projects

## Overview

This module contains complete, working projects that integrate concepts from all previous modules. Each project is designed to simulate real-world trading system components.

## Projects

### 1. Simple Order Book Engine
**Difficulty**: Intermediate
**Topics**: OOP, STL containers, algorithms
**File**: `01_simple_order_book.cpp`

Features:
- Add/cancel/modify orders
- Price-time priority
- Order matching
- Book depth visualization

### 2. Market Data Feed Handler
**Difficulty**: Intermediate-Advanced
**Topics**: Parsing, data structures, performance
**File**: `02_market_data_handler.cpp`

Features:
- Parse tick data
- Maintain OHLCV bars
- Calculate technical indicators
- Handle market data gaps

### 3. Risk Calculator
**Difficulty**: Intermediate
**Topics**: Financial math, optimization
**File**: `03_risk_calculator.cpp`

Features:
- Portfolio VaR calculation
- Position limits checking
- Greeks calculation for options
- Real-time P&L tracking

### 4. Option Pricing Engine
**Difficulty**: Advanced
**Topics**: Mathematical computation, optimization
**File**: `04_option_pricer.cpp`

Features:
- Black-Scholes pricing
- Implied volatility calculation
- Greeks computation
- Monte Carlo simulation

### 5. Backtesting Framework
**Difficulty**: Advanced
**Topics**: System design, performance, data processing
**File**: `05_backtester.cpp`

Features:
- Load historical data
- Simulate order execution
- Calculate performance metrics
- Support multiple strategies

### 6. Lock-Free Message Queue
**Difficulty**: Advanced
**Topics**: Concurrency, lock-free programming
**File**: `06_lockfree_queue.cpp`

Features:
- Single-producer single-consumer queue
- Wait-free operations
- Cache-aligned implementation
- Benchmark suite

### 7. Low-Latency Logger
**Difficulty**: Advanced
**Topics**: System programming, I/O optimization
**File**: `07_async_logger.cpp`

Features:
- Asynchronous logging
- Ring buffer
- Minimal allocation
- Nanosecond timestamps

### 8. Trade Matching Engine
**Difficulty**: Expert
**Topics**: All topics combined
**File**: `08_matching_engine.cpp`

Features:
- Full matching engine
- Multiple order types
- Risk checks
- Performance metrics

## Project Guidelines

### Code Quality
- Follow modern C++ best practices
- Use appropriate STL containers
- Implement proper error handling
- Add comprehensive comments

### Performance
- Measure and optimize hot paths
- Use appropriate data structures
- Minimize allocations
- Profile before optimizing

### Testing
- Write unit tests
- Test edge cases
- Benchmark performance
- Validate financial calculations

## How to Use These Projects

1. **Study**: Read through the code, understand the design
2. **Compile**: Build with optimization flags
3. **Run**: Execute and observe behavior
4. **Modify**: Extend functionality, try improvements
5. **Benchmark**: Measure performance impacts
6. **Interview Prep**: Be able to explain design decisions

## Building Projects

```bash
# Individual project
g++ -std=c++20 -O3 -Wall -Wextra -pthread 01_simple_order_book.cpp -o order_book

# With profiling
g++ -std=c++20 -O3 -g -pg 08_matching_engine.cpp -o matching_engine

# With sanitizers (debugging)
g++ -std=c++20 -O0 -g -fsanitize=address,undefined project.cpp -o project_debug
```

## Extension Ideas

### Beginner Extensions
- Add more order types (stop-loss, iceberg)
- Implement additional technical indicators
- Create visualization of order book
- Add configuration file support

### Intermediate Extensions
- Multi-threaded market data processing
- Implement FIX protocol support
- Add database persistence
- Create REST API for order submission

### Advanced Extensions
- Implement full exchange simulator
- Add machine learning strategy
- Optimize with SIMD operations
- Implement kernel bypass networking

## Learning Path

### Week 1-2: Basic Projects
- Simple Order Book
- Market Data Handler
- Risk Calculator

### Week 3-4: Advanced Projects
- Option Pricing Engine
- Backtesting Framework

### Week 5-6: Expert Projects
- Lock-Free Queue
- Async Logger
- Matching Engine

## Interview Preparation

Common questions about these projects:
1. Explain order book data structure choices
2. How would you optimize the matching engine?
3. Discuss trade-offs in lock-free vs locked designs
4. How to handle market data gaps?
5. Explain backtesting challenges
6. Risk management implementation
7. Latency optimization techniques

## Performance Targets

### Order Book
- Add order: < 100 ns
- Cancel order: < 50 ns
- Match order: < 200 ns

### Market Data
- Process tick: < 500 ns
- Update book: < 1 μs

### Matching Engine
- Throughput: > 1M orders/sec
- Latency: < 10 μs (99th percentile)

## Additional Resources

- Historical market data sources
- Exchange API documentation
- Backtesting best practices
- Performance profiling guides

## Next Steps

After completing these projects:
1. Contribute to open-source trading systems
2. Build your own strategy
3. Study real exchange protocols
4. Learn about market microstructure
5. Explore quantitative research

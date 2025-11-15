# C++ Interview Quick Reference Guide

## Interview Format

### Coding Round (60-90 minutes)
1. **Problem solving** (30-45 min)
   - Data structures and algorithms
   - Trading-specific problems
   - Time/space complexity analysis

2. **Code review** (15-30 min)
   - Identify bugs and issues
   - Suggest optimizations
   - Discuss best practices

3. **Design discussion** (15-30 min)
   - Component design
   - API design
   - Trade-offs discussion

### System Design Round (45-60 minutes)
1. **Requirements gathering** (5-10 min)
2. **High-level architecture** (15-20 min)
3. **Component deep dive** (15-20 min)
4. **Bottlenecks and scaling** (10-15 min)

## Key Topics by Category

### 1. Modern C++ Features (Must Know)

#### Move Semantics
```cpp
// Move constructor
Widget(Widget&& other) noexcept
    : data_(std::move(other.data_)) {
    other.data_ = nullptr;
}

// Perfect forwarding
template<typename T>
void wrapper(T&& arg) {
    func(std::forward<T>(arg));
}
```

**When asked:** "Explain move semantics and when to use it"
- Transfers ownership instead of copying
- Use for large objects, containers
- Critical for performance in HFT
- Always mark noexcept (enables optimizations)

#### Smart Pointers
```cpp
// Unique ownership
std::unique_ptr<Order> order = std::make_unique<Order>();

// Shared ownership
std::shared_ptr<MarketData> data = std::make_shared<MarketData>();

// Weak reference (break cycles)
std::weak_ptr<OrderBook> book_ref = book;
```

**When asked:** "unique_ptr vs shared_ptr"
- unique_ptr: Zero overhead, exclusive ownership
- shared_ptr: Reference counting, atomic ops
- HFT: Prefer unique_ptr for performance

#### Templates and Metaprogramming
```cpp
// SFINAE
template<typename T>
std::enable_if_t<std::is_integral_v<T>, T>
process(T value);

// if constexpr (C++17)
if constexpr (std::is_trivially_copyable_v<T>) {
    memcpy(dest, src, sizeof(T));
}
```

**When asked:** "When to use templates?"
- Generic programming
- Zero-cost abstractions
- Compile-time computation
- Type-safe without runtime cost

### 2. HFT Data Structures

#### Lock-Free Queue
```cpp
// SPSC Queue (fastest)
template<typename T, size_t Size>
class SPSCQueue {
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
    T buffer_[Size];
};
```

**Key points:**
- Latency: 10-20ns (vs 100-500ns with mutex)
- Memory ordering: relaxed for local, acquire/release for sync
- Cache line padding prevents false sharing
- SPSC for 1-to-1, MPMC for many-to-many

#### Order Book
```cpp
class OrderBook {
    std::map<Price, PriceLevel, std::greater<>> bids_;
    std::map<Price, PriceLevel, std::less<>> asks_;
    std::unordered_map<OrderId, Order*> orders_;
};
```

**Operations:**
- Add order: O(log n)
- Cancel: O(log n)
- Best bid/ask: O(1)
- Match: O(k log n)

**Optimizations:**
- Memory pool for orders
- Intrusive containers
- Cache best prices
- Batch updates

### 3. Algorithms

#### Stock Price Problems
```cpp
// Max profit with one transaction
int maxProfit(vector<int>& prices) {
    int min_price = INT_MAX;
    int max_profit = 0;
    for (int price : prices) {
        min_price = min(min_price, price);
        max_profit = max(max_profit, price - min_price);
    }
    return max_profit;
}
```

#### Technical Indicators
```cpp
// SMA
double sma = accumulate(begin, end, 0.0) / size;

// EMA
ema = alpha * price + (1 - alpha) * prev_ema;

// VWAP
vwap = sum(price * volume) / sum(volume);
```

### 4. Design Patterns

#### Strategy Pattern
```cpp
class TradingStrategy {
public:
    virtual void onMarketData(const Data& data) = 0;
    virtual vector<Order> generateSignals() = 0;
};

class MomentumStrategy : public TradingStrategy {
    void onMarketData(const Data& data) override;
};
```

**Use when:** Multiple interchangeable algorithms

#### Observer Pattern
```cpp
class MarketDataFeed {
    vector<Observer*> observers_;
public:
    void notify() {
        for (auto* obs : observers_) {
            obs->update(data_);
        }
    }
};
```

**Use when:** One-to-many dependency, events

#### Object Pool
```cpp
template<typename T>
class ObjectPool {
    queue<unique_ptr<T>> available_;
public:
    unique_ptr<T> acquire();
    void release(unique_ptr<T> obj);
};
```

**Use when:** Frequent allocation/deallocation
**HFT benefit:** Predictable latency, no GC pauses

### 5. Concurrency

#### Thread Pool
```cpp
class ThreadPool {
    vector<thread> workers_;
    queue<function<void()>> tasks_;
    mutex mutex_;
    condition_variable cv_;
};
```

**Sizing:**
- CPU-bound: num_cores
- I/O-bound: 2-4x cores
- HFT: Pin threads to cores

#### Atomic Operations
```cpp
atomic<int> counter{0};
counter.fetch_add(1, memory_order_relaxed);

// Memory ordering
// - relaxed: No synchronization
// - acquire: Prevents reordering before
// - release: Prevents reordering after
// - seq_cst: Full ordering (slowest)
```

#### Lock-Free Programming
```cpp
// CAS loop
T old_val = atomic_var.load();
while (!atomic_var.compare_exchange_weak(
    old_val, new_val,
    memory_order_release,
    memory_order_relaxed)) {
    // Retry
}
```

### 6. Low-Latency Optimizations

#### Cache Optimization
```cpp
// BAD: False sharing
struct {
    atomic<int> counter1;  // Same cache line
    atomic<int> counter2;
};

// GOOD: Cache line aligned
struct {
    alignas(64) atomic<int> counter1;
    alignas(64) atomic<int> counter2;
};
```

#### Branch Prediction
```cpp
// Hint to compiler
if (likely(x > 0)) { ... }
if (unlikely(error)) { ... }

// Branchless
int result = (x > 0) ? x : -x;  // May use CMOV
```

#### SIMD
```cpp
// AVX: 4 doubles in parallel
__m256d a = _mm256_load_pd(ptr);
__m256d b = _mm256_load_pd(ptr + 4);
__m256d c = _mm256_add_pd(a, b);
```

**Speedup:** 2-8x for vectorizable code

#### Memory Prefetch
```cpp
for (int i = 0; i < n; i++) {
    __builtin_prefetch(&array[i + 2]);
    process(array[i]);
}
```

### 7. System Design

#### HFT System Components
1. **Market Data Handler**
   - UDP multicast
   - Sequence gap detection
   - Lock-free processing

2. **Strategy Engine**
   - Multiple strategies
   - Signal generation
   - Risk checks

3. **Order Management**
   - State machine
   - Order routing
   - FIX protocol

4. **Execution Engine**
   - Smart order routing
   - Fill tracking
   - Latency monitoring

#### Latency Breakdown
```
Network:        1-10μs
Parsing:        200-500ns
Strategy:       1-5μs
Risk check:     50-200ns
Order send:     1-10μs
-----------------------------
Total:          2-30μs
```

#### Optimization Techniques
- CPU affinity (pin threads)
- Huge pages (reduce TLB misses)
- Kernel bypass (DPDK)
- Zero-copy networking
- NUMA-aware allocation

## Common Interview Questions

### Technical

**Q: How to achieve microsecond latency?**
A:
1. Lock-free data structures
2. CPU affinity and isolation
3. Memory pools (no allocation)
4. Minimize cache misses
5. Kernel bypass (DPDK)
6. Optimize hot paths only

**Q: Trade-offs in system design?**
A:
- Latency vs Throughput
- Consistency vs Availability
- Simplicity vs Performance
- Memory vs CPU

**Q: How to handle market data bursts?**
A:
- Ring buffers (bounded)
- Back-pressure mechanism
- Drop old data if full
- Rate limiting
- Load shedding

**Q: Explain false sharing**
A:
- Different threads accessing different variables on same cache line
- Causes cache invalidation
- Solution: Align to cache line (64 bytes)
- 10-100x performance impact

### Behavioral

**Q: Describe your most complex project**
- Focus on technical challenges
- Trade-offs made
- Results achieved
- What you learned

**Q: How do you optimize code?**
1. Profile first (measure)
2. Identify bottlenecks (80/20 rule)
3. Optimize hot paths only
4. Measure improvement
5. Don't over-optimize

**Q: Handling production issues?**
- Reproduce the issue
- Isolate the cause
- Implement fix
- Add tests
- Post-mortem

## Time/Space Complexity Cheat Sheet

| Operation | Time | Space |
|-----------|------|-------|
| Array access | O(1) | - |
| Vector push_back | O(1) amortized | - |
| Map insert/find | O(log n) | O(n) |
| Unordered_map insert/find | O(1) avg | O(n) |
| Sort | O(n log n) | O(log n) |
| Binary search | O(log n) | O(1) |
| DFS/BFS | O(V + E) | O(V) |
| Dijkstra | O((V + E) log V) | O(V) |

## Code Quality Checklist

- [ ] Correct and handles edge cases
- [ ] Optimal time/space complexity
- [ ] Clean and readable
- [ ] Good variable names
- [ ] Comments for complex logic
- [ ] Error handling
- [ ] Memory management (no leaks)
- [ ] Thread-safe if concurrent
- [ ] Testable design

## Final Tips

### Before Interview
1. Practice on whiteboard/paper
2. Review company's tech stack
3. Prepare questions to ask
4. Sleep well

### During Interview
1. Clarify requirements first
2. Think aloud
3. Start with simple solution
4. Optimize iteratively
5. Test your code
6. Discuss trade-offs

### Common Mistakes to Avoid
1. Jumping to code immediately
2. Not testing edge cases
3. Over-engineering
4. Not asking clarifying questions
5. Poor communication
6. Giving up too quickly

### Red Flags to Avoid
1. "I don't know" without trying
2. Blaming others for failures
3. Not knowing basics
4. Unable to explain own code
5. No questions for interviewer

## Resources

### Books
- Effective Modern C++ (Scott Meyers)
- C++ Concurrency in Action (Anthony Williams)
- The Art of Multiprocessor Programming

### Online
- CppCon talks
- https://en.cppreference.com
- LeetCode (practice problems)
- System Design Primer (GitHub)

### Practice
- Implement data structures from scratch
- Solve 100+ LeetCode problems
- Design 10+ systems
- Code reviews with peers

---

**Remember:** Interviewers want to see:
1. Problem-solving ability
2. Code quality
3. Communication skills
4. Depth of knowledge
5. Cultural fit

Good luck! 🚀

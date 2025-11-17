#pragma once

#include <thread>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <immintrin.h>  // SIMD intrinsics
#include "market_data/order_book.hpp"
#include "core/lock_free_queue.hpp"

namespace hft {
namespace market_data {

/**
 * Optimized Market Data Handler with Performance Enhancements
 *
 * Optimizations:
 * 1. Batch processing for reduced overhead
 * 2. SIMD operations for parallel data processing
 * 3. Prefetching for cache optimization
 * 4. Reduced lock contention with lock-free queues
 * 5. Cache-aligned data structures
 * 6. Zero-copy message processing where possible
 */

// Cache-aligned market data message (64 bytes)
struct alignas(64) MarketDataMessage {
    MessageType type;
    std::string symbol;
    Order order;
    uint64_t timestamp;
    uint8_t padding[16];  // Align to cache line
};

class OptimizedMarketDataHandler {
public:
    using MarketDataCallback = std::function<void(const std::string&, const OrderBook&)>;

    static constexpr size_t BATCH_SIZE = 64;  // Process in batches of 64
    static constexpr size_t QUEUE_SIZE = 1048576;  // 1M message queue

    OptimizedMarketDataHandler();
    ~OptimizedMarketDataHandler();

    // Lifecycle
    void start();
    void stop();

    // Message processing
    void processMessage(const MarketDataMessage& msg);
    void processBatch(const std::vector<MarketDataMessage>& messages);

    // Zero-copy batch processing (pass pointer to message array)
    void processBatchZeroCopy(const MarketDataMessage* messages, size_t count);

    // Order book access
    const OrderBook* getOrderBook(const std::string& symbol) const;
    OrderBook* getOrderBook(const std::string& symbol);

    // Subscription
    void subscribe(const std::string& symbol, MarketDataCallback callback);

    // Statistics
    uint64_t getMessagesProcessed() const {
        return messages_processed_.load(std::memory_order_relaxed);
    }

    uint64_t getAverageLatencyNs() const {
        return avg_latency_ns_.load(std::memory_order_relaxed);
    }

    uint64_t getBatchesProcessed() const {
        return batches_processed_.load(std::memory_order_relaxed);
    }

private:
    // Processing thread
    void processingThread();

    // Batch processing implementation
    void processBatchInternal(const MarketDataMessage* messages, size_t count);

    // SIMD-optimized batch operations
    void simdProcessPrices(const uint64_t* prices, size_t count, uint64_t* results);

    // Statistics update
    void updateStatistics(int64_t latency_ns);

    // Order books per symbol
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> order_books_;

    // Callbacks per symbol
    std::unordered_map<std::string, MarketDataCallback> callbacks_;

    // Lock-free message queue
    core::LockFreeQueue<MarketDataMessage, QUEUE_SIZE> message_queue_;

    // Processing thread
    std::thread processing_thread_;
    std::atomic<bool> running_{false};

    // Statistics (lock-free)
    std::atomic<uint64_t> messages_processed_{0};
    std::atomic<uint64_t> batches_processed_{0};
    std::atomic<uint64_t> avg_latency_ns_{0};

    // Batch buffer for accumulation
    alignas(64) std::array<MarketDataMessage, BATCH_SIZE> batch_buffer_;
    size_t batch_count_{0};
};

/**
 * Simulated Feed with Realistic Market Data
 */
class OptimizedSimulatedFeed {
public:
    OptimizedSimulatedFeed(OptimizedMarketDataHandler& handler);

    void start();
    void stop();

    void setSymbol(const std::string& symbol) { symbol_ = symbol; }
    void setTickIntervalUs(uint64_t us) { tick_interval_us_ = us; }

    // Generate burst of messages (for testing throughput)
    void generateBurst(size_t message_count);

private:
    void feedThread();
    MarketDataMessage generateMessage();

    OptimizedMarketDataHandler& handler_;
    std::string symbol_{"AAPL"};
    uint64_t tick_interval_us_{100};  // 100μs = 10K messages/sec

    std::thread feed_thread_;
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> next_order_id_{1};
};

} // namespace market_data
} // namespace hft

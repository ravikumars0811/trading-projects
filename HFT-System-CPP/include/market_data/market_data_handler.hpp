#pragma once

#include "order_book.hpp"
#include "core/lock_free_queue.hpp"
#include <thread>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <string>

namespace hft {
namespace market_data {

/**
 * Market data message types
 */
enum class MessageType : uint8_t {
    ORDER_ADD,
    ORDER_MODIFY,
    ORDER_CANCEL,
    TRADE,
    SNAPSHOT
};

struct MarketDataMessage {
    MessageType type;
    std::string symbol;
    Order order;
    uint64_t timestamp;
};

/**
 * Market data handler
 * Processes incoming market data and maintains order books
 */
class MarketDataHandler {
public:
    using MarketDataCallback = std::function<void(const std::string& symbol, const OrderBook&)>;

    MarketDataHandler();
    ~MarketDataHandler();

    // Lifecycle
    void start();
    void stop();

    // Feed handling
    void processMessage(const MarketDataMessage& msg);
    void processBatch(const std::vector<MarketDataMessage>& messages);

    // Order book access
    const OrderBook* getOrderBook(const std::string& symbol) const;
    OrderBook* getOrderBook(const std::string& symbol);

    // Subscribe to market data updates
    void subscribe(const std::string& symbol, MarketDataCallback callback);

    // Statistics
    uint64_t getMessagesProcessed() const { return messages_processed_; }
    uint64_t getAverageLatency() const { return avg_latency_ns_; }

private:
    void processingThread();
    void updateStatistics(int64_t latency_ns);

    std::unordered_map<std::string, std::unique_ptr<OrderBook>> order_books_;
    std::unordered_map<std::string, MarketDataCallback> callbacks_;

    core::LockFreeQueue<MarketDataMessage, 65536> message_queue_;

    std::thread processing_thread_;
    std::atomic<bool> running_{false};

    std::atomic<uint64_t> messages_processed_{0};
    std::atomic<uint64_t> avg_latency_ns_{0};
};

/**
 * Simulated market data feed for testing
 */
class SimulatedFeed {
public:
    explicit SimulatedFeed(MarketDataHandler& handler);

    void start();
    void stop();

    void setSymbol(const std::string& symbol) { symbol_ = symbol; }
    void setTickInterval(int microseconds) { tick_interval_us_ = microseconds; }

private:
    void feedThread();
    MarketDataMessage generateMessage();

    MarketDataHandler& handler_;
    std::thread feed_thread_;
    std::atomic<bool> running_{false};

    std::string symbol_ = "TEST";
    int tick_interval_us_ = 1000;  // 1ms default
    uint64_t next_order_id_ = 1;
};

} // namespace market_data
} // namespace hft

#pragma once

#include "order.hpp"
#include "types.hpp"
#include <functional>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>
#include <queue>
#include <thread>
#include <condition_variable>

namespace trading {

// Lock-free ring buffer for high-performance market data
template<typename T, size_t Size>
class LockFreeRingBuffer {
private:
    std::array<T, Size> buffer_;
    std::atomic<size_t> write_pos_{0};
    std::atomic<size_t> read_pos_{0};

public:
    bool push(const T& item) {
        size_t write = write_pos_.load(std::memory_order_relaxed);
        size_t next_write = (write + 1) % Size;
        if (next_write == read_pos_.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }
        buffer_[write] = item;
        write_pos_.store(next_write, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        size_t read = read_pos_.load(std::memory_order_relaxed);
        if (read == write_pos_.load(std::memory_order_acquire)) {
            return false; // Buffer empty
        }
        item = buffer_[read];
        read_pos_.store((read + 1) % Size, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return read_pos_.load(std::memory_order_acquire) ==
               write_pos_.load(std::memory_order_acquire);
    }

    size_t size() const {
        size_t write = write_pos_.load(std::memory_order_acquire);
        size_t read = read_pos_.load(std::memory_order_acquire);
        if (write >= read) {
            return write - read;
        }
        return Size - read + write;
    }
};

// Market data snapshot with tick data
struct TickData {
    Symbol symbol;
    Price price;
    Quantity quantity;
    Timestamp timestamp;
    bool is_bid;  // true for bid, false for ask
};

// Callback types
using MarketDataCallback = std::function<void(const MarketData&)>;
using TickDataCallback = std::function<void(const TickData&)>;

class MarketDataHandler {
public:
    MarketDataHandler();
    ~MarketDataHandler();

    // Start/Stop the handler
    void start();
    void stop();

    // Subscribe/Unsubscribe to symbols
    void subscribe(const Symbol& symbol);
    void unsubscribe(const Symbol& symbol);
    void subscribe_all(const std::vector<Symbol>& symbols);

    // Get current market data
    bool get_market_data(const Symbol& symbol, MarketData& data) const;
    std::vector<MarketData> get_all_market_data() const;

    // Register callbacks
    void register_market_data_callback(const MarketDataCallback& callback);
    void register_tick_callback(const TickDataCallback& callback);

    // Update market data (used by feed handlers)
    void update_market_data(const MarketData& data);
    void update_tick_data(const TickData& tick);

    // Historical data retrieval
    std::vector<TickData> get_recent_ticks(const Symbol& symbol, size_t count) const;

    // Statistics
    uint64_t get_updates_per_second() const { return updates_per_second_.load(); }
    uint64_t get_total_updates() const { return total_updates_.load(); }

private:
    void process_queue();
    void calculate_statistics();

    mutable std::shared_mutex data_mutex_;
    std::unordered_map<Symbol, MarketData> market_data_;
    std::unordered_map<Symbol, std::deque<TickData>> tick_history_;

    // High-performance queue for incoming data
    static constexpr size_t QUEUE_SIZE = 100000;
    LockFreeRingBuffer<MarketData, QUEUE_SIZE> market_data_queue_;
    LockFreeRingBuffer<TickData, QUEUE_SIZE> tick_queue_;

    // Callbacks
    std::vector<MarketDataCallback> market_data_callbacks_;
    std::vector<TickDataCallback> tick_callbacks_;
    mutable std::mutex callback_mutex_;

    // Worker threads
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> running_{false};

    // Statistics
    std::atomic<uint64_t> total_updates_{0};
    std::atomic<uint64_t> updates_per_second_{0};
    std::thread stats_thread_;

    static constexpr size_t MAX_TICK_HISTORY = 10000;
};

// WebSocket client for real-time market data
class WebSocketClient {
public:
    using MessageCallback = std::function<void(const std::string&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    WebSocketClient(const std::string& uri);
    ~WebSocketClient();

    void connect();
    void disconnect();
    void send(const std::string& message);

    void on_message(const MessageCallback& callback) { message_callback_ = callback; }
    void on_error(const ErrorCallback& callback) { error_callback_ = callback; }

    bool is_connected() const { return connected_.load(); }

private:
    void run();
    void reconnect();

    std::string uri_;
    std::atomic<bool> connected_{false};
    std::atomic<bool> running_{false};
    std::thread io_thread_;

    MessageCallback message_callback_;
    ErrorCallback error_callback_;

    static constexpr int RECONNECT_DELAY_MS = 5000;
};

// Market data feed interface
class IMarketDataFeed {
public:
    virtual ~IMarketDataFeed() = default;
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual void subscribe(const std::vector<Symbol>& symbols) = 0;
    virtual void unsubscribe(const std::vector<Symbol>& symbols) = 0;
    virtual bool is_connected() const = 0;
};

// Alpaca market data feed
class AlpacaMarketDataFeed : public IMarketDataFeed {
public:
    AlpacaMarketDataFeed(const std::string& api_key, const std::string& api_secret,
                         MarketDataHandler& handler);
    ~AlpacaMarketDataFeed() override;

    void connect() override;
    void disconnect() override;
    void subscribe(const std::vector<Symbol>& symbols) override;
    void unsubscribe(const std::vector<Symbol>& symbols) override;
    bool is_connected() const override;

private:
    void handle_message(const std::string& message);
    void parse_quote(const std::string& json);
    void parse_trade(const std::string& json);

    std::string api_key_;
    std::string api_secret_;
    MarketDataHandler& handler_;
    std::unique_ptr<WebSocketClient> ws_client_;
};

} // namespace trading

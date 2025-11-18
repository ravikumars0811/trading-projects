#include "../../include/market_data_handler.hpp"
#include <algorithm>
#include <iostream>
#include <chrono>

namespace trading {

MarketDataHandler::MarketDataHandler() {
    tick_history_.reserve(1000);
}

MarketDataHandler::~MarketDataHandler() {
    stop();
}

void MarketDataHandler::start() {
    if (running_.load()) return;

    running_.store(true);

    // Start worker threads for processing queues
    size_t num_workers = std::thread::hardware_concurrency() > 0 ?
                        std::thread::hardware_concurrency() : 4;

    for (size_t i = 0; i < num_workers; ++i) {
        worker_threads_.emplace_back(&MarketDataHandler::process_queue, this);
    }

    // Start statistics thread
    stats_thread_ = std::thread(&MarketDataHandler::calculate_statistics, this);

    std::cout << "MarketDataHandler started with " << num_workers << " worker threads\n";
}

void MarketDataHandler::stop() {
    if (!running_.load()) return;

    running_.store(false);

    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads_.clear();

    if (stats_thread_.joinable()) {
        stats_thread_.join();
    }

    std::cout << "MarketDataHandler stopped\n";
}

void MarketDataHandler::subscribe(const Symbol& symbol) {
    std::unique_lock lock(data_mutex_);
    if (market_data_.find(symbol) == market_data_.end()) {
        market_data_[symbol] = MarketData();
        market_data_[symbol].symbol = symbol;
        tick_history_[symbol] = std::deque<TickData>();
    }
}

void MarketDataHandler::unsubscribe(const Symbol& symbol) {
    std::unique_lock lock(data_mutex_);
    market_data_.erase(symbol);
    tick_history_.erase(symbol);
}

void MarketDataHandler::subscribe_all(const std::vector<Symbol>& symbols) {
    for (const auto& symbol : symbols) {
        subscribe(symbol);
    }
}

bool MarketDataHandler::get_market_data(const Symbol& symbol, MarketData& data) const {
    std::shared_lock lock(data_mutex_);
    auto it = market_data_.find(symbol);
    if (it != market_data_.end()) {
        data = it->second;
        return true;
    }
    return false;
}

std::vector<MarketData> MarketDataHandler::get_all_market_data() const {
    std::shared_lock lock(data_mutex_);
    std::vector<MarketData> result;
    result.reserve(market_data_.size());
    for (const auto& [symbol, data] : market_data_) {
        result.push_back(data);
    }
    return result;
}

void MarketDataHandler::register_market_data_callback(const MarketDataCallback& callback) {
    std::lock_guard lock(callback_mutex_);
    market_data_callbacks_.push_back(callback);
}

void MarketDataHandler::register_tick_callback(const TickDataCallback& callback) {
    std::lock_guard lock(callback_mutex_);
    tick_callbacks_.push_back(callback);
}

void MarketDataHandler::update_market_data(const MarketData& data) {
    if (!market_data_queue_.push(data)) {
        std::cerr << "Market data queue full! Dropping update for " << data.symbol << "\n";
    }
}

void MarketDataHandler::update_tick_data(const TickData& tick) {
    if (!tick_queue_.push(tick)) {
        std::cerr << "Tick queue full! Dropping tick for " << tick.symbol << "\n";
    }
}

std::vector<TickData> MarketDataHandler::get_recent_ticks(const Symbol& symbol, size_t count) const {
    std::shared_lock lock(data_mutex_);
    auto it = tick_history_.find(symbol);
    if (it == tick_history_.end()) {
        return {};
    }

    const auto& history = it->second;
    size_t start = history.size() > count ? history.size() - count : 0;
    return std::vector<TickData>(history.begin() + start, history.end());
}

void MarketDataHandler::process_queue() {
    MarketData md;
    TickData tick;

    while (running_.load()) {
        bool processed = false;

        // Process market data
        while (market_data_queue_.pop(md)) {
            {
                std::unique_lock lock(data_mutex_);
                market_data_[md.symbol] = md;
            }

            // Call callbacks
            {
                std::lock_guard lock(callback_mutex_);
                for (const auto& callback : market_data_callbacks_) {
                    try {
                        callback(md);
                    } catch (const std::exception& e) {
                        std::cerr << "Exception in market data callback: " << e.what() << "\n";
                    }
                }
            }

            total_updates_.fetch_add(1);
            processed = true;
        }

        // Process tick data
        while (tick_queue_.pop(tick)) {
            {
                std::unique_lock lock(data_mutex_);
                auto& history = tick_history_[tick.symbol];
                history.push_back(tick);
                if (history.size() > MAX_TICK_HISTORY) {
                    history.pop_front();
                }
            }

            // Call callbacks
            {
                std::lock_guard lock(callback_mutex_);
                for (const auto& callback : tick_callbacks_) {
                    try {
                        callback(tick);
                    } catch (const std::exception& e) {
                        std::cerr << "Exception in tick callback: " << e.what() << "\n";
                    }
                }
            }

            total_updates_.fetch_add(1);
            processed = true;
        }

        if (!processed) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

void MarketDataHandler::calculate_statistics() {
    uint64_t last_count = 0;

    while (running_.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        uint64_t current_count = total_updates_.load();
        updates_per_second_.store(current_count - last_count);
        last_count = current_count;
    }
}

// WebSocket Client Implementation
WebSocketClient::WebSocketClient(const std::string& uri) : uri_(uri) {}

WebSocketClient::~WebSocketClient() {
    disconnect();
}

void WebSocketClient::connect() {
    if (connected_.load()) return;

    running_.store(true);
    io_thread_ = std::thread(&WebSocketClient::run, this);

    std::cout << "WebSocket connecting to " << uri_ << "\n";
}

void WebSocketClient::disconnect() {
    if (!running_.load()) return;

    running_.store(false);
    connected_.store(false);

    if (io_thread_.joinable()) {
        io_thread_.join();
    }

    std::cout << "WebSocket disconnected\n";
}

void WebSocketClient::send(const std::string& message) {
    if (!connected_.load()) {
        std::cerr << "WebSocket not connected, cannot send message\n";
        return;
    }
    // Implementation would use a real WebSocket library like websocketpp or Boost.Beast
    std::cout << "Sending: " << message << "\n";
}

void WebSocketClient::run() {
    // This is a simplified version. In production, use websocketpp or Boost.Beast
    std::cout << "WebSocket I/O thread started\n";

    while (running_.load()) {
        if (!connected_.load()) {
            reconnect();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void WebSocketClient::reconnect() {
    std::cout << "Attempting to reconnect...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY_MS));
    // Implementation would actually reconnect here
    connected_.store(true);
}

// Alpaca Market Data Feed Implementation
AlpacaMarketDataFeed::AlpacaMarketDataFeed(const std::string& api_key,
                                           const std::string& api_secret,
                                           MarketDataHandler& handler)
    : api_key_(api_key), api_secret_(api_secret), handler_(handler) {
    ws_client_ = std::make_unique<WebSocketClient>("wss://stream.data.alpaca.markets/v2/iex");
}

AlpacaMarketDataFeed::~AlpacaMarketDataFeed() {
    disconnect();
}

void AlpacaMarketDataFeed::connect() {
    ws_client_->on_message([this](const std::string& msg) {
        handle_message(msg);
    });

    ws_client_->on_error([](const std::string& error) {
        std::cerr << "WebSocket error: " << error << "\n";
    });

    ws_client_->connect();

    // Authenticate
    std::string auth_msg = R"({"action":"auth","key":")" + api_key_ +
                          R"(","secret":")" + api_secret_ + R"("})";
    ws_client_->send(auth_msg);
}

void AlpacaMarketDataFeed::disconnect() {
    if (ws_client_) {
        ws_client_->disconnect();
    }
}

void AlpacaMarketDataFeed::subscribe(const std::vector<Symbol>& symbols) {
    std::string symbols_str;
    for (size_t i = 0; i < symbols.size(); ++i) {
        symbols_str += "\"" + symbols[i] + "\"";
        if (i < symbols.size() - 1) symbols_str += ",";
    }

    std::string sub_msg = R"({"action":"subscribe","quotes":[)" + symbols_str +
                         R"(],"trades":[)" + symbols_str + R"(]})";
    ws_client_->send(sub_msg);

    for (const auto& symbol : symbols) {
        handler_.subscribe(symbol);
    }
}

void AlpacaMarketDataFeed::unsubscribe(const std::vector<Symbol>& symbols) {
    std::string symbols_str;
    for (size_t i = 0; i < symbols.size(); ++i) {
        symbols_str += "\"" + symbols[i] + "\"";
        if (i < symbols.size() - 1) symbols_str += ",";
    }

    std::string unsub_msg = R"({"action":"unsubscribe","quotes":[)" + symbols_str +
                           R"(],"trades":[)" + symbols_str + R"(]})";
    ws_client_->send(unsub_msg);
}

bool AlpacaMarketDataFeed::is_connected() const {
    return ws_client_ && ws_client_->is_connected();
}

void AlpacaMarketDataFeed::handle_message(const std::string& message) {
    // In production, use a JSON library like nlohmann/json or rapidjson
    // This is a simplified example
    if (message.find("\"T\":\"q\"") != std::string::npos) {
        parse_quote(message);
    } else if (message.find("\"T\":\"t\"") != std::string::npos) {
        parse_trade(message);
    }
}

void AlpacaMarketDataFeed::parse_quote(const std::string& json) {
    // Simplified parsing - use a proper JSON library in production
    MarketData md;
    md.timestamp = now();

    // Extract values from JSON (simplified)
    // In production: use nlohmann::json or rapidjson
    // Example: {"T":"q","S":"AAPL","bp":150.10,"bs":100,"ap":150.15,"as":200}

    handler_.update_market_data(md);
}

void AlpacaMarketDataFeed::parse_trade(const std::string& json) {
    // Simplified parsing
    TickData tick;
    tick.timestamp = now();

    handler_.update_tick_data(tick);
}

} // namespace trading

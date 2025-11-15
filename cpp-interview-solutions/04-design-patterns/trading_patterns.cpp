/**
 * Design Patterns for Trading Systems
 *
 * Essential patterns for HFT and trading platforms:
 * 1. Strategy Pattern - Multiple trading strategies
 * 2. Observer Pattern - Market data distribution
 * 3. Factory Pattern - Order creation
 * 4. Command Pattern - Order operations
 * 5. Singleton Pattern - Configuration/connection managers
 * 6. Object Pool Pattern - Memory management
 * 7. State Pattern - Order lifecycle
 * 8. Chain of Responsibility - Risk checks
 *
 * Interview Focus:
 * - When to use each pattern
 * - Real-world trading applications
 * - Performance implications
 * - Alternative approaches
 */

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <queue>

// ============================================================================
// 1. STRATEGY PATTERN - Trading Strategies
// ============================================================================

// Market data structure
struct MarketData {
    std::string symbol;
    double bid;
    double ask;
    int bid_size;
    int ask_size;
    uint64_t timestamp;
};

// Strategy interface
class TradingStrategy {
public:
    virtual ~TradingStrategy() = default;
    virtual void onMarketData(const MarketData& data) = 0;
    virtual std::string getName() const = 0;
};

// Concrete Strategy 1: Market Making
class MarketMakingStrategy : public TradingStrategy {
private:
    double spread_target_;
    int max_position_;
    int current_position_;

public:
    explicit MarketMakingStrategy(double spread, int max_pos)
        : spread_target_(spread), max_position_(max_pos), current_position_(0) {}

    void onMarketData(const MarketData& data) override {
        double mid = (data.bid + data.ask) / 2.0;
        double half_spread = spread_target_ / 2.0;

        std::cout << "[MarketMaking] " << data.symbol
                  << " Quote: " << (mid - half_spread)
                  << " / " << (mid + half_spread) << std::endl;
    }

    std::string getName() const override { return "MarketMaking"; }
};

// Concrete Strategy 2: Momentum Trading
class MomentumStrategy : public TradingStrategy {
private:
    double threshold_;
    double last_price_;

public:
    explicit MomentumStrategy(double threshold)
        : threshold_(threshold), last_price_(0.0) {}

    void onMarketData(const MarketData& data) override {
        double mid = (data.bid + data.ask) / 2.0;

        if (last_price_ > 0) {
            double change = (mid - last_price_) / last_price_;
            if (std::abs(change) > threshold_) {
                std::cout << "[Momentum] " << data.symbol
                          << " Signal: " << (change > 0 ? "BUY" : "SELL")
                          << " Change: " << (change * 100) << "%" << std::endl;
            }
        }
        last_price_ = mid;
    }

    std::string getName() const override { return "Momentum"; }
};

// Concrete Strategy 3: Mean Reversion
class MeanReversionStrategy : public TradingStrategy {
private:
    std::vector<double> price_history_;
    size_t window_size_;
    double z_score_threshold_;

public:
    MeanReversionStrategy(size_t window, double threshold)
        : window_size_(window), z_score_threshold_(threshold) {}

    void onMarketData(const MarketData& data) override {
        double mid = (data.bid + data.ask) / 2.0;
        price_history_.push_back(mid);

        if (price_history_.size() > window_size_) {
            price_history_.erase(price_history_.begin());
        }

        if (price_history_.size() == window_size_) {
            double mean = 0.0;
            for (double p : price_history_) mean += p;
            mean /= window_size_;

            double variance = 0.0;
            for (double p : price_history_) {
                variance += (p - mean) * (p - mean);
            }
            double std_dev = std::sqrt(variance / window_size_);

            if (std_dev > 0) {
                double z_score = (mid - mean) / std_dev;
                if (std::abs(z_score) > z_score_threshold_) {
                    std::cout << "[MeanReversion] " << data.symbol
                              << " Signal: " << (z_score > 0 ? "SELL" : "BUY")
                              << " Z-score: " << z_score << std::endl;
                }
            }
        }
    }

    std::string getName() const override { return "MeanReversion"; }
};

// Strategy Manager (Context)
class StrategyManager {
private:
    std::vector<std::unique_ptr<TradingStrategy>> strategies_;

public:
    void addStrategy(std::unique_ptr<TradingStrategy> strategy) {
        strategies_.push_back(std::move(strategy));
    }

    void distributeMarketData(const MarketData& data) {
        for (auto& strategy : strategies_) {
            strategy->onMarketData(data);
        }
    }
};

// ============================================================================
// 2. OBSERVER PATTERN - Market Data Distribution
// ============================================================================

// Observer interface
class MarketDataObserver {
public:
    virtual ~MarketDataObserver() = default;
    virtual void update(const MarketData& data) = 0;
};

// Subject (Observable)
class MarketDataFeed {
private:
    std::vector<MarketDataObserver*> observers_;
    MarketData latest_data_;

public:
    void attach(MarketDataObserver* observer) {
        observers_.push_back(observer);
    }

    void detach(MarketDataObserver* observer) {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end()
        );
    }

    void notify() {
        for (auto* observer : observers_) {
            observer->update(latest_data_);
        }
    }

    void updateMarketData(const MarketData& data) {
        latest_data_ = data;
        notify();
    }
};

// Concrete Observer: Trading Engine
class TradingEngine : public MarketDataObserver {
private:
    std::string name_;

public:
    explicit TradingEngine(const std::string& name) : name_(name) {}

    void update(const MarketData& data) override {
        std::cout << "[Engine:" << name_ << "] Received: "
                  << data.symbol << " " << data.bid << "/" << data.ask
                  << std::endl;
    }
};

// ============================================================================
// 3. FACTORY PATTERN - Order Creation
// ============================================================================

enum class OrderSide { BUY, SELL };
enum class OrderType { LIMIT, MARKET, STOP, STOP_LIMIT };

struct Order {
    uint64_t id;
    std::string symbol;
    OrderSide side;
    OrderType type;
    double price;
    int quantity;

    virtual ~Order() = default;
    virtual void print() const = 0;
};

struct LimitOrder : public Order {
    void print() const override {
        std::cout << "LimitOrder #" << id << ": "
                  << (side == OrderSide::BUY ? "BUY" : "SELL") << " "
                  << quantity << " " << symbol << " @ " << price << std::endl;
    }
};

struct MarketOrder : public Order {
    void print() const override {
        std::cout << "MarketOrder #" << id << ": "
                  << (side == OrderSide::BUY ? "BUY" : "SELL") << " "
                  << quantity << " " << symbol << " @ MARKET" << std::endl;
    }
};

struct StopOrder : public Order {
    double stop_price;

    void print() const override {
        std::cout << "StopOrder #" << id << ": "
                  << (side == OrderSide::BUY ? "BUY" : "SELL") << " "
                  << quantity << " " << symbol
                  << " @ " << price << " stop " << stop_price << std::endl;
    }
};

// Factory
class OrderFactory {
private:
    static uint64_t next_id_;

public:
    static std::unique_ptr<Order> createLimitOrder(
        const std::string& symbol, OrderSide side, double price, int qty) {
        auto order = std::make_unique<LimitOrder>();
        order->id = next_id_++;
        order->symbol = symbol;
        order->side = side;
        order->type = OrderType::LIMIT;
        order->price = price;
        order->quantity = qty;
        return order;
    }

    static std::unique_ptr<Order> createMarketOrder(
        const std::string& symbol, OrderSide side, int qty) {
        auto order = std::make_unique<MarketOrder>();
        order->id = next_id_++;
        order->symbol = symbol;
        order->side = side;
        order->type = OrderType::MARKET;
        order->price = 0.0;
        order->quantity = qty;
        return order;
    }

    static std::unique_ptr<Order> createStopOrder(
        const std::string& symbol, OrderSide side,
        double price, double stop_price, int qty) {
        auto order = std::make_unique<StopOrder>();
        order->id = next_id_++;
        order->symbol = symbol;
        order->side = side;
        order->type = OrderType::STOP;
        order->price = price;
        order->stop_price = stop_price;
        order->quantity = qty;
        return order;
    }
};

uint64_t OrderFactory::next_id_ = 1;

// ============================================================================
// 4. COMMAND PATTERN - Order Operations
// ============================================================================

// Command interface
class OrderCommand {
public:
    virtual ~OrderCommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

// Receiver
class OrderManagementSystem {
public:
    void submitOrder(uint64_t order_id) {
        std::cout << "OMS: Submitted order #" << order_id << std::endl;
    }

    void cancelOrder(uint64_t order_id) {
        std::cout << "OMS: Cancelled order #" << order_id << std::endl;
    }

    void modifyOrder(uint64_t order_id, double new_price) {
        std::cout << "OMS: Modified order #" << order_id
                  << " to price " << new_price << std::endl;
    }
};

// Concrete Commands
class SubmitOrderCommand : public OrderCommand {
private:
    OrderManagementSystem* oms_;
    uint64_t order_id_;

public:
    SubmitOrderCommand(OrderManagementSystem* oms, uint64_t id)
        : oms_(oms), order_id_(id) {}

    void execute() override {
        oms_->submitOrder(order_id_);
    }

    void undo() override {
        oms_->cancelOrder(order_id_);
    }
};

class CancelOrderCommand : public OrderCommand {
private:
    OrderManagementSystem* oms_;
    uint64_t order_id_;

public:
    CancelOrderCommand(OrderManagementSystem* oms, uint64_t id)
        : oms_(oms), order_id_(id) {}

    void execute() override {
        oms_->cancelOrder(order_id_);
    }

    void undo() override {
        oms_->submitOrder(order_id_);
    }
};

// ============================================================================
// 5. SINGLETON PATTERN - Configuration Manager
// ============================================================================

class ConfigurationManager {
private:
    static std::unique_ptr<ConfigurationManager> instance_;
    static std::mutex mutex_;

    std::unordered_map<std::string, std::string> config_;

    // Private constructor
    ConfigurationManager() {
        config_["max_position"] = "10000";
        config_["risk_limit"] = "1000000";
        config_["api_endpoint"] = "https://api.exchange.com";
    }

public:
    // Delete copy constructor and assignment
    ConfigurationManager(const ConfigurationManager&) = delete;
    ConfigurationManager& operator=(const ConfigurationManager&) = delete;

    static ConfigurationManager* getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
            instance_.reset(new ConfigurationManager());
        }
        return instance_.get();
    }

    std::string get(const std::string& key) {
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : "";
    }

    void set(const std::string& key, const std::string& value) {
        config_[key] = value;
    }
};

std::unique_ptr<ConfigurationManager> ConfigurationManager::instance_ = nullptr;
std::mutex ConfigurationManager::mutex_;

// ============================================================================
// 6. OBJECT POOL PATTERN - Order Object Pool
// ============================================================================

template<typename T>
class ObjectPool {
private:
    std::queue<std::unique_ptr<T>> available_;
    std::mutex mutex_;
    size_t created_count_;
    size_t max_size_;

public:
    explicit ObjectPool(size_t max_size = 1000)
        : created_count_(0), max_size_(max_size) {}

    std::unique_ptr<T> acquire() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!available_.empty()) {
            auto obj = std::move(available_.front());
            available_.pop();
            return obj;
        }

        if (created_count_ < max_size_) {
            ++created_count_;
            return std::make_unique<T>();
        }

        return nullptr;  // Pool exhausted
    }

    void release(std::unique_ptr<T> obj) {
        if (obj) {
            std::lock_guard<std::mutex> lock(mutex_);
            available_.push(std::move(obj));
        }
    }

    size_t available_count() const {
        return available_.size();
    }

    size_t total_created() const {
        return created_count_;
    }
};

// ============================================================================
// 7. STATE PATTERN - Order State Machine
// ============================================================================

class OrderState;

class OrderContext {
private:
    std::unique_ptr<OrderState> state_;
    uint64_t order_id_;

public:
    explicit OrderContext(uint64_t id);
    void setState(std::unique_ptr<OrderState> state);
    void submit();
    void acknowledge();
    void fill();
    void cancel();
    uint64_t getOrderId() const { return order_id_; }
};

class OrderState {
public:
    virtual ~OrderState() = default;
    virtual void submit(OrderContext* context) = 0;
    virtual void acknowledge(OrderContext* context) = 0;
    virtual void fill(OrderContext* context) = 0;
    virtual void cancel(OrderContext* context) = 0;
    virtual std::string name() const = 0;
};

class NewState : public OrderState {
public:
    void submit(OrderContext* context) override;
    void acknowledge(OrderContext*) override {
        std::cout << "Cannot acknowledge from NEW state" << std::endl;
    }
    void fill(OrderContext*) override {
        std::cout << "Cannot fill from NEW state" << std::endl;
    }
    void cancel(OrderContext*) override {
        std::cout << "Cancelled from NEW state" << std::endl;
    }
    std::string name() const override { return "NEW"; }
};

class PendingState : public OrderState {
public:
    void submit(OrderContext*) override {
        std::cout << "Already submitted" << std::endl;
    }
    void acknowledge(OrderContext* context) override;
    void fill(OrderContext*) override {
        std::cout << "Cannot fill from PENDING state" << std::endl;
    }
    void cancel(OrderContext*) override {
        std::cout << "Cancelled from PENDING state" << std::endl;
    }
    std::string name() const override { return "PENDING"; }
};

class AcknowledgedState : public OrderState {
public:
    void submit(OrderContext*) override {
        std::cout << "Already submitted" << std::endl;
    }
    void acknowledge(OrderContext*) override {
        std::cout << "Already acknowledged" << std::endl;
    }
    void fill(OrderContext* context) override;
    void cancel(OrderContext*) override {
        std::cout << "Cancelled from ACKNOWLEDGED state" << std::endl;
    }
    std::string name() const override { return "ACKNOWLEDGED"; }
};

class FilledState : public OrderState {
public:
    void submit(OrderContext*) override {
        std::cout << "Order already filled" << std::endl;
    }
    void acknowledge(OrderContext*) override {
        std::cout << "Order already filled" << std::endl;
    }
    void fill(OrderContext*) override {
        std::cout << "Order already filled" << std::endl;
    }
    void cancel(OrderContext*) override {
        std::cout << "Cannot cancel filled order" << std::endl;
    }
    std::string name() const override { return "FILLED"; }
};

// State implementations
void NewState::submit(OrderContext* context) {
    std::cout << "Order #" << context->getOrderId() << ": NEW -> PENDING" << std::endl;
    context->setState(std::make_unique<PendingState>());
}

void PendingState::acknowledge(OrderContext* context) {
    std::cout << "Order #" << context->getOrderId() << ": PENDING -> ACKNOWLEDGED" << std::endl;
    context->setState(std::make_unique<AcknowledgedState>());
}

void AcknowledgedState::fill(OrderContext* context) {
    std::cout << "Order #" << context->getOrderId() << ": ACKNOWLEDGED -> FILLED" << std::endl;
    context->setState(std::make_unique<FilledState>());
}

// OrderContext implementation
OrderContext::OrderContext(uint64_t id)
    : state_(std::make_unique<NewState>()), order_id_(id) {}

void OrderContext::setState(std::unique_ptr<OrderState> state) {
    state_ = std::move(state);
}

void OrderContext::submit() { state_->submit(this); }
void OrderContext::acknowledge() { state_->acknowledge(this); }
void OrderContext::fill() { state_->fill(this); }
void OrderContext::cancel() { state_->cancel(this); }

// ============================================================================
// DEMONSTRATION
// ============================================================================

void demonstrate_patterns() {
    std::cout << "\n=== STRATEGY PATTERN ===" << std::endl;
    StrategyManager manager;
    manager.addStrategy(std::make_unique<MarketMakingStrategy>(0.02, 1000));
    manager.addStrategy(std::make_unique<MomentumStrategy>(0.01));
    manager.addStrategy(std::make_unique<MeanReversionStrategy>(10, 2.0));

    MarketData data{"AAPL", 150.00, 150.02, 100, 100, 123456};
    manager.distributeMarketData(data);

    std::cout << "\n=== OBSERVER PATTERN ===" << std::endl;
    MarketDataFeed feed;
    TradingEngine engine1("Alpha");
    TradingEngine engine2("Beta");

    feed.attach(&engine1);
    feed.attach(&engine2);
    feed.updateMarketData(data);

    std::cout << "\n=== FACTORY PATTERN ===" << std::endl;
    auto limit_order = OrderFactory::createLimitOrder("GOOGL", OrderSide::BUY, 2800.0, 10);
    auto market_order = OrderFactory::createMarketOrder("MSFT", OrderSide::SELL, 50);
    auto stop_order = OrderFactory::createStopOrder("TSLA", OrderSide::BUY, 700.0, 695.0, 25);

    limit_order->print();
    market_order->print();
    stop_order->print();

    std::cout << "\n=== COMMAND PATTERN ===" << std::endl;
    OrderManagementSystem oms;
    SubmitOrderCommand submit_cmd(&oms, 123);
    CancelOrderCommand cancel_cmd(&oms, 123);

    submit_cmd.execute();
    cancel_cmd.execute();
    cancel_cmd.undo();  // Re-submit

    std::cout << "\n=== SINGLETON PATTERN ===" << std::endl;
    auto* config = ConfigurationManager::getInstance();
    std::cout << "Max Position: " << config->get("max_position") << std::endl;
    std::cout << "Risk Limit: " << config->get("risk_limit") << std::endl;

    std::cout << "\n=== OBJECT POOL PATTERN ===" << std::endl;
    ObjectPool<LimitOrder> pool(100);
    auto order1 = pool.acquire();
    auto order2 = pool.acquire();
    std::cout << "Created: " << pool.total_created()
              << ", Available: " << pool.available_count() << std::endl;
    pool.release(std::move(order1));
    std::cout << "After release - Available: " << pool.available_count() << std::endl;

    std::cout << "\n=== STATE PATTERN ===" << std::endl;
    OrderContext order(456);
    order.submit();
    order.acknowledge();
    order.fill();
    order.cancel();  // Should fail - already filled
}

/**
 * KEY INTERVIEW POINTS:
 *
 * 1. Strategy Pattern:
 *    - When: Multiple algorithms, runtime selection
 *    - HFT Use: Different trading strategies, execution algos
 *    - Benefit: Easy to add new strategies, testable
 *
 * 2. Observer Pattern:
 *    - When: One-to-many dependency
 *    - HFT Use: Market data distribution, event notification
 *    - Caution: Can cause performance issues if many observers
 *
 * 3. Factory Pattern:
 *    - When: Complex object creation
 *    - HFT Use: Order creation, connection factories
 *    - Benefit: Centralized creation logic, validation
 *
 * 4. Command Pattern:
 *    - When: Encapsulate requests, support undo
 *    - HFT Use: Order operations, transaction log
 *    - Benefit: Audit trail, replay capability
 *
 * 5. Singleton Pattern:
 *    - When: Exactly one instance needed
 *    - HFT Use: Config, connection manager, logger
 *    - Caution: Global state, testing difficulty
 *    - Modern: Prefer dependency injection
 *
 * 6. Object Pool Pattern:
 *    - When: Object creation is expensive
 *    - HFT Use: Order objects, network buffers
 *    - Benefit: Reduce allocation overhead, predictable latency
 *    - Critical: Very important for low-latency systems
 *
 * 7. State Pattern:
 *    - When: Object behavior changes with state
 *    - HFT Use: Order lifecycle, connection state
 *    - Benefit: Clear state transitions, maintainable
 *
 * 8. Common Interview Questions:
 *    Q: When to use Strategy vs State?
 *    A: Strategy: Interchangeable algorithms
 *       State: Behavior changes with internal state
 *
 *    Q: Why use Object Pool in HFT?
 *    A: Avoid allocation in hot path, predictable latency,
 *       reduce memory fragmentation, GC-free
 *
 *    Q: Singleton in multi-threaded environment?
 *    A: Use double-checked locking or Meyer's singleton
 *       Be careful with initialization order
 *
 * 9. Performance Implications:
 *    - Virtual calls: ~5-20ns overhead
 *    - Object pooling: Saves 100-500ns per allocation
 *    - Factory: Minimal overhead, worth the clarity
 *    - Observer: O(n) notify, consider lock-free queue
 *
 * 10. Modern C++ Alternatives:
 *     - Strategy: std::function, lambdas
 *     - Singleton: Namespace-level functions
 *     - Factory: make_unique with if/switch
 *     - Observer: Signals/slots libraries
 *
 * Best Practices for HFT:
 * - Minimize virtual calls in hot paths
 * - Use object pools for frequently allocated objects
 * - Prefer composition over inheritance
 * - Consider template-based patterns (zero overhead)
 * - Profile pattern overhead in critical paths
 */

int main() {
    demonstrate_patterns();
    return 0;
}

/**
 * Smart Pointers - Essential for Resource Management in HFT
 *
 * Applications in HFT:
 * - Automatic resource cleanup (network connections, file handles)
 * - Exception-safe code in critical paths
 * - Shared ownership of market data feeds
 * - Unique ownership of order objects
 *
 * Interview Focus:
 * - Implementing smart pointers from scratch
 * - unique_ptr vs shared_ptr vs weak_ptr
 * - Custom deleters
 * - Performance implications
 */

#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <atomic>

// Example 1: Implementing unique_ptr from scratch
template<typename T>
class UniquePtr {
private:
    T* ptr_;

public:
    // Constructor
    explicit UniquePtr(T* ptr = nullptr) : ptr_(ptr) {}

    // Destructor - RAII cleanup
    ~UniquePtr() {
        delete ptr_;
    }

    // Delete copy operations - unique ownership
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // Move constructor
    UniquePtr(UniquePtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    // Move assignment
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    // Dereference operators
    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }

    // Get raw pointer
    T* get() const { return ptr_; }

    // Release ownership
    T* release() {
        T* temp = ptr_;
        ptr_ = nullptr;
        return temp;
    }

    // Reset pointer
    void reset(T* ptr = nullptr) {
        delete ptr_;
        ptr_ = ptr;
    }

    // Boolean conversion
    explicit operator bool() const { return ptr_ != nullptr; }
};

// Example 2: Implementing shared_ptr from scratch (simplified)
template<typename T>
class SharedPtr {
private:
    T* ptr_;
    std::atomic<size_t>* ref_count_;

    void release() {
        if (ref_count_) {
            if (--(*ref_count_) == 0) {
                delete ptr_;
                delete ref_count_;
            }
        }
    }

public:
    // Constructor
    explicit SharedPtr(T* ptr = nullptr)
        : ptr_(ptr), ref_count_(ptr ? new std::atomic<size_t>(1) : nullptr) {}

    // Destructor
    ~SharedPtr() {
        release();
    }

    // Copy constructor
    SharedPtr(const SharedPtr& other)
        : ptr_(other.ptr_), ref_count_(other.ref_count_) {
        if (ref_count_) {
            ++(*ref_count_);
        }
    }

    // Copy assignment
    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            ref_count_ = other.ref_count_;
            if (ref_count_) {
                ++(*ref_count_);
            }
        }
        return *this;
    }

    // Move constructor
    SharedPtr(SharedPtr&& other) noexcept
        : ptr_(other.ptr_), ref_count_(other.ref_count_) {
        other.ptr_ = nullptr;
        other.ref_count_ = nullptr;
    }

    // Move assignment
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            ref_count_ = other.ref_count_;
            other.ptr_ = nullptr;
            other.ref_count_ = nullptr;
        }
        return *this;
    }

    // Dereference operators
    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    T* get() const { return ptr_; }

    // Get reference count
    size_t use_count() const {
        return ref_count_ ? ref_count_->load() : 0;
    }

    // Boolean conversion
    explicit operator bool() const { return ptr_ != nullptr; }
};

// Example 3: Real-world HFT application - Market Data Feed
struct MarketDataFeed {
    std::string exchange;
    std::vector<double> prices;

    MarketDataFeed(const std::string& exch) : exchange(exch) {
        std::cout << "MarketDataFeed created for " << exchange << std::endl;
    }

    ~MarketDataFeed() {
        std::cout << "MarketDataFeed destroyed for " << exchange << std::endl;
    }

    void addPrice(double price) {
        prices.push_back(price);
    }
};

// Example 4: Custom Deleter - Critical for resource management
class NetworkConnection {
private:
    int socket_fd_;
    std::string address_;

public:
    NetworkConnection(const std::string& addr, int fd)
        : socket_fd_(fd), address_(addr) {
        std::cout << "Connection opened to " << address_ << std::endl;
    }

    ~NetworkConnection() {
        std::cout << "Connection closed to " << address_ << std::endl;
        // In real code: close(socket_fd_);
    }

    void send(const std::string& data) {
        std::cout << "Sending to " << address_ << ": " << data << std::endl;
    }
};

// Custom deleter for network connection
struct NetworkDeleter {
    void operator()(NetworkConnection* conn) const {
        std::cout << "Custom deleter called" << std::endl;
        delete conn;
    }
};

// Example 5: weak_ptr to break circular references
class OrderBook;

class MarketMaker {
private:
    std::string name_;
    std::weak_ptr<OrderBook> order_book_;  // weak_ptr prevents circular reference

public:
    MarketMaker(const std::string& name) : name_(name) {}

    void setOrderBook(std::weak_ptr<OrderBook> book) {
        order_book_ = book;
    }

    void placeOrder() {
        if (auto book = order_book_.lock()) {  // Convert to shared_ptr
            std::cout << name_ << " placing order in order book" << std::endl;
        } else {
            std::cout << "Order book no longer exists" << std::endl;
        }
    }

    ~MarketMaker() {
        std::cout << "MarketMaker " << name_ << " destroyed" << std::endl;
    }
};

class OrderBook {
private:
    std::vector<std::shared_ptr<MarketMaker>> makers_;

public:
    void addMaker(std::shared_ptr<MarketMaker> maker) {
        makers_.push_back(maker);
    }

    ~OrderBook() {
        std::cout << "OrderBook destroyed" << std::endl;
    }
};

// Example 6: make_unique and make_shared - Exception Safety
template<typename T, typename... Args>
UniquePtr<T> make_unique_custom(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

// Example 7: Array specialization
template<typename T>
class UniqueArrayPtr {
private:
    T* ptr_;

public:
    explicit UniqueArrayPtr(T* ptr = nullptr) : ptr_(ptr) {}

    ~UniqueArrayPtr() {
        delete[] ptr_;  // Array delete
    }

    UniqueArrayPtr(const UniqueArrayPtr&) = delete;
    UniqueArrayPtr& operator=(const UniqueArrayPtr&) = delete;

    UniqueArrayPtr(UniqueArrayPtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    T& operator[](size_t index) const { return ptr_[index]; }
    T* get() const { return ptr_; }
};

// Performance comparison structure
struct Order {
    char symbol[16];
    double price;
    int quantity;

    Order(const char* sym, double p, int q) : price(p), quantity(q) {
        strncpy(symbol, sym, 15);
        symbol[15] = '\0';
    }

    ~Order() {
        // std::cout << "Order destroyed: " << symbol << std::endl;
    }
};

void demonstrate_smart_pointers() {
    std::cout << "\n=== Custom UniquePtr ===" << std::endl;
    {
        UniquePtr<Order> order1(new Order("AAPL", 150.0, 100));
        std::cout << "Order: " << order1->symbol << ", Price: " << order1->price << std::endl;

        // Move ownership
        UniquePtr<Order> order2 = std::move(order1);
        // order1 is now nullptr
    }  // order2 destroyed here, memory freed

    std::cout << "\n=== Custom SharedPtr ===" << std::endl;
    {
        SharedPtr<Order> shared1(new Order("GOOGL", 2800.0, 50));
        std::cout << "Ref count: " << shared1.use_count() << std::endl;

        {
            SharedPtr<Order> shared2 = shared1;
            std::cout << "Ref count: " << shared1.use_count() << std::endl;

            SharedPtr<Order> shared3 = shared1;
            std::cout << "Ref count: " << shared1.use_count() << std::endl;
        }  // shared2 and shared3 destroyed

        std::cout << "Ref count: " << shared1.use_count() << std::endl;
    }  // shared1 destroyed, memory freed

    std::cout << "\n=== Standard unique_ptr with Custom Deleter ===" << std::endl;
    {
        std::unique_ptr<NetworkConnection, NetworkDeleter> conn(
            new NetworkConnection("192.168.1.100:8080", 42),
            NetworkDeleter()
        );
        conn->send("BUY AAPL 100@150");
    }  // Custom deleter called

    std::cout << "\n=== Shared Market Data Feed ===" << std::endl;
    {
        auto feed = std::make_shared<MarketDataFeed>("NYSE");

        std::vector<std::shared_ptr<MarketDataFeed>> subscribers;
        subscribers.push_back(feed);
        subscribers.push_back(feed);
        subscribers.push_back(feed);

        std::cout << "Feed ref count: " << feed.use_count() << std::endl;

        feed->addPrice(150.25);
        feed->addPrice(150.50);
    }  // All references released, feed destroyed

    std::cout << "\n=== weak_ptr to Break Circular References ===" << std::endl;
    {
        auto book = std::make_shared<OrderBook>();
        auto maker1 = std::make_shared<MarketMaker>("MM1");
        auto maker2 = std::make_shared<MarketMaker>("MM2");

        maker1->setOrderBook(book);
        maker2->setOrderBook(book);

        book->addMaker(maker1);
        book->addMaker(maker2);

        maker1->placeOrder();
        maker2->placeOrder();
    }  // Properly cleaned up without memory leak

    std::cout << "\n=== Array Support ===" << std::endl;
    {
        auto prices = std::make_unique<double[]>(5);
        for (int i = 0; i < 5; ++i) {
            prices[i] = 100.0 + i * 0.5;
        }

        UniqueArrayPtr<int> quantities(new int[5]{100, 200, 300, 400, 500});
        for (int i = 0; i < 5; ++i) {
            std::cout << "Price: " << prices[i] << ", Qty: " << quantities[i] << std::endl;
        }
    }

    std::cout << "\n=== Exception Safety ===" << std::endl;
    {
        // WRONG - not exception safe
        // process(std::shared_ptr<Order>(new Order(...)), compute());
        // If compute() throws, Order leaks!

        // RIGHT - exception safe
        auto order = std::make_shared<Order>("TSLA", 700.0, 25);
        // process(order, compute());
    }
}

/**
 * KEY INTERVIEW POINTS:
 *
 * 1. unique_ptr:
 *    - Zero overhead vs raw pointer (same size, no ref counting)
 *    - Move-only semantics
 *    - Perfect for ownership transfer in HFT pipelines
 *    - Use for: exclusive ownership, factory functions, PIMPL
 *
 * 2. shared_ptr:
 *    - Reference counting (atomic - thread-safe but slower)
 *    - Size: 2 * sizeof(void*) - pointer + control block
 *    - Use for: shared ownership, caching, multi-threaded access
 *    - Overhead: atomic increment/decrement on copy
 *
 * 3. weak_ptr:
 *    - Non-owning reference to shared_ptr
 *    - Breaks circular references
 *    - Must lock() before use (may return nullptr)
 *    - Use for: observers, caches, parent-child relationships
 *
 * 4. Custom Deleters:
 *    - Critical for managing non-memory resources
 *    - Network sockets, file handles, locks
 *    - Lambda or function object
 *    - No overhead for shared_ptr, small overhead for unique_ptr
 *
 * 5. Performance Considerations (HFT Critical):
 *    - unique_ptr: ~0ns overhead vs raw pointer
 *    - shared_ptr copy: ~20-50ns (atomic operations)
 *    - shared_ptr move: ~5-10ns (no atomic ops)
 *    - make_shared: 1 allocation vs 2 (better cache locality)
 *
 * 6. Common Interview Questions:
 *    Q: Why use make_shared over shared_ptr constructor?
 *    A: Single allocation, exception safety, better cache locality
 *
 *    Q: Can you have circular shared_ptr references?
 *    A: Yes, causes memory leak. Use weak_ptr to break cycle.
 *
 *    Q: When to use unique_ptr vs shared_ptr in HFT?
 *    A: Prefer unique_ptr for ownership transfer (zero overhead).
 *       Use shared_ptr only when truly need shared ownership.
 *
 *    Q: Thread safety of shared_ptr?
 *    A: Reference counting is thread-safe (atomic).
 *       Object access is NOT thread-safe (need external sync).
 *
 * 7. Memory Layout:
 *    unique_ptr<T>: sizeof(T*)
 *    shared_ptr<T>: 2 * sizeof(T*) [data ptr + control block ptr]
 *    Control block: ref_count + weak_count + deleter + allocator
 *
 * 8. Best Practices for HFT:
 *    - Prefer unique_ptr by default (zero overhead)
 *    - Use make_unique/make_shared for exception safety
 *    - Avoid shared_ptr in hot paths if possible
 *    - Use custom allocators with shared_ptr for performance
 *    - Profile atomic operations on shared_ptr in critical paths
 *
 * Time Complexity:
 * - unique_ptr operations: O(1)
 * - shared_ptr copy: O(1) but with atomic overhead
 * - shared_ptr destructor: O(1) but may delete object
 *
 * Space Complexity:
 * - unique_ptr: sizeof(T*)
 * - shared_ptr: 2 * sizeof(T*) + control block
 */

int main() {
    demonstrate_smart_pointers();
    return 0;
}

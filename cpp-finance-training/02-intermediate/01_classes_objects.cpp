/*
 * Classes and Objects - Object-Oriented Programming Basics
 * HFT/Finance Context: Modeling financial instruments and orders
 */

#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>

// Simple class: Stock
class Stock {
private:
    // Private member variables (data hiding)
    std::string symbol_;
    double price_;
    int volume_;

public:
    // Default constructor
    Stock() : symbol_(""), price_(0.0), volume_(0) {
        std::cout << "Stock default constructor called" << std::endl;
    }

    // Parameterized constructor
    Stock(const std::string& symbol, double price, int volume)
        : symbol_(symbol), price_(price), volume_(volume) {
        std::cout << "Stock parameterized constructor called for " << symbol_ << std::endl;
    }

    // Copy constructor
    Stock(const Stock& other)
        : symbol_(other.symbol_), price_(other.price_), volume_(other.volume_) {
        std::cout << "Stock copy constructor called for " << symbol_ << std::endl;
    }

    // Destructor
    ~Stock() {
        std::cout << "Stock destructor called for " << symbol_ << std::endl;
    }

    // Getter methods (accessors)
    std::string getSymbol() const { return symbol_; }
    double getPrice() const { return price_; }
    int getVolume() const { return volume_; }

    // Setter methods (mutators)
    void setPrice(double price) {
        if (price >= 0) {
            price_ = price;
        }
    }

    void setVolume(int volume) {
        if (volume >= 0) {
            volume_ = volume;
        }
    }

    // Member function
    double getMarketCap(int shares_outstanding) const {
        return price_ * shares_outstanding;
    }

    // Display method
    void display() const {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Symbol: " << symbol_
                  << ", Price: $" << price_
                  << ", Volume: " << volume_ << std::endl;
    }
};

// More complex class: Order
class Order {
private:
    uint64_t order_id_;
    std::string symbol_;
    char side_;  // 'B' or 'S'
    int quantity_;
    double price_;
    bool is_filled_;

    static uint64_t next_order_id_;  // Static member for ID generation

public:
    // Constructor with member initializer list
    Order(const std::string& symbol, char side, int quantity, double price)
        : order_id_(next_order_id_++),
          symbol_(symbol),
          side_(side),
          quantity_(quantity),
          price_(price),
          is_filled_(false) {
    }

    // Static method
    static uint64_t getNextOrderId() {
        return next_order_id_;
    }

    // Const member function (doesn't modify object)
    void display() const {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Order #" << order_id_ << ": "
                  << (side_ == 'B' ? "BUY" : "SELL") << " "
                  << quantity_ << " " << symbol_
                  << " @ $" << price_
                  << " [" << (is_filled_ ? "FILLED" : "OPEN") << "]" << std::endl;
    }

    // Non-const member function (can modify object)
    void fill() {
        is_filled_ = true;
        std::cout << "Order #" << order_id_ << " filled!" << std::endl;
    }

    double getValue() const {
        return quantity_ * price_;
    }

    // Friend function declaration (can access private members)
    friend bool matchOrders(const Order& buy, const Order& sell);
};

// Static member initialization
uint64_t Order::next_order_id_ = 1000;

// Friend function implementation
bool matchOrders(const Order& buy, const Order& sell) {
    return buy.symbol_ == sell.symbol_ &&
           buy.side_ == 'B' &&
           sell.side_ == 'S' &&
           buy.price_ >= sell.price_;
}

// Class with composition: Portfolio
class Portfolio {
private:
    std::string owner_;
    Stock* holdings_;  // Composition: Portfolio contains Stocks
    int capacity_;
    int count_;

public:
    // Constructor
    Portfolio(const std::string& owner, int capacity)
        : owner_(owner), capacity_(capacity), count_(0) {
        holdings_ = new Stock[capacity_];
        std::cout << "Portfolio created for " << owner_ << std::endl;
    }

    // Destructor (must clean up dynamic memory)
    ~Portfolio() {
        delete[] holdings_;
        std::cout << "Portfolio destroyed for " << owner_ << std::endl;
    }

    // Add stock to portfolio
    void addStock(const Stock& stock) {
        if (count_ < capacity_) {
            holdings_[count_++] = stock;
            std::cout << "Added " << stock.getSymbol() << " to portfolio" << std::endl;
        } else {
            std::cout << "Portfolio is full!" << std::endl;
        }
    }

    // Display portfolio
    void display() const {
        std::cout << "\nPortfolio for " << owner_ << ":" << std::endl;
        std::cout << "Holdings (" << count_ << "/" << capacity_ << "):" << std::endl;
        for (int i = 0; i < count_; ++i) {
            std::cout << "  ";
            holdings_[i].display();
        }
    }

    // Calculate total value
    double getTotalValue() const {
        double total = 0.0;
        for (int i = 0; i < count_; ++i) {
            // Simplified: assumes 100 shares of each
            total += holdings_[i].getPrice() * 100;
        }
        return total;
    }
};

// Class with constructor delegation (C++11)
class Trade {
private:
    std::string symbol_;
    double price_;
    int quantity_;
    std::string timestamp_;

public:
    // Main constructor
    Trade(const std::string& symbol, double price, int quantity, const std::string& timestamp)
        : symbol_(symbol), price_(price), quantity_(quantity), timestamp_(timestamp) {
    }

    // Delegating constructor (uses default timestamp)
    Trade(const std::string& symbol, double price, int quantity)
        : Trade(symbol, price, quantity, "2024-01-15T10:00:00") {
        // Delegates to main constructor
    }

    void display() const {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << timestamp_ << " | " << symbol_ << " | "
                  << quantity_ << " @ $" << price_ << std::endl;
    }
};

// Struct (like class but default public)
struct Tick {
    std::string symbol;
    double price;
    int volume;
    long long timestamp_ns;

    // Structs can have methods too
    void display() const {
        std::cout << symbol << ": $" << price
                  << " vol=" << volume << std::endl;
    }
};

int main() {
    std::cout << "=== Classes and Objects ===" << std::endl << std::endl;

    // Creating objects
    std::cout << "--- Creating Stock Objects ---" << std::endl;
    Stock apple("AAPL", 150.25, 1000000);
    Stock microsoft("MSFT", 375.50, 500000);
    apple.display();
    microsoft.display();
    std::cout << std::endl;

    // Using methods
    std::cout << "--- Using Methods ---" << std::endl;
    apple.setPrice(151.50);
    std::cout << "Updated Apple price: $" << apple.getPrice() << std::endl;

    double market_cap = apple.getMarketCap(16500000000);  // ~16.5B shares
    std::cout << "Apple Market Cap: $" << (market_cap / 1e12) << "T" << std::endl;
    std::cout << std::endl;

    // Copy constructor
    std::cout << "--- Copy Constructor ---" << std::endl;
    Stock apple_copy = apple;  // Calls copy constructor
    apple_copy.display();
    std::cout << std::endl;

    // Order class with static members
    std::cout << "--- Order Class ---" << std::endl;
    Order buy_order("AAPL", 'B', 100, 150.25);
    Order sell_order("AAPL", 'S', 100, 150.30);

    buy_order.display();
    sell_order.display();

    std::cout << "Next Order ID: " << Order::getNextOrderId() << std::endl;
    std::cout << std::endl;

    // Friend function
    std::cout << "--- Friend Function (Order Matching) ---" << std::endl;
    if (matchOrders(buy_order, sell_order)) {
        std::cout << "Orders match! Executing trade..." << std::endl;
        buy_order.fill();
        sell_order.fill();
    } else {
        std::cout << "Orders do not match" << std::endl;
    }
    std::cout << std::endl;

    // Portfolio (composition)
    std::cout << "--- Portfolio (Composition) ---" << std::endl;
    Portfolio myPortfolio("John Doe", 5);
    myPortfolio.addStock(Stock("AAPL", 150.25, 0));
    myPortfolio.addStock(Stock("MSFT", 375.50, 0));
    myPortfolio.addStock(Stock("GOOGL", 140.75, 0));
    myPortfolio.display();
    std::cout << "Total Value: $" << myPortfolio.getTotalValue() << std::endl;
    std::cout << std::endl;

    // Constructor delegation
    std::cout << "--- Constructor Delegation ---" << std::endl;
    Trade trade1("AAPL", 150.25, 100, "2024-01-15T09:30:00");
    Trade trade2("MSFT", 375.50, 200);  // Uses default timestamp

    trade1.display();
    trade2.display();
    std::cout << std::endl;

    // Struct
    std::cout << "--- Struct ---" << std::endl;
    Tick tick{"AAPL", 150.25, 500, 1705318800000000000LL};
    tick.display();
    std::cout << std::endl;

    // Object array
    std::cout << "--- Array of Objects ---" << std::endl;
    Stock stocks[3] = {
        Stock("AAPL", 150.25, 1000000),
        Stock("MSFT", 375.50, 500000),
        Stock("GOOGL", 140.75, 750000)
    };

    std::cout << "Stock array:" << std::endl;
    for (int i = 0; i < 3; ++i) {
        std::cout << "  ";
        stocks[i].display();
    }

    std::cout << "\n--- Destructors Called at End of Scope ---" << std::endl;
    return 0;
}

/*
 * Key Takeaways:
 * 1. Classes encapsulate data (members) and behavior (methods)
 * 2. Constructor initializer lists are more efficient than assignment
 * 3. Destructors clean up resources (RAII principle)
 * 4. const methods promise not to modify object state
 * 5. Static members belong to class, not individual objects
 * 6. Friend functions can access private members (use sparingly)
 * 7. Composition: objects can contain other objects
 * 8. Structs are like classes but default to public access
 * 9. Copy constructors are called when objects are copied
 * 10. Constructor delegation (C++11) reduces code duplication
 *
 * Best Practices for HFT:
 * - Keep objects small and cache-friendly
 * - Minimize virtual functions in hot paths
 * - Use composition over inheritance for flexibility
 * - Mark methods const when possible for compiler optimizations
 * - Consider struct-of-arrays over array-of-structs for SIMD
 */

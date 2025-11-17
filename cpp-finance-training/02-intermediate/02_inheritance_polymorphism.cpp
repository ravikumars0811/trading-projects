/*
 * Inheritance and Polymorphism
 * HFT/Finance Context: Financial instrument hierarchy, strategy patterns
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <iomanip>

// Base class: Financial Instrument
class Instrument {
protected:
    std::string symbol_;
    double price_;

public:
    Instrument(const std::string& symbol, double price)
        : symbol_(symbol), price_(price) {
    }

    // Virtual destructor (important for polymorphism!)
    virtual ~Instrument() {
        std::cout << "~Instrument() for " << symbol_ << std::endl;
    }

    // Virtual function (can be overridden)
    virtual void display() const {
        std::cout << "Instrument: " << symbol_ << " @ $" << price_;
    }

    // Pure virtual function (must be overridden - makes class abstract)
    virtual double calculateValue(int quantity) const = 0;

    // Pure virtual function
    virtual std::string getType() const = 0;

    // Non-virtual function
    std::string getSymbol() const { return symbol_; }
    double getPrice() const { return price_; }
};

// Derived class: Stock
class Stock : public Instrument {
private:
    double dividend_yield_;

public:
    Stock(const std::string& symbol, double price, double dividend_yield = 0.0)
        : Instrument(symbol, price), dividend_yield_(dividend_yield) {
    }

    ~Stock() override {
        std::cout << "~Stock() for " << symbol_ << std::endl;
    }

    // Override virtual function
    void display() const override {
        Instrument::display();  // Call base class version
        std::cout << ", Div Yield: " << (dividend_yield_ * 100) << "%" << std::endl;
    }

    // Implement pure virtual function
    double calculateValue(int quantity) const override {
        return price_ * quantity;
    }

    std::string getType() const override {
        return "Stock";
    }

    double getDividendYield() const { return dividend_yield_; }
};

// Derived class: Option
class Option : public Instrument {
protected:
    double strike_;
    double time_to_expiry_;  // In years
    char type_;  // 'C' for Call, 'P' for Put

public:
    Option(const std::string& symbol, double price, double strike,
           double time_to_expiry, char type)
        : Instrument(symbol, price), strike_(strike),
          time_to_expiry_(time_to_expiry), type_(type) {
    }

    ~Option() override {
        std::cout << "~Option() for " << symbol_ << std::endl;
    }

    void display() const override {
        Instrument::display();
        std::cout << ", Strike: $" << strike_
                  << ", Type: " << (type_ == 'C' ? "Call" : "Put")
                  << ", TTM: " << time_to_expiry_ << " years" << std::endl;
    }

    double calculateValue(int quantity) const override {
        // Option value = premium * quantity * 100 (contract multiplier)
        return price_ * quantity * 100;
    }

    std::string getType() const override {
        return type_ == 'C' ? "Call Option" : "Put Option";
    }

    double getStrike() const { return strike_; }
};

// Further derived class: European Option
class EuropeanOption : public Option {
private:
    double volatility_;
    double risk_free_rate_;

public:
    EuropeanOption(const std::string& symbol, double underlying_price,
                   double strike, double time_to_expiry, char type,
                   double volatility, double risk_free_rate)
        : Option(symbol, 0.0, strike, time_to_expiry, type),
          volatility_(volatility), risk_free_rate_(risk_free_rate) {
        // Calculate Black-Scholes price
        price_ = calculateBlackScholesPrice(underlying_price);
    }

    void display() const override {
        Option::display();
        std::cout << "    Volatility: " << (volatility_ * 100) << "%"
                  << ", Risk-Free Rate: " << (risk_free_rate_ * 100) << "%" << std::endl;
    }

private:
    double normalCDF(double x) const {
        return 0.5 * std::erfc(-x * M_SQRT1_2);
    }

    double calculateBlackScholesPrice(double S) const {
        double d1 = (std::log(S / strike_) + (risk_free_rate_ + 0.5 * volatility_ * volatility_) * time_to_expiry_)
                    / (volatility_ * std::sqrt(time_to_expiry_));
        double d2 = d1 - volatility_ * std::sqrt(time_to_expiry_);

        if (type_ == 'C') {
            return S * normalCDF(d1) - strike_ * std::exp(-risk_free_rate_ * time_to_expiry_) * normalCDF(d2);
        } else {
            return strike_ * std::exp(-risk_free_rate_ * time_to_expiry_) * normalCDF(-d2) - S * normalCDF(-d1);
        }
    }
};

// Abstract base class: Trading Strategy
class TradingStrategy {
protected:
    std::string name_;

public:
    TradingStrategy(const std::string& name) : name_(name) {}
    virtual ~TradingStrategy() = default;

    // Pure virtual: each strategy implements its own signal logic
    virtual bool generateSignal(double price, double moving_avg) const = 0;

    std::string getName() const { return name_; }
};

// Concrete strategy: Moving Average Crossover
class MACrossover : public TradingStrategy {
private:
    double threshold_;

public:
    MACrossover(double threshold = 0.0)
        : TradingStrategy("MA Crossover"), threshold_(threshold) {
    }

    bool generateSignal(double price, double moving_avg) const override {
        double diff = price - moving_avg;
        return diff > threshold_;  // Buy signal if price > MA + threshold
    }
};

// Concrete strategy: Mean Reversion
class MeanReversion : public TradingStrategy {
private:
    double std_dev_threshold_;

public:
    MeanReversion(double std_dev_threshold = 2.0)
        : TradingStrategy("Mean Reversion"), std_dev_threshold_(std_dev_threshold) {
    }

    bool generateSignal(double price, double moving_avg) const override {
        // Simplified: buy when price is below MA (mean reversion)
        return price < moving_avg;
    }
};

// Function using polymorphism
void analyzeInstrument(const Instrument& instrument, int quantity) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nAnalyzing " << instrument.getType() << ":" << std::endl;
    instrument.display();
    std::cout << "Value for " << quantity << " units: $"
              << instrument.calculateValue(quantity) << std::endl;
}

// Function demonstrating polymorphism with strategy pattern
void testStrategy(const TradingStrategy& strategy, double price, double ma) {
    std::cout << "Strategy: " << strategy.getName() << std::endl;
    std::cout << "  Price: $" << price << ", MA: $" << ma << std::endl;
    std::cout << "  Signal: " << (strategy.generateSignal(price, ma) ? "BUY" : "HOLD/SELL")
              << std::endl << std::endl;
}

int main() {
    std::cout << "=== Inheritance and Polymorphism ===" << std::endl << std::endl;

    // Creating derived objects
    std::cout << "--- Creating Derived Objects ---" << std::endl;
    Stock apple("AAPL", 150.25, 0.005);  // 0.5% dividend yield
    Option apple_call("AAPL250117C00150000", 5.50, 150.0, 0.25, 'C');

    apple.display();
    apple_call.display();
    std::cout << std::endl;

    // Polymorphism with references
    std::cout << "--- Polymorphism with References ---" << std::endl;
    analyzeInstrument(apple, 100);
    analyzeInstrument(apple_call, 10);
    std::cout << std::endl;

    // Polymorphism with pointers
    std::cout << "--- Polymorphism with Pointers ---" << std::endl;
    Instrument* instruments[3];
    instruments[0] = new Stock("MSFT", 375.50, 0.008);
    instruments[1] = new Option("MSFT250117C00380000", 8.25, 380.0, 0.25, 'C');
    instruments[2] = new Stock("GOOGL", 140.75, 0.0);

    for (int i = 0; i < 3; ++i) {
        std::cout << "Instrument " << i << ": ";
        instruments[i]->display();
    }

    // Clean up
    for (int i = 0; i < 3; ++i) {
        delete instruments[i];
    }
    std::cout << std::endl;

    // Polymorphism with smart pointers (C++11)
    std::cout << "--- Polymorphism with Smart Pointers ---" << std::endl;
    std::vector<std::unique_ptr<Instrument>> portfolio;

    portfolio.push_back(std::make_unique<Stock>("AAPL", 150.25, 0.005));
    portfolio.push_back(std::make_unique<Option>("AAPL250117C00150000", 5.50, 150.0, 0.25, 'C'));
    portfolio.push_back(std::make_unique<Stock>("TSLA", 242.15, 0.0));

    double total_value = 0.0;
    for (const auto& instrument : portfolio) {
        instrument->display();
        total_value += instrument->calculateValue(100);
    }
    std::cout << "Total Portfolio Value (100 units each): $" << total_value << std::endl;
    std::cout << std::endl;

    // Multi-level inheritance
    std::cout << "--- Multi-Level Inheritance ---" << std::endl;
    EuropeanOption euro_call("AAPL", 150.25, 150.0, 0.25, 'C', 0.20, 0.05);
    euro_call.display();
    std::cout << std::endl;

    // Strategy pattern (polymorphism for behavior)
    std::cout << "--- Strategy Pattern ---" << std::endl;
    MACrossover ma_strategy(0.5);
    MeanReversion mr_strategy(2.0);

    double current_price = 150.50;
    double moving_average = 149.80;

    testStrategy(ma_strategy, current_price, moving_average);
    testStrategy(mr_strategy, current_price, moving_average);

    // Virtual function table demonstration
    std::cout << "--- Virtual Function Overhead ---" << std::endl;
    std::cout << "Size of Stock object: " << sizeof(Stock) << " bytes" << std::endl;
    std::cout << "Size of Option object: " << sizeof(Option) << " bytes" << std::endl;
    std::cout << "(Includes vptr for virtual function table)" << std::endl;
    std::cout << std::endl;

    // Dynamic casting
    std::cout << "--- Dynamic Casting ---" << std::endl;
    Instrument* inst = new Stock("NVDA", 495.50, 0.001);

    // Try to downcast
    Stock* stock_ptr = dynamic_cast<Stock*>(inst);
    if (stock_ptr != nullptr) {
        std::cout << "Successfully cast to Stock*" << std::endl;
        std::cout << "Dividend Yield: " << (stock_ptr->getDividendYield() * 100) << "%" << std::endl;
    }

    Option* option_ptr = dynamic_cast<Option*>(inst);
    if (option_ptr == nullptr) {
        std::cout << "Cannot cast Stock to Option (correct!)" << std::endl;
    }

    delete inst;

    return 0;
}

/*
 * Key Takeaways:
 * 1. Inheritance models "is-a" relationships (Stock IS-A Instrument)
 * 2. Virtual functions enable runtime polymorphism
 * 3. Pure virtual functions (= 0) make classes abstract
 * 4. Override keyword (C++11) prevents errors
 * 5. Virtual destructor is essential for polymorphic classes
 * 6. Base class pointers can point to derived objects
 * 7. Dynamic dispatch has small runtime cost (vtable lookup)
 * 8. Use final keyword to prevent further overriding
 * 9. Strategy pattern separates algorithms from objects
 * 10. Smart pointers work seamlessly with polymorphism
 *
 * HFT Considerations:
 * - Virtual functions add overhead (~1 indirection)
 * - Avoid virtual calls in ultra-hot paths if possible
 * - Consider CRTP (Curiously Recurring Template Pattern) for compile-time polymorphism
 * - Template-based polymorphism has zero runtime cost
 * - Profile before optimizing - sometimes virtual overhead is negligible
 */

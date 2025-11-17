/*
 * Move Semantics and Rvalue References (C++11)
 * HFT/Finance Context: Zero-copy transfer of large market data buffers
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstring>

// Market data buffer class demonstrating move semantics
class MarketDataBuffer {
private:
    double* data_;
    size_t size_;
    std::string symbol_;

public:
    // Constructor
    MarketDataBuffer(const std::string& symbol, size_t size)
        : data_(new double[size]), size_(size), symbol_(symbol) {
        std::cout << "  Constructor: Allocated " << size_ << " doubles for "
                  << symbol_ << std::endl;
    }

    // Destructor
    ~MarketDataBuffer() {
        if (data_ != nullptr) {
            std::cout << "  Destructor: Freeing buffer for " << symbol_ << std::endl;
            delete[] data_;
        }
    }

    // Copy constructor (expensive!)
    MarketDataBuffer(const MarketDataBuffer& other)
        : data_(new double[other.size_]), size_(other.size_), symbol_(other.symbol_) {
        std::cout << "  COPY Constructor: Deep copy of " << size_ << " doubles for "
                  << symbol_ << " [EXPENSIVE!]" << std::endl;
        std::memcpy(data_, other.data_, size_ * sizeof(double));
    }

    // Copy assignment (expensive!)
    MarketDataBuffer& operator=(const MarketDataBuffer& other) {
        std::cout << "  COPY Assignment for " << other.symbol_ << " [EXPENSIVE!]" << std::endl;
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            symbol_ = other.symbol_;
            data_ = new double[size_];
            std::memcpy(data_, other.data_, size_ * sizeof(double));
        }
        return *this;
    }

    // Move constructor (efficient!)
    MarketDataBuffer(MarketDataBuffer&& other) noexcept
        : data_(other.data_), size_(other.size_), symbol_(std::move(other.symbol_)) {
        std::cout << "  MOVE Constructor: Stealing buffer from " << symbol_
                  << " [FAST!]" << std::endl;
        // Leave other in valid but unspecified state
        other.data_ = nullptr;
        other.size_ = 0;
    }

    // Move assignment (efficient!)
    MarketDataBuffer& operator=(MarketDataBuffer&& other) noexcept {
        std::cout << "  MOVE Assignment from " << other.symbol_ << " [FAST!]" << std::endl;
        if (this != &other) {
            delete[] data_;  // Clean up our current resources

            // Steal other's resources
            data_ = other.data_;
            size_ = other.size_;
            symbol_ = std::move(other.symbol_);

            // Leave other in valid state
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    void fillWithSampleData() {
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = 150.0 + (i * 0.01);
        }
    }

    double* getData() { return data_; }
    size_t getSize() const { return size_; }
    const std::string& getSymbol() const { return symbol_; }
};

// Function returning by value (move-enabled)
MarketDataBuffer createBuffer(const std::string& symbol, size_t size) {
    MarketDataBuffer buffer(symbol, size);
    buffer.fillWithSampleData();
    return buffer;  // Move constructor called (RVO may eliminate this)
}

// Lvalue vs Rvalue demonstration
void processData(const MarketDataBuffer& data) {
    std::cout << "Processing lvalue reference (const)" << std::endl;
}

void processData(MarketDataBuffer&& data) {
    std::cout << "Processing rvalue reference (temporary)" << std::endl;
    // Can safely steal resources from 'data'
}

// Perfect forwarding template
template<typename T>
void forwardToProcess(T&& arg) {
    processData(std::forward<T>(arg));  // Perfect forwarding preserves value category
}

// Demonstrate std::move
void demonstrateStdMove() {
    std::cout << "\n--- std::move Demonstration ---" << std::endl;

    MarketDataBuffer buffer1("AAPL", 1000);
    buffer1.fillWithSampleData();

    std::cout << "\nMoving buffer1 to buffer2..." << std::endl;
    MarketDataBuffer buffer2 = std::move(buffer1);  // Explicit move

    std::cout << "buffer1 size after move: " << buffer1.getSize() << std::endl;
    std::cout << "buffer2 size after move: " << buffer2.getSize() << std::endl;
}

// Vector with move semantics
void demonstrateVectorMove() {
    std::cout << "\n--- Vector with Move Semantics ---" << std::endl;

    std::vector<MarketDataBuffer> buffers;
    buffers.reserve(3);  // Prevent reallocations

    std::cout << "\nPushing back temporary (rvalue):" << std::endl;
    buffers.push_back(MarketDataBuffer("AAPL", 500));  // Move constructor

    std::cout << "\nPushing back lvalue with std::move:" << std::endl;
    MarketDataBuffer msft_buffer("MSFT", 500);
    buffers.push_back(std::move(msft_buffer));  // Explicit move

    std::cout << "\nEmplacing (construct in-place):" << std::endl;
    buffers.emplace_back("GOOGL", 500);  // Construct directly in vector
}

// Performance comparison: Copy vs Move
void performanceComparison() {
    std::cout << "\n--- Performance Comparison ---" << std::endl;

    const size_t LARGE_SIZE = 1000000;  // 1M doubles

    // Copy operation
    auto start = std::chrono::high_resolution_clock::now();
    {
        MarketDataBuffer original("TEST", LARGE_SIZE);
        original.fillWithSampleData();
        MarketDataBuffer copy = original;  // Copy constructor
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto copy_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Move operation
    start = std::chrono::high_resolution_clock::now();
    {
        MarketDataBuffer original("TEST", LARGE_SIZE);
        original.fillWithSampleData();
        MarketDataBuffer moved = std::move(original);  // Move constructor
    }
    end = std::chrono::high_resolution_clock::now();
    auto move_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "\nCopy time: " << copy_time.count() << " μs" << std::endl;
    std::cout << "Move time: " << move_time.count() << " μs" << std::endl;
    std::cout << "Speedup: " << (copy_time.count() / (double)move_time.count()) << "x" << std::endl;
}

// String move semantics (built-in)
void stringMoveExample() {
    std::cout << "\n--- String Move Semantics ---" << std::endl;

    std::string long_string(1000000, 'A');  // 1M characters

    std::cout << "Original string address: " << (void*)long_string.data() << std::endl;
    std::cout << "Original string size: " << long_string.size() << std::endl;

    std::string moved_string = std::move(long_string);

    std::cout << "Moved string address: " << (void*)moved_string.data() << std::endl;
    std::cout << "Moved string size: " << moved_string.size() << std::endl;
    std::cout << "Original string size after move: " << long_string.size() << std::endl;
    std::cout << "(Same address = no copy, just pointer transfer!)" << std::endl;
}

int main() {
    std::cout << "=== Move Semantics and Rvalue References ===" << std::endl << std::endl;

    // Basic move constructor
    std::cout << "--- Move Constructor ---" << std::endl;
    MarketDataBuffer buffer = createBuffer("AAPL", 1000);
    std::cout << "Returned buffer size: " << buffer.getSize() << std::endl;

    // Lvalue vs Rvalue
    std::cout << "\n--- Lvalue vs Rvalue ---" << std::endl;
    MarketDataBuffer lvalue_buffer("MSFT", 100);
    processData(lvalue_buffer);  // Lvalue
    processData(MarketDataBuffer("GOOGL", 100));  // Rvalue

    // Perfect forwarding
    std::cout << "\n--- Perfect Forwarding ---" << std::endl;
    MarketDataBuffer forwarded("AMZN", 100);
    forwardToProcess(forwarded);  // Forwards as lvalue
    forwardToProcess(MarketDataBuffer("TSLA", 100));  // Forwards as rvalue

    // std::move
    demonstrateStdMove();

    // Vector with move
    demonstrateVectorMove();

    // String move
    stringMoveExample();

    // Performance comparison
    performanceComparison();

    std::cout << "\n--- Rule of Five ---" << std::endl;
    std::cout << "If you define ANY of these, define ALL:" << std::endl;
    std::cout << "1. Destructor" << std::endl;
    std::cout << "2. Copy constructor" << std::endl;
    std::cout << "3. Copy assignment operator" << std::endl;
    std::cout << "4. Move constructor" << std::endl;
    std::cout << "5. Move assignment operator" << std::endl;
    std::cout << "\nOr use = default / = delete" << std::endl;

    return 0;
}

/*
 * Key Takeaways:
 * 1. Lvalues have names and addresses, rvalues are temporaries
 * 2. Move semantics transfer resources instead of copying
 * 3. Move constructor/assignment take rvalue references (&&)
 * 4. std::move converts lvalue to rvalue (cast)
 * 5. Move operations should be noexcept for std::vector optimization
 * 6. RVO (Return Value Optimization) may eliminate moves entirely
 * 7. std::forward preserves value category (perfect forwarding)
 * 8. Always leave moved-from objects in valid state
 * 9. Rule of Five: define all 5 special members or none
 * 10. Move semantics are critical for performance in C++11+
 *
 * HFT Implications:
 * - Move large buffers instead of copying (10-1000x faster)
 * - Return large objects by value safely
 * - Use emplace_back instead of push_back for in-place construction
 * - Reserve vector capacity to avoid moves during growth
 * - Move-only types (unique_ptr) enforce single ownership
 * - Critical for zero-copy message passing
 */

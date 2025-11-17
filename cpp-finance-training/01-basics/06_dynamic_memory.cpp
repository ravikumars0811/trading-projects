/*
 * Dynamic Memory Allocation
 * HFT/Finance Context: Allocating buffers for market data, dynamic order books
 */

#include <iostream>
#include <cstring>
#include <iomanip>

struct Order {
    uint64_t order_id;
    char symbol[8];
    double price;
    int quantity;
    char side;
};

// Create dynamic array of prices
double* createPriceBuffer(int size) {
    double* buffer = new double[size];  // Allocate on heap
    std::cout << "Allocated buffer for " << size << " prices" << std::endl;
    return buffer;
}

// Deallocate dynamic array
void destroyPriceBuffer(double* buffer) {
    delete[] buffer;  // Must use delete[] for arrays
    std::cout << "Deallocated price buffer" << std::endl;
}

// Create single order dynamically
Order* createOrder(uint64_t id, const char* symbol, double price, int qty, char side) {
    Order* order = new Order;  // Allocate single object

    order->order_id = id;
    strncpy(order->symbol, symbol, sizeof(order->symbol) - 1);
    order->symbol[sizeof(order->symbol) - 1] = '\0';
    order->price = price;
    order->quantity = qty;
    order->side = side;

    return order;
}

// Create dynamic order book
Order** createOrderBook(int max_orders) {
    Order** order_book = new Order*[max_orders];  // Array of pointers

    for (int i = 0; i < max_orders; ++i) {
        order_book[i] = nullptr;  // Initialize to null
    }

    std::cout << "Created order book with capacity: " << max_orders << std::endl;
    return order_book;
}

// Destroy order book
void destroyOrderBook(Order** order_book, int max_orders) {
    for (int i = 0; i < max_orders; ++i) {
        if (order_book[i] != nullptr) {
            delete order_book[i];  // Delete each order
        }
    }
    delete[] order_book;  // Delete array of pointers
    std::cout << "Destroyed order book" << std::endl;
}

// Resize dynamic array (simplified - not efficient)
double* resizeBuffer(double* old_buffer, int old_size, int new_size) {
    double* new_buffer = new double[new_size];

    // Copy old data
    int copy_size = (old_size < new_size) ? old_size : new_size;
    for (int i = 0; i < copy_size; ++i) {
        new_buffer[i] = old_buffer[i];
    }

    // Initialize remaining elements
    for (int i = copy_size; i < new_size; ++i) {
        new_buffer[i] = 0.0;
    }

    delete[] old_buffer;  // Free old buffer
    return new_buffer;
}

// Demonstrate memory leaks (DON'T DO THIS!)
void demonstrateMemoryLeak() {
    std::cout << "\n⚠️  Demonstrating memory leak (educational only):" << std::endl;

    double* leaked = new double[1000];
    // ... use buffer ...
    // Oops! Forgot to delete - MEMORY LEAK!
    std::cout << "Allocated 1000 doubles but didn't delete - LEAKED!" << std::endl;

    // Proper way:
    double* proper = new double[1000];
    // ... use buffer ...
    delete[] proper;  // ✓ Properly deallocated
    std::cout << "Allocated 1000 doubles and properly deleted - OK!" << std::endl;
}

// 2D dynamic array
double** create2DArray(int rows, int cols) {
    double** array = new double*[rows];

    for (int i = 0; i < rows; ++i) {
        array[i] = new double[cols];
        // Initialize to zero
        for (int j = 0; j < cols; ++j) {
            array[i][j] = 0.0;
        }
    }

    return array;
}

void destroy2DArray(double** array, int rows) {
    for (int i = 0; i < rows; ++i) {
        delete[] array[i];
    }
    delete[] array;
}

int main() {
    std::cout << "=== Dynamic Memory Allocation ===" << std::endl << std::endl;

    // Basic new/delete
    std::cout << "--- Basic new/delete ---" << std::endl;
    double* price_ptr = new double;  // Allocate single double
    *price_ptr = 150.25;
    std::cout << "Dynamically allocated price: $" << *price_ptr << std::endl;
    delete price_ptr;  // Deallocate
    price_ptr = nullptr;  // Good practice: set to nullptr after delete
    std::cout << std::endl;

    // Dynamic array allocation
    std::cout << "--- Dynamic Array ---" << std::endl;
    int buffer_size = 5;
    double* prices = createPriceBuffer(buffer_size);

    // Initialize prices
    prices[0] = 150.25;
    prices[1] = 150.30;
    prices[2] = 150.28;
    prices[3] = 150.35;
    prices[4] = 150.32;

    std::cout << "Prices in buffer:" << std::endl;
    for (int i = 0; i < buffer_size; ++i) {
        std::cout << "  [" << i << "] $" << prices[i] << std::endl;
    }

    destroyPriceBuffer(prices);
    prices = nullptr;
    std::cout << std::endl;

    // Single object allocation
    std::cout << "--- Single Object Allocation ---" << std::endl;
    Order* order1 = createOrder(12345, "AAPL", 150.25, 100, 'B');

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Order created:" << std::endl;
    std::cout << "  ID: " << order1->order_id << std::endl;
    std::cout << "  Symbol: " << order1->symbol << std::endl;
    std::cout << "  Price: $" << order1->price << std::endl;
    std::cout << "  Quantity: " << order1->quantity << std::endl;
    std::cout << "  Side: " << order1->side << std::endl;

    delete order1;
    order1 = nullptr;
    std::cout << std::endl;

    // Order book (array of pointers)
    std::cout << "--- Order Book (Array of Pointers) ---" << std::endl;
    int max_orders = 10;
    Order** order_book = createOrderBook(max_orders);

    // Add some orders
    order_book[0] = createOrder(1001, "AAPL", 150.25, 100, 'B');
    order_book[1] = createOrder(1002, "MSFT", 375.50, 200, 'S');
    order_book[2] = createOrder(1003, "GOOGL", 140.75, 150, 'B');

    std::cout << "Orders in book:" << std::endl;
    for (int i = 0; i < max_orders; ++i) {
        if (order_book[i] != nullptr) {
            std::cout << "  [" << i << "] " << order_book[i]->symbol
                      << " " << order_book[i]->side
                      << " " << order_book[i]->quantity
                      << " @ $" << order_book[i]->price << std::endl;
        }
    }

    destroyOrderBook(order_book, max_orders);
    order_book = nullptr;
    std::cout << std::endl;

    // Resizing dynamic array
    std::cout << "--- Resizing Dynamic Array ---" << std::endl;
    int initial_size = 3;
    double* buffer = new double[initial_size]{150.25, 150.30, 150.28};

    std::cout << "Initial buffer (size " << initial_size << "):" << std::endl;
    for (int i = 0; i < initial_size; ++i) {
        std::cout << "  [" << i << "] $" << buffer[i] << std::endl;
    }

    int new_size = 5;
    buffer = resizeBuffer(buffer, initial_size, new_size);

    std::cout << "After resize (size " << new_size << "):" << std::endl;
    for (int i = 0; i < new_size; ++i) {
        std::cout << "  [" << i << "] $" << buffer[i] << std::endl;
    }

    delete[] buffer;
    buffer = nullptr;
    std::cout << std::endl;

    // 2D dynamic array
    std::cout << "--- 2D Dynamic Array ---" << std::endl;
    int rows = 3, cols = 4;
    double** matrix = create2DArray(rows, cols);

    // Fill with sample data (correlation matrix)
    matrix[0][0] = 1.00; matrix[0][1] = 0.75; matrix[0][2] = 0.50; matrix[0][3] = 0.25;
    matrix[1][0] = 0.75; matrix[1][1] = 1.00; matrix[1][2] = 0.60; matrix[1][3] = 0.30;
    matrix[2][0] = 0.50; matrix[2][1] = 0.60; matrix[2][2] = 1.00; matrix[2][3] = 0.40;

    std::cout << "Correlation Matrix:" << std::endl;
    for (int i = 0; i < rows; ++i) {
        std::cout << "  ";
        for (int j = 0; j < cols; ++j) {
            std::cout << std::setw(6) << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }

    destroy2DArray(matrix, rows);
    matrix = nullptr;
    std::cout << std::endl;

    // Memory leak demonstration
    demonstrateMemoryLeak();
    std::cout << std::endl;

    // Stack vs Heap
    std::cout << "--- Stack vs Heap ---" << std::endl;
    double stack_price = 150.25;  // Stack allocation
    double* heap_price = new double(150.25);  // Heap allocation

    std::cout << "Stack variable address: " << &stack_price << std::endl;
    std::cout << "Heap variable address: " << heap_price << std::endl;
    std::cout << "\nStack: Fast, automatic cleanup, limited size" << std::endl;
    std::cout << "Heap: Slower, manual cleanup, large size" << std::endl;

    delete heap_price;

    return 0;
}

/*
 * Key Takeaways:
 * 1. new allocates memory on heap, delete frees it
 * 2. Use new[] and delete[] for arrays (matching pairs!)
 * 3. Always delete what you new to prevent memory leaks
 * 4. Set pointers to nullptr after delete
 * 5. Stack allocation is faster but limited in size
 * 6. Heap allocation is flexible but requires manual management
 * 7. 2D arrays require deleting each row, then the array of pointers
 * 8. Modern C++ prefers smart pointers (covered in advanced section)
 * 9. RAII pattern: allocate in constructor, deallocate in destructor
 * 10. Use tools like valgrind to detect memory leaks
 */

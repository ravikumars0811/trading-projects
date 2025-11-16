/*
 * Pointers and References
 * HFT/Finance Context: Efficient data access, memory management
 */

#include <iostream>
#include <iomanip>
#include <cstring>

struct Trade {
    char symbol[8];
    double price;
    int quantity;
    char side;  // 'B' or 'S'
};

// Pass by pointer - can modify original and handle nullptr
void updatePrice(double* price_ptr, double change) {
    if (price_ptr == nullptr) {
        std::cout << "Error: null pointer!" << std::endl;
        return;
    }
    *price_ptr += change;
    std::cout << "Updated price via pointer: $" << *price_ptr << std::endl;
}

// Pass by reference - cleaner syntax, can't be null
void updatePriceRef(double& price_ref, double change) {
    price_ref += change;
    std::cout << "Updated price via reference: $" << price_ref << std::endl;
}

// Return pointer to element in array
double* findPrice(double prices[], int size, double target) {
    for (int i = 0; i < size; ++i) {
        if (prices[i] == target) {
            return &prices[i];  // Return address of element
        }
    }
    return nullptr;  // Not found
}

// Pointer arithmetic for array traversal
double calculateAverage(const double* prices, int size) {
    double sum = 0.0;
    const double* end = prices + size;  // Pointer to one past last element

    for (const double* ptr = prices; ptr != end; ++ptr) {
        sum += *ptr;
    }

    return sum / size;
}

// Function that takes array via pointer
void printTrades(const Trade* trades, int count) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nTrade Report:" << std::endl;
    std::cout << "Symbol    Side  Quantity    Price      Value" << std::endl;
    std::cout << "------    ----  --------  --------  ----------" << std::endl;

    for (int i = 0; i < count; ++i) {
        // Arrow operator: ptr-> is equivalent to (*ptr).
        double value = trades[i].quantity * trades[i].price;
        std::cout << std::setw(6) << trades[i].symbol << "      "
                  << trades[i].side << "      "
                  << std::setw(4) << trades[i].quantity << "    "
                  << std::setw(6) << trades[i].price << "    "
                  << std::setw(8) << value << std::endl;
    }
}

// Const pointer vs pointer to const
void demonstrateConstPointers() {
    double price = 150.25;
    double new_price = 151.00;

    // Pointer to const: can't modify value, can change pointer
    const double* ptr1 = &price;
    std::cout << "Pointer to const: $" << *ptr1 << std::endl;
    // *ptr1 = 200.0;  // ERROR: can't modify value
    ptr1 = &new_price;  // OK: can change pointer
    std::cout << "After pointer change: $" << *ptr1 << std::endl;

    // Const pointer: can modify value, can't change pointer
    double* const ptr2 = &price;
    std::cout << "Const pointer: $" << *ptr2 << std::endl;
    *ptr2 = 152.00;  // OK: can modify value
    std::cout << "After value change: $" << *ptr2 << std::endl;
    // ptr2 = &new_price;  // ERROR: can't change pointer

    // Const pointer to const: can't modify value or pointer
    const double* const ptr3 = &price;
    std::cout << "Const pointer to const: $" << *ptr3 << std::endl;
    // *ptr3 = 200.0;  // ERROR
    // ptr3 = &new_price;  // ERROR
}

// Reference as function return (be careful!)
double& getElement(double arr[], int index) {
    return arr[index];  // Returns reference to array element
}

int main() {
    std::cout << "=== Pointers and References ===" << std::endl << std::endl;

    // Basic pointer operations
    std::cout << "--- Basic Pointers ---" << std::endl;
    double price = 150.25;
    double* price_ptr = &price;  // Pointer stores address

    std::cout << "Price value: $" << price << std::endl;
    std::cout << "Price address: " << &price << std::endl;
    std::cout << "Pointer value (address): " << price_ptr << std::endl;
    std::cout << "Pointer dereferenced: $" << *price_ptr << std::endl;
    std::cout << std::endl;

    // Modifying via pointer
    *price_ptr = 151.50;
    std::cout << "After modifying via pointer: $" << price << std::endl;
    std::cout << std::endl;

    // References
    std::cout << "--- References ---" << std::endl;
    double bid = 150.25;
    double& bid_ref = bid;  // Reference is an alias

    std::cout << "Bid: $" << bid << std::endl;
    std::cout << "Bid reference: $" << bid_ref << std::endl;

    bid_ref = 150.30;  // Modifying via reference
    std::cout << "After modifying via reference: $" << bid << std::endl;
    std::cout << "Same address? " << (&bid == &bid_ref ? "Yes" : "No") << std::endl;
    std::cout << std::endl;

    // Pass by pointer vs reference
    std::cout << "--- Pass by Pointer vs Reference ---" << std::endl;
    double ask = 150.27;
    std::cout << "Original ask: $" << ask << std::endl;

    updatePrice(&ask, 0.05);  // Pass address
    updatePriceRef(ask, 0.05);  // Pass by reference (no & needed at call site)
    std::cout << std::endl;

    // Null pointer check
    std::cout << "--- Null Pointer Safety ---" << std::endl;
    double* null_ptr = nullptr;  // C++11: use nullptr instead of NULL
    updatePrice(null_ptr, 0.05);
    std::cout << std::endl;

    // Array and pointers
    std::cout << "--- Arrays and Pointers ---" << std::endl;
    double prices[] = {150.25, 150.30, 150.28, 150.35, 150.32};
    int size = sizeof(prices) / sizeof(prices[0]);

    std::cout << "Array name as pointer: " << prices << std::endl;
    std::cout << "First element address: " << &prices[0] << std::endl;
    std::cout << "Same? " << (prices == &prices[0] ? "Yes" : "No") << std::endl;
    std::cout << std::endl;

    // Pointer arithmetic
    std::cout << "--- Pointer Arithmetic ---" << std::endl;
    double* ptr = prices;
    std::cout << "prices[0] = " << *ptr << std::endl;
    ptr++;  // Move to next element
    std::cout << "prices[1] = " << *ptr << std::endl;
    ptr += 2;  // Move forward 2 elements
    std::cout << "prices[3] = " << *ptr << std::endl;
    std::cout << std::endl;

    // Array traversal using pointers
    std::cout << "All prices using pointer arithmetic:" << std::endl;
    for (int i = 0; i < size; ++i) {
        std::cout << "  prices[" << i << "] = $" << *(prices + i) << std::endl;
    }
    std::cout << std::endl;

    // Find element and return pointer
    std::cout << "--- Finding Element ---" << std::endl;
    double target = 150.35;
    double* found = findPrice(prices, size, target);
    if (found != nullptr) {
        std::cout << "Found $" << target << " at address " << found << std::endl;
        std::cout << "Index: " << (found - prices) << std::endl;  // Pointer difference
    } else {
        std::cout << "Price not found" << std::endl;
    }
    std::cout << std::endl;

    // Calculate average using pointer
    double avg = calculateAverage(prices, size);
    std::cout << "Average price (via pointer): $" << avg << std::endl;
    std::cout << std::endl;

    // Structures and pointers
    std::cout << "--- Structures and Pointers ---" << std::endl;
    Trade trades[3];

    // Initialize trades
    strcpy(trades[0].symbol, "AAPL");
    trades[0].price = 150.25;
    trades[0].quantity = 100;
    trades[0].side = 'B';

    strcpy(trades[1].symbol, "MSFT");
    trades[1].price = 375.50;
    trades[1].quantity = 200;
    trades[1].side = 'B';

    strcpy(trades[2].symbol, "GOOGL");
    trades[2].price = 140.75;
    trades[2].quantity = 150;
    trades[2].side = 'S';

    printTrades(trades, 3);
    std::cout << std::endl;

    // Arrow operator
    Trade* trade_ptr = &trades[0];
    std::cout << "Using arrow operator:" << std::endl;
    std::cout << "Symbol: " << trade_ptr->symbol << std::endl;
    std::cout << "Price: $" << trade_ptr->price << std::endl;
    std::cout << std::endl;

    // Const pointers
    std::cout << "--- Const Pointers ---" << std::endl;
    demonstrateConstPointers();
    std::cout << std::endl;

    // Reference to array element
    std::cout << "--- Reference Return ---" << std::endl;
    double& first_price = getElement(prices, 0);
    std::cout << "First price: $" << first_price << std::endl;
    first_price = 151.00;  // Modify array through reference
    std::cout << "Modified first price: $" << prices[0] << std::endl;
    std::cout << std::endl;

    // Pointer vs reference summary
    std::cout << "--- Pointer vs Reference Summary ---" << std::endl;
    std::cout << "Pointers:" << std::endl;
    std::cout << "  + Can be reassigned" << std::endl;
    std::cout << "  + Can be null" << std::endl;
    std::cout << "  + Requires explicit dereferencing (*)" << std::endl;
    std::cout << "  + Used for optional parameters, dynamic memory" << std::endl;
    std::cout << "\nReferences:" << std::endl;
    std::cout << "  + Cannot be reassigned" << std::endl;
    std::cout << "  + Cannot be null" << std::endl;
    std::cout << "  + Cleaner syntax (automatic dereferencing)" << std::endl;
    std::cout << "  + Preferred for function parameters" << std::endl;

    return 0;
}

/*
 * Key Takeaways:
 * 1. Pointers store memory addresses, use * to dereference
 * 2. References are aliases, cleaner syntax than pointers
 * 3. Use nullptr (C++11) instead of NULL
 * 4. Pointer arithmetic: ptr + n moves n * sizeof(type) bytes
 * 5. Arrays decay to pointers in function calls
 * 6. Arrow operator (->) for accessing members through pointer
 * 7. const double* = pointer to const (can't modify value)
 * 8. double* const = const pointer (can't change pointer)
 * 9. Prefer references for function parameters unless null is valid
 * 10. Always check for nullptr before dereferencing
 */

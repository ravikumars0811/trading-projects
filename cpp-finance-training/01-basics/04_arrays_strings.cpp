/*
 * Arrays and Strings
 * HFT/Finance Context: Time series data, tick processing, symbol handling
 */

#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <iomanip>

const int MAX_TICKS = 10;

// Process price array
void analyzeTimeSeries(const double prices[], int size) {
    if (size == 0) return;

    double min = prices[0];
    double max = prices[0];
    double sum = 0.0;

    for (int i = 0; i < size; ++i) {
        sum += prices[i];
        if (prices[i] < min) min = prices[i];
        if (prices[i] > max) max = prices[i];
    }

    double avg = sum / size;
    double range = max - min;

    std::cout << "Time Series Analysis:" << std::endl;
    std::cout << "  Count: " << size << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Min: $" << min << std::endl;
    std::cout << "  Max: $" << max << std::endl;
    std::cout << "  Avg: $" << avg << std::endl;
    std::cout << "  Range: $" << range << std::endl;
}

// Multi-dimensional array: Portfolio positions
void displayPortfolio(const double portfolio[][3], int num_positions) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nPortfolio Positions:" << std::endl;
    std::cout << "Symbol  Quantity    Price       Value" << std::endl;
    std::cout << "------  --------  --------  ----------" << std::endl;

    double total_value = 0.0;
    for (int i = 0; i < num_positions; ++i) {
        // portfolio[i][0] = quantity
        // portfolio[i][1] = price
        // portfolio[i][2] = value (quantity * price)
        double value = portfolio[i][0] * portfolio[i][1];
        std::cout << "  " << i << "       "
                  << std::setw(6) << portfolio[i][0] << "    "
                  << std::setw(6) << portfolio[i][1] << "    "
                  << std::setw(8) << value << std::endl;
        total_value += value;
    }
    std::cout << "                           Total: $" << total_value << std::endl;
}

// C-style strings (legacy, but still seen in low-latency systems)
void parseTicker(const char* ticker) {
    std::cout << "Parsing ticker: " << ticker << std::endl;
    std::cout << "  Length: " << strlen(ticker) << std::endl;

    // Convert to uppercase
    char upper[16];
    strcpy(upper, ticker);
    for (int i = 0; upper[i]; ++i) {
        upper[i] = toupper(upper[i]);
    }
    std::cout << "  Uppercase: " << upper << std::endl;

    // Check if starts with certain prefix
    if (strncmp(upper, "AAPL", 4) == 0) {
        std::cout << "  Company: Apple Inc." << std::endl;
    }
}

// Modern C++ strings (std::string)
void analyzeOrderId(const std::string& order_id) {
    std::cout << "\nOrder ID Analysis: " << order_id << std::endl;

    // Length
    std::cout << "  Length: " << order_id.length() << std::endl;

    // Extract components (assuming format: VENUE-YYYYMMDD-SEQNUM)
    size_t first_dash = order_id.find('-');
    size_t second_dash = order_id.find('-', first_dash + 1);

    if (first_dash != std::string::npos && second_dash != std::string::npos) {
        std::string venue = order_id.substr(0, first_dash);
        std::string date = order_id.substr(first_dash + 1, second_dash - first_dash - 1);
        std::string seq = order_id.substr(second_dash + 1);

        std::cout << "  Venue: " << venue << std::endl;
        std::cout << "  Date: " << date << std::endl;
        std::cout << "  Sequence: " << seq << std::endl;
    }

    // Check prefix
    if (order_id.substr(0, 4) == "NYSE") {
        std::cout << "  Exchange: New York Stock Exchange" << std::endl;
    }
}

// String manipulation for symbol parsing
std::string extractSymbol(const std::string& order_msg) {
    // Example: "BUY|AAPL|100|150.25" -> extract "AAPL"
    size_t first = order_msg.find('|');
    size_t second = order_msg.find('|', first + 1);

    if (first != std::string::npos && second != std::string::npos) {
        return order_msg.substr(first + 1, second - first - 1);
    }
    return "";
}

// Array of strings: Symbol list
void displayWatchlist(const std::string watchlist[], int size) {
    std::cout << "\nWatchlist (" << size << " symbols):" << std::endl;
    for (int i = 0; i < size; ++i) {
        std::cout << "  " << (i + 1) << ". " << watchlist[i] << std::endl;
    }
}

int main() {
    std::cout << "=== Arrays and Strings in Trading ===" << std::endl << std::endl;

    // 1D Array: Price time series
    std::cout << "--- 1D Array: Price Time Series ---" << std::endl;
    double prices[MAX_TICKS] = {150.25, 150.30, 150.28, 150.35, 150.32,
                                 150.40, 150.38, 150.42, 150.45, 150.43};
    analyzeTimeSeries(prices, MAX_TICKS);

    // Array initialization variations
    std::cout << "\n--- Array Initialization ---" << std::endl;
    int volumes[] = {1000, 2000, 1500, 3000, 2500};  // Size inferred
    int size = sizeof(volumes) / sizeof(volumes[0]);
    std::cout << "Volumes array size: " << size << std::endl;
    for (int i = 0; i < size; ++i) {
        std::cout << "  Volume[" << i << "] = " << volumes[i] << std::endl;
    }

    // 2D Array: Portfolio data
    std::cout << "\n--- 2D Array: Portfolio ---" << std::endl;
    double portfolio[4][3] = {
        {100, 150.25, 0},  // AAPL: 100 shares @ $150.25
        {200, 75.50, 0},   // MSFT: 200 shares @ $75.50
        {150, 200.00, 0},  // GOOGL: 150 shares @ $200.00
        {300, 50.75, 0}    // TSLA: 300 shares @ $50.75
    };
    displayPortfolio(portfolio, 4);

    // C-style strings
    std::cout << "\n--- C-Style Strings ---" << std::endl;
    char ticker1[] = "aapl";
    char ticker2[16] = "msft";
    const char* ticker3 = "googl";  // String literal

    parseTicker(ticker1);
    parseTicker(ticker2);
    parseTicker(ticker3);

    // Modern C++ strings (std::string)
    std::cout << "\n--- std::string ---" << std::endl;
    std::string symbol = "AAPL";
    std::string order_id = "NYSE-20240115-123456";

    std::cout << "Symbol: " << symbol << std::endl;
    std::cout << "Length: " << symbol.length() << std::endl;
    std::cout << "First char: " << symbol[0] << std::endl;
    std::cout << "Last char: " << symbol.back() << std::endl;

    // String operations
    analyzeOrderId(order_id);

    // String concatenation
    std::cout << "\n--- String Operations ---" << std::endl;
    std::string msg = "Order for " + symbol + " executed";
    std::cout << msg << std::endl;

    // String comparison
    std::string sym1 = "AAPL";
    std::string sym2 = "MSFT";
    if (sym1 == sym2) {
        std::cout << "Symbols match" << std::endl;
    } else {
        std::cout << "Symbols differ: " << sym1 << " != " << sym2 << std::endl;
    }

    // Parsing order message
    std::string order_msg = "BUY|AAPL|100|150.25";
    std::cout << "\nParsing: " << order_msg << std::endl;
    std::string extracted = extractSymbol(order_msg);
    std::cout << "Extracted symbol: " << extracted << std::endl;

    // Array of strings
    std::cout << "\n--- Array of Strings ---" << std::endl;
    std::string watchlist[] = {"AAPL", "MSFT", "GOOGL", "AMZN", "TSLA"};
    int watchlist_size = sizeof(watchlist) / sizeof(watchlist[0]);
    displayWatchlist(watchlist, watchlist_size);

    // String to number conversion
    std::cout << "\n--- String to Number Conversion ---" << std::endl;
    std::string price_str = "150.25";
    std::string qty_str = "1000";

    double price = std::stod(price_str);
    int quantity = std::stoi(qty_str);

    std::cout << "Price string: \"" << price_str << "\" -> " << price << std::endl;
    std::cout << "Quantity string: \"" << qty_str << "\" -> " << quantity << std::endl;
    std::cout << "Trade value: $" << (price * quantity) << std::endl;

    // Number to string
    double value = 150250.50;
    std::string value_str = std::to_string(value);
    std::cout << "Value: " << value << " -> \"" << value_str << "\"" << std::endl;

    // Character array iteration
    std::cout << "\n--- Character Iteration ---" << std::endl;
    std::string ticker = "AAPL";
    std::cout << "Ticker characters: ";
    for (char c : ticker) {
        std::cout << c << " ";
    }
    std::cout << std::endl;

    // String search and replace
    std::cout << "\n--- String Search & Replace ---" << std::endl;
    std::string message = "Buy AAPL at market price";
    size_t pos = message.find("AAPL");
    if (pos != std::string::npos) {
        std::cout << "Found 'AAPL' at position: " << pos << std::endl;
        message.replace(pos, 4, "MSFT");
        std::cout << "After replace: " << message << std::endl;
    }

    return 0;
}

/*
 * Key Takeaways:
 * 1. Arrays store fixed-size sequences of same-type elements
 * 2. Use sizeof() to calculate array size
 * 3. Multi-dimensional arrays useful for tabular data
 * 4. C-style strings are char arrays ending with '\0'
 * 5. std::string is safer and more convenient than C-style strings
 * 6. String operations: find, substr, replace, concatenation
 * 7. Convert between strings and numbers: stoi, stod, to_string
 * 8. For performance-critical code, consider string_view (C++17)
 */

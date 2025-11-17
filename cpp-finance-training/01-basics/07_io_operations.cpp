/*
 * Input/Output Operations
 * HFT/Finance Context: Reading market data files, logging trades
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>

struct MarketData {
    std::string timestamp;
    std::string symbol;
    double bid;
    double ask;
    int bid_size;
    int ask_size;
};

// Write trades to file
void writeTradesToFile(const std::string& filename) {
    std::ofstream outfile(filename);

    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
        return;
    }

    // Write CSV header
    outfile << "OrderID,Symbol,Side,Quantity,Price,Timestamp" << std::endl;

    // Write sample trades
    outfile << "1001,AAPL,B,100,150.25,2024-01-15T09:30:00" << std::endl;
    outfile << "1002,MSFT,S,200,375.50,2024-01-15T09:30:01" << std::endl;
    outfile << "1003,GOOGL,B,150,140.75,2024-01-15T09:30:02" << std::endl;
    outfile << "1004,AMZN,B,75,185.30,2024-01-15T09:30:03" << std::endl;
    outfile << "1005,TSLA,S,300,242.15,2024-01-15T09:30:04" << std::endl;

    outfile.close();
    std::cout << "Trades written to " << filename << std::endl;
}

// Read trades from file
void readTradesFromFile(const std::string& filename) {
    std::ifstream infile(filename);

    if (!infile.is_open()) {
        std::cerr << "Error: Could not open file for reading: " << filename << std::endl;
        return;
    }

    std::cout << "\nReading trades from " << filename << ":" << std::endl;

    std::string line;
    std::getline(infile, line);  // Skip header
    std::cout << "Header: " << line << std::endl << std::endl;

    int count = 0;
    while (std::getline(infile, line)) {
        std::cout << "Trade " << (++count) << ": " << line << std::endl;
    }

    infile.close();
    std::cout << "\nTotal trades read: " << count << std::endl;
}

// Parse CSV line into fields
std::vector<std::string> parseCSV(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;

    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }

    return fields;
}

// Read and parse market data
void parseMarketDataFile(const std::string& filename) {
    std::ifstream infile(filename);

    if (!infile.is_open()) {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
        return;
    }

    std::vector<MarketData> data;
    std::string line;
    std::getline(infile, line);  // Skip header

    while (std::getline(infile, line)) {
        std::vector<std::string> fields = parseCSV(line);

        if (fields.size() >= 6) {
            MarketData md;
            md.timestamp = fields[0];
            md.symbol = fields[1];
            md.bid = std::stod(fields[2]);
            md.ask = std::stod(fields[3]);
            md.bid_size = std::stoi(fields[4]);
            md.ask_size = std::stoi(fields[5]);
            data.push_back(md);
        }
    }

    infile.close();

    // Display parsed data
    std::cout << "\nParsed Market Data:" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    for (const auto& md : data) {
        std::cout << md.timestamp << " | " << md.symbol
                  << " | Bid: $" << md.bid << " x " << md.bid_size
                  << " | Ask: $" << md.ask << " x " << md.ask_size << std::endl;
    }
}

// Write binary data (more efficient for large datasets)
void writeBinaryPrices(const std::string& filename, const double* prices, int count) {
    std::ofstream outfile(filename, std::ios::binary);

    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open binary file for writing" << std::endl;
        return;
    }

    // Write count first
    outfile.write(reinterpret_cast<const char*>(&count), sizeof(count));

    // Write prices array
    outfile.write(reinterpret_cast<const char*>(prices), count * sizeof(double));

    outfile.close();
    std::cout << "Written " << count << " prices to binary file" << std::endl;
}

// Read binary data
void readBinaryPrices(const std::string& filename) {
    std::ifstream infile(filename, std::ios::binary);

    if (!infile.is_open()) {
        std::cerr << "Error: Could not open binary file for reading" << std::endl;
        return;
    }

    // Read count
    int count;
    infile.read(reinterpret_cast<char*>(&count), sizeof(count));

    // Read prices array
    double* prices = new double[count];
    infile.read(reinterpret_cast<char*>(prices), count * sizeof(double));

    infile.close();

    std::cout << "\nRead " << count << " prices from binary file:" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    for (int i = 0; i < count; ++i) {
        std::cout << "  Price[" << i << "] = $" << prices[i] << std::endl;
    }

    delete[] prices;
}

// String stream for formatting
void demonstrateStringStream() {
    std::cout << "\n--- String Stream Formatting ---" << std::endl;

    // Building a formatted string
    std::ostringstream oss;
    oss << "Order "
        << std::setw(6) << std::setfill('0') << 12345
        << " | " << "AAPL"
        << " | BUY "
        << std::setw(4) << 100
        << " @ $" << std::fixed << std::setprecision(2) << 150.25;

    std::string order_msg = oss.str();
    std::cout << "Formatted order: " << order_msg << std::endl;

    // Parsing from string
    std::string data = "AAPL 150.25 1000";
    std::istringstream iss(data);

    std::string symbol;
    double price;
    int quantity;

    iss >> symbol >> price >> quantity;

    std::cout << "Parsed: Symbol=" << symbol
              << ", Price=$" << price
              << ", Qty=" << quantity << std::endl;
}

// Console I/O formatting
void demonstrateConsoleFormatting() {
    std::cout << "\n--- Console Formatting ---" << std::endl;

    // Column headers
    std::cout << std::left
              << std::setw(10) << "Symbol"
              << std::setw(8) << "Side"
              << std::setw(10) << "Quantity"
              << std::setw(10) << "Price"
              << std::setw(12) << "Value" << std::endl;

    std::cout << std::string(50, '-') << std::endl;

    // Data rows
    std::cout << std::fixed << std::setprecision(2);
    std::cout << std::left
              << std::setw(10) << "AAPL"
              << std::setw(8) << "BUY"
              << std::setw(10) << 100
              << std::right << "$" << std::setw(8) << 150.25
              << " $" << std::setw(10) << 15025.00 << std::endl;

    std::cout << std::left
              << std::setw(10) << "MSFT"
              << std::setw(8) << "SELL"
              << std::setw(10) << 200
              << std::right << "$" << std::setw(8) << 375.50
              << " $" << std::setw(10) << 75100.00 << std::endl;
}

// Append to log file
void logTrade(const std::string& log_file, const std::string& message) {
    std::ofstream logfile(log_file, std::ios::app);  // Append mode

    if (logfile.is_open()) {
        logfile << "[" << "2024-01-15T10:30:00" << "] " << message << std::endl;
        logfile.close();
    }
}

int main() {
    std::cout << "=== Input/Output Operations ===" << std::endl << std::endl;

    // Write to file
    std::cout << "--- Writing to File ---" << std::endl;
    writeTradesToFile("trades.csv");
    std::cout << std::endl;

    // Read from file
    std::cout << "--- Reading from File ---" << std::endl;
    readTradesFromFile("trades.csv");
    std::cout << std::endl;

    // Create and parse market data file
    std::cout << "--- Creating Market Data File ---" << std::endl;
    std::ofstream mdfile("market_data.csv");
    mdfile << "Timestamp,Symbol,Bid,Ask,BidSize,AskSize" << std::endl;
    mdfile << "2024-01-15T09:30:00,AAPL,150.24,150.26,500,300" << std::endl;
    mdfile << "2024-01-15T09:30:01,AAPL,150.25,150.27,600,400" << std::endl;
    mdfile << "2024-01-15T09:30:02,AAPL,150.23,150.25,700,350" << std::endl;
    mdfile.close();
    std::cout << "Market data file created" << std::endl;

    parseMarketDataFile("market_data.csv");
    std::cout << std::endl;

    // Binary file I/O
    std::cout << "--- Binary File I/O ---" << std::endl;
    double prices[] = {150.25, 150.30, 150.28, 150.35, 150.32};
    writeBinaryPrices("prices.bin", prices, 5);
    readBinaryPrices("prices.bin");
    std::cout << std::endl;

    // String streams
    demonstrateStringStream();

    // Console formatting
    demonstrateConsoleFormatting();

    // Logging
    std::cout << "\n--- Logging ---" << std::endl;
    std::string log_file = "trades.log";
    logTrade(log_file, "Trade executed: AAPL BUY 100 @ $150.25");
    logTrade(log_file, "Trade executed: MSFT SELL 200 @ $375.50");
    std::cout << "Trades logged to " << log_file << std::endl;

    // Error handling
    std::cout << "\n--- Error Handling ---" << std::endl;
    std::ifstream test_file("nonexistent.txt");
    if (!test_file.is_open()) {
        std::cerr << "✓ Properly handled missing file" << std::endl;
    }

    // Formatting flags
    std::cout << "\n--- Formatting Flags ---" << std::endl;
    double value = 1234.5;

    std::cout << "Default: " << value << std::endl;
    std::cout << "Fixed: " << std::fixed << value << std::endl;
    std::cout << "Scientific: " << std::scientific << value << std::endl;
    std::cout << "Precision 4: " << std::setprecision(4) << value << std::endl;

    std::cout << std::defaultfloat;  // Reset to default

    return 0;
}

/*
 * Key Takeaways:
 * 1. Use std::ifstream for reading files, std::ofstream for writing
 * 2. Always check if file opened successfully (!file.is_open())
 * 3. Text files: human-readable, CSV is common format
 * 4. Binary files: more efficient for large numerical datasets
 * 5. std::stringstream for string parsing and formatting
 * 6. Use std::getline() to read lines from files
 * 7. std::ios::app for append mode, std::ios::binary for binary mode
 * 8. Use iomanip manipulators (setw, setprecision, fixed) for formatting
 * 9. Always close files or use RAII (file closes automatically)
 * 10. Handle errors: check if file operations succeeded
 *
 * HFT Considerations:
 * - Binary files are faster for tick data storage
 * - Memory-mapped files even faster (covered in system programming)
 * - Minimize I/O in hot paths - buffer writes
 * - Consider async I/O for logging without blocking trading
 */

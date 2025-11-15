/**
 * Trading-Specific Algorithms for Quant Interviews
 *
 * Common problems asked in HFT/Quant interviews:
 * 1. Stock price problems (max profit, buy/sell timing)
 * 2. Time-series analysis
 * 3. Portfolio optimization
 * 4. Statistical calculations (moving averages, volatility)
 * 5. Order execution algorithms
 *
 * Interview Focus:
 * - Optimal solutions with analysis
 * - Real-world trading applications
 * - Performance considerations
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>
#include <queue>
#include <stack>
#include <unordered_map>

/**
 * Problem 1: Best Time to Buy and Sell Stock
 * Given stock prices, find maximum profit with one transaction
 *
 * Application: Simple trade execution strategy
 * Time: O(n), Space: O(1)
 */
int maxProfit_oneTransaction(const std::vector<int>& prices) {
    if (prices.empty()) return 0;

    int min_price = prices[0];
    int max_profit = 0;

    for (int price : prices) {
        min_price = std::min(min_price, price);
        max_profit = std::max(max_profit, price - min_price);
    }

    return max_profit;
}

/**
 * Problem 2: Best Time to Buy and Sell Stock II
 * Multiple transactions allowed (unlimited)
 *
 * Application: Market making, capturing all upward moves
 * Time: O(n), Space: O(1)
 */
int maxProfit_multipleTransactions(const std::vector<int>& prices) {
    int total_profit = 0;

    for (size_t i = 1; i < prices.size(); ++i) {
        // Capture every positive difference
        if (prices[i] > prices[i-1]) {
            total_profit += prices[i] - prices[i-1];
        }
    }

    return total_profit;
}

/**
 * Problem 3: Best Time with K Transactions
 * Maximum profit with at most k transactions
 *
 * Application: Optimal execution with limited trades
 * Time: O(nk), Space: O(k)
 */
int maxProfit_kTransactions(int k, const std::vector<int>& prices) {
    int n = prices.size();
    if (n == 0 || k == 0) return 0;

    // If k >= n/2, unlimited transactions
    if (k >= n / 2) {
        return maxProfit_multipleTransactions(prices);
    }

    // dp[i][j] = max profit with at most i transactions by day j
    std::vector<std::vector<int>> dp(k + 1, std::vector<int>(n, 0));

    for (int i = 1; i <= k; ++i) {
        int max_diff = -prices[0];
        for (int j = 1; j < n; ++j) {
            dp[i][j] = std::max(dp[i][j-1], prices[j] + max_diff);
            max_diff = std::max(max_diff, dp[i-1][j] - prices[j]);
        }
    }

    return dp[k][n-1];
}

/**
 * Problem 4: Simple Moving Average (SMA)
 * Calculate moving average over window
 *
 * Application: Trend following, signal generation
 * Time: O(n), Space: O(1)
 */
class SimpleMovingAverage {
private:
    int window_size_;
    std::deque<double> window_;
    double sum_;

public:
    explicit SimpleMovingAverage(int window)
        : window_size_(window), sum_(0.0) {}

    double next(double price) {
        window_.push_back(price);
        sum_ += price;

        if (window_.size() > window_size_) {
            sum_ -= window_.front();
            window_.pop_front();
        }

        return sum_ / window_.size();
    }
};

/**
 * Problem 5: Exponential Moving Average (EMA)
 * Weighted average giving more weight to recent prices
 *
 * Application: More responsive than SMA, trend detection
 * Time: O(1) per update, Space: O(1)
 */
class ExponentialMovingAverage {
private:
    double alpha_;  // Smoothing factor
    double ema_;
    bool initialized_;

public:
    explicit ExponentialMovingAverage(int period)
        : alpha_(2.0 / (period + 1)), ema_(0.0), initialized_(false) {}

    double next(double price) {
        if (!initialized_) {
            ema_ = price;
            initialized_ = true;
        } else {
            ema_ = alpha_ * price + (1 - alpha_) * ema_;
        }
        return ema_;
    }

    double get() const { return ema_; }
};

/**
 * Problem 6: Calculate Historical Volatility
 * Standard deviation of log returns
 *
 * Application: Risk management, option pricing
 * Time: O(n), Space: O(n)
 */
double calculateVolatility(const std::vector<double>& prices) {
    if (prices.size() < 2) return 0.0;

    // Calculate log returns
    std::vector<double> returns;
    for (size_t i = 1; i < prices.size(); ++i) {
        returns.push_back(std::log(prices[i] / prices[i-1]));
    }

    // Calculate mean
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();

    // Calculate variance
    double variance = 0.0;
    for (double ret : returns) {
        variance += (ret - mean) * (ret - mean);
    }
    variance /= returns.size();

    // Annualize (assuming daily data, 252 trading days)
    return std::sqrt(variance * 252);
}

/**
 * Problem 7: VWAP (Volume Weighted Average Price)
 * Average price weighted by volume
 *
 * Application: Execution benchmark, algorithmic trading
 * Time: O(n), Space: O(1)
 */
double calculateVWAP(const std::vector<double>& prices,
                     const std::vector<int>& volumes) {
    if (prices.size() != volumes.size() || prices.empty()) {
        return 0.0;
    }

    double total_pv = 0.0;  // Price * Volume
    int total_volume = 0;

    for (size_t i = 0; i < prices.size(); ++i) {
        total_pv += prices[i] * volumes[i];
        total_volume += volumes[i];
    }

    return total_volume > 0 ? total_pv / total_volume : 0.0;
}

/**
 * Problem 8: TWAP (Time Weighted Average Price)
 * Simple average of prices over time
 *
 * Application: Execution strategy, minimize market impact
 * Time: O(n), Space: O(1)
 */
double calculateTWAP(const std::vector<double>& prices) {
    if (prices.empty()) return 0.0;
    return std::accumulate(prices.begin(), prices.end(), 0.0) / prices.size();
}

/**
 * Problem 9: Bollinger Bands
 * Moving average ± k * standard deviation
 *
 * Application: Mean reversion strategy, volatility bands
 * Time: O(n), Space: O(window)
 */
struct BollingerBands {
    double middle;
    double upper;
    double lower;
};

BollingerBands calculateBollingerBands(const std::vector<double>& prices,
                                       int window, double k = 2.0) {
    if (prices.size() < window) {
        return {0.0, 0.0, 0.0};
    }

    // Calculate SMA
    double sum = 0.0;
    for (int i = prices.size() - window; i < prices.size(); ++i) {
        sum += prices[i];
    }
    double sma = sum / window;

    // Calculate standard deviation
    double variance = 0.0;
    for (int i = prices.size() - window; i < prices.size(); ++i) {
        variance += (prices[i] - sma) * (prices[i] - sma);
    }
    double std_dev = std::sqrt(variance / window);

    return {sma, sma + k * std_dev, sma - k * std_dev};
}

/**
 * Problem 10: RSI (Relative Strength Index)
 * Momentum oscillator (0-100)
 *
 * Application: Overbought/oversold indicator
 * Time: O(n), Space: O(1)
 */
double calculateRSI(const std::vector<double>& prices, int period = 14) {
    if (prices.size() < period + 1) return 50.0;

    double avg_gain = 0.0;
    double avg_loss = 0.0;

    // Initial average
    for (size_t i = 1; i <= period; ++i) {
        double change = prices[i] - prices[i-1];
        if (change > 0) {
            avg_gain += change;
        } else {
            avg_loss += -change;
        }
    }
    avg_gain /= period;
    avg_loss /= period;

    // Smooth subsequent values
    for (size_t i = period + 1; i < prices.size(); ++i) {
        double change = prices[i] - prices[i-1];
        if (change > 0) {
            avg_gain = (avg_gain * (period - 1) + change) / period;
            avg_loss = (avg_loss * (period - 1)) / period;
        } else {
            avg_gain = (avg_gain * (period - 1)) / period;
            avg_loss = (avg_loss * (period - 1) + (-change)) / period;
        }
    }

    if (avg_loss == 0) return 100.0;

    double rs = avg_gain / avg_loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

/**
 * Problem 11: Maximum Drawdown
 * Largest peak-to-trough decline
 *
 * Application: Risk metric, strategy evaluation
 * Time: O(n), Space: O(1)
 */
double calculateMaxDrawdown(const std::vector<double>& equity_curve) {
    if (equity_curve.empty()) return 0.0;

    double max_equity = equity_curve[0];
    double max_drawdown = 0.0;

    for (double equity : equity_curve) {
        max_equity = std::max(max_equity, equity);
        double drawdown = (max_equity - equity) / max_equity;
        max_drawdown = std::max(max_drawdown, drawdown);
    }

    return max_drawdown;
}

/**
 * Problem 12: Sharpe Ratio
 * Risk-adjusted return metric
 *
 * Application: Strategy performance evaluation
 * Time: O(n), Space: O(1)
 */
double calculateSharpeRatio(const std::vector<double>& returns,
                           double risk_free_rate = 0.02) {
    if (returns.empty()) return 0.0;

    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();

    double variance = 0.0;
    for (double ret : returns) {
        variance += (ret - mean) * (ret - mean);
    }
    double std_dev = std::sqrt(variance / returns.size());

    if (std_dev == 0) return 0.0;

    // Annualize (assuming daily returns)
    double annualized_return = mean * 252;
    double annualized_vol = std_dev * std::sqrt(252);

    return (annualized_return - risk_free_rate) / annualized_vol;
}

/**
 * Problem 13: Pair Trading - Find Cointegrated Pairs
 * Calculate correlation between two time series
 *
 * Application: Statistical arbitrage
 * Time: O(n), Space: O(1)
 */
double calculateCorrelation(const std::vector<double>& x,
                           const std::vector<double>& y) {
    if (x.size() != y.size() || x.empty()) return 0.0;

    int n = x.size();
    double mean_x = std::accumulate(x.begin(), x.end(), 0.0) / n;
    double mean_y = std::accumulate(y.begin(), y.end(), 0.0) / n;

    double cov = 0.0, var_x = 0.0, var_y = 0.0;

    for (int i = 0; i < n; ++i) {
        double dx = x[i] - mean_x;
        double dy = y[i] - mean_y;
        cov += dx * dy;
        var_x += dx * dx;
        var_y += dy * dy;
    }

    if (var_x == 0 || var_y == 0) return 0.0;

    return cov / std::sqrt(var_x * var_y);
}

/**
 * Problem 14: Optimal Order Execution Schedule
 * Split large order over time to minimize impact
 *
 * Application: VWAP/TWAP execution algorithms
 * Time: O(n), Space: O(n)
 */
std::vector<int> optimizeTWAPSchedule(int total_quantity, int num_periods) {
    std::vector<int> schedule(num_periods);
    int base_qty = total_quantity / num_periods;
    int remainder = total_quantity % num_periods;

    for (int i = 0; i < num_periods; ++i) {
        schedule[i] = base_qty + (i < remainder ? 1 : 0);
    }

    return schedule;
}

/**
 * Problem 15: Detect Price Anomalies
 * Find outliers using z-score
 *
 * Application: Market surveillance, risk management
 * Time: O(n), Space: O(1)
 */
std::vector<int> detectAnomalies(const std::vector<double>& prices,
                                 double threshold = 3.0) {
    std::vector<int> anomalies;
    if (prices.size() < 2) return anomalies;

    double mean = std::accumulate(prices.begin(), prices.end(), 0.0) / prices.size();

    double variance = 0.0;
    for (double price : prices) {
        variance += (price - mean) * (price - mean);
    }
    double std_dev = std::sqrt(variance / prices.size());

    if (std_dev == 0) return anomalies;

    for (size_t i = 0; i < prices.size(); ++i) {
        double z_score = std::abs(prices[i] - mean) / std_dev;
        if (z_score > threshold) {
            anomalies.push_back(i);
        }
    }

    return anomalies;
}

// Demonstration and Testing
void demonstrate_trading_algorithms() {
    std::cout << "\n=== Stock Price Problems ===" << std::endl;

    std::vector<int> prices1 = {7, 1, 5, 3, 6, 4};
    std::cout << "Prices: ";
    for (int p : prices1) std::cout << p << " ";
    std::cout << "\nMax profit (1 transaction): "
              << maxProfit_oneTransaction(prices1) << std::endl;
    std::cout << "Max profit (unlimited): "
              << maxProfit_multipleTransactions(prices1) << std::endl;
    std::cout << "Max profit (2 transactions): "
              << maxProfit_kTransactions(2, prices1) << std::endl;

    std::cout << "\n=== Moving Averages ===" << std::endl;
    std::vector<double> prices2 = {100, 102, 101, 103, 105, 104, 106};
    SimpleMovingAverage sma(3);
    ExponentialMovingAverage ema(3);

    std::cout << "Price\tSMA(3)\tEMA(3)" << std::endl;
    for (double price : prices2) {
        std::cout << price << "\t"
                  << sma.next(price) << "\t"
                  << ema.next(price) << std::endl;
    }

    std::cout << "\n=== Volatility ===" << std::endl;
    std::vector<double> prices3 = {100, 102, 98, 101, 99, 103, 97, 102};
    double vol = calculateVolatility(prices3);
    std::cout << "Annual volatility: " << (vol * 100) << "%" << std::endl;

    std::cout << "\n=== VWAP vs TWAP ===" << std::endl;
    std::vector<double> prices4 = {100, 101, 102, 103};
    std::vector<int> volumes = {1000, 2000, 1500, 1000};
    std::cout << "VWAP: " << calculateVWAP(prices4, volumes) << std::endl;
    std::cout << "TWAP: " << calculateTWAP(prices4) << std::endl;

    std::cout << "\n=== Bollinger Bands ===" << std::endl;
    std::vector<double> prices5 = {100, 102, 101, 103, 105, 104, 106, 108, 107, 109};
    auto bands = calculateBollingerBands(prices5, 5, 2.0);
    std::cout << "Middle: " << bands.middle << std::endl;
    std::cout << "Upper:  " << bands.upper << std::endl;
    std::cout << "Lower:  " << bands.lower << std::endl;

    std::cout << "\n=== RSI ===" << std::endl;
    std::vector<double> prices6(30);
    for (int i = 0; i < 30; ++i) {
        prices6[i] = 100 + std::sin(i * 0.5) * 10;
    }
    double rsi = calculateRSI(prices6, 14);
    std::cout << "RSI(14): " << rsi << std::endl;

    std::cout << "\n=== Risk Metrics ===" << std::endl;
    std::vector<double> equity = {100, 105, 103, 108, 107, 112, 108, 115};
    std::cout << "Max Drawdown: "
              << (calculateMaxDrawdown(equity) * 100) << "%" << std::endl;

    std::vector<double> returns = {0.01, -0.02, 0.03, 0.01, -0.01, 0.02};
    std::cout << "Sharpe Ratio: " << calculateSharpeRatio(returns) << std::endl;

    std::cout << "\n=== Correlation ===" << std::endl;
    std::vector<double> stock_a = {100, 102, 104, 103, 105};
    std::vector<double> stock_b = {50, 51, 52, 51.5, 52.5};
    std::cout << "Correlation: " << calculateCorrelation(stock_a, stock_b) << std::endl;

    std::cout << "\n=== Order Execution Schedule ===" << std::endl;
    auto schedule = optimizeTWAPSchedule(10000, 10);
    std::cout << "TWAP schedule for 10,000 shares over 10 periods:" << std::endl;
    for (size_t i = 0; i < schedule.size(); ++i) {
        std::cout << "Period " << i+1 << ": " << schedule[i] << " shares" << std::endl;
    }

    std::cout << "\n=== Anomaly Detection ===" << std::endl;
    std::vector<double> prices7 = {100, 101, 102, 150, 101, 99, 100, 98};
    auto anomalies = detectAnomalies(prices7, 2.0);
    std::cout << "Anomalies at indices: ";
    for (int idx : anomalies) {
        std::cout << idx << " (price=" << prices7[idx] << ") ";
    }
    std::cout << std::endl;
}

/**
 * KEY INTERVIEW POINTS:
 *
 * 1. Time-Series Analysis:
 *    - Moving averages: SMA (simple), EMA (exponential)
 *    - Choose based on: SMA for smooth, EMA for responsive
 *    - Common periods: 20, 50, 200 days
 *
 * 2. Volatility Measures:
 *    - Historical: Standard deviation of log returns
 *    - Implied: From option prices (Black-Scholes)
 *    - Realized: Actual price movements
 *
 * 3. Technical Indicators:
 *    - RSI: Overbought (>70), Oversold (<30)
 *    - Bollinger Bands: Mean reversion plays
 *    - MACD: Trend following
 *
 * 4. Execution Algorithms:
 *    - VWAP: Match average market price
 *    - TWAP: Minimize market impact
 *    - POV: Percentage of volume
 *    - Implementation Shortfall: Minimize cost
 *
 * 5. Risk Metrics:
 *    - Sharpe Ratio: Return per unit risk
 *    - Max Drawdown: Worst peak-to-trough
 *    - VaR: Value at Risk
 *    - Beta: Market correlation
 *
 * 6. Statistical Arbitrage:
 *    - Pairs trading: Long/short correlated pairs
 *    - Mean reversion: Price returns to average
 *    - Cointegration: Long-term relationship
 *
 * 7. Common Interview Questions:
 *    Q: SMA vs EMA?
 *    A: SMA equal weights, EMA weights recent more
 *       EMA responds faster to price changes
 *
 *    Q: How to minimize slippage?
 *    A: Use VWAP/TWAP, split orders, dark pools,
 *       limit orders, avoid market orders
 *
 *    Q: What makes a good trading signal?
 *    A: High Sharpe ratio, low drawdown,
 *       robust across time periods, low correlation
 *       with other strategies
 *
 * 8. Optimization Tips:
 *    - Use circular buffers for moving windows
 *    - Incremental updates for running stats
 *    - Cache frequently used calculations
 *    - SIMD for vector operations
 *
 * Time Complexity Summary:
 * - Most indicators: O(n) one-pass
 * - Moving averages: O(1) amortized with deque
 * - Correlation: O(n) single pass
 * - DP problems: O(nk) for k transactions
 */

int main() {
    demonstrate_trading_algorithms();
    return 0;
}

/**
 * Backtesting Engine Implementation
 *
 * A complete backtesting framework for trading strategies
 *
 * Features:
 * - Event-driven architecture
 * - Support for multiple strategies
 * - Portfolio management
 * - Performance metrics calculation
 * - Transaction cost modeling
 *
 * Interview Focus:
 * - System design
 * - Object-oriented design
 * - Financial domain knowledge
 * - Performance optimization
 *
 * Complexity:
 * - Time: O(n * s) where n = bars, s = strategies
 * - Space: O(n + p) where p = positions
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cmath>
#include <algorithm>
#include <iomanip>

// ============================================================================
// DATA STRUCTURES
// ============================================================================

struct Bar {
    std::string timestamp;
    double open;
    double high;
    double low;
    double close;
    int volume;

    Bar(const std::string& ts, double o, double h, double l, double c, int v)
        : timestamp(ts), open(o), high(h), low(l), close(c), volume(v) {}
};

enum class OrderType { MARKET, LIMIT };
enum class OrderSide { BUY, SELL };

struct Order {
    int order_id;
    std::string symbol;
    OrderType type;
    OrderSide side;
    int quantity;
    double price;
    std::string timestamp;

    Order(int id, const std::string& sym, OrderType t, OrderSide s, int q, double p, const std::string& ts)
        : order_id(id), symbol(sym), type(t), side(s), quantity(q), price(p), timestamp(ts) {}
};

struct Fill {
    int order_id;
    std::string symbol;
    OrderSide side;
    int quantity;
    double price;
    double commission;
    std::string timestamp;

    Fill(int id, const std::string& sym, OrderSide s, int q, double p, double c, const std::string& ts)
        : order_id(id), symbol(sym), side(s), quantity(q), price(p), commission(c), timestamp(ts) {}
};

struct Position {
    std::string symbol;
    int quantity;
    double avg_price;
    double unrealized_pnl;
    double realized_pnl;

    Position(const std::string& sym)
        : symbol(sym), quantity(0), avg_price(0.0), unrealized_pnl(0.0), realized_pnl(0.0) {}

    void update(const Fill& fill, double current_price) {
        if (fill.side == OrderSide::BUY) {
            // Buying
            if (quantity >= 0) {
                // Adding to long
                avg_price = (avg_price * quantity + fill.price * fill.quantity) /
                           (quantity + fill.quantity);
                quantity += fill.quantity;
            } else {
                // Covering short
                int close_qty = std::min(-quantity, fill.quantity);
                realized_pnl += close_qty * (avg_price - fill.price);
                quantity += fill.quantity;
                if (quantity > 0) {
                    avg_price = fill.price;
                }
            }
        } else {
            // Selling
            if (quantity <= 0) {
                // Adding to short
                avg_price = (avg_price * (-quantity) + fill.price * fill.quantity) /
                           (-quantity + fill.quantity);
                quantity -= fill.quantity;
            } else {
                // Closing long
                int close_qty = std::min(quantity, fill.quantity);
                realized_pnl += close_qty * (fill.price - avg_price);
                quantity -= fill.quantity;
                if (quantity < 0) {
                    avg_price = fill.price;
                }
            }
        }

        // Update unrealized P&L
        if (quantity > 0) {
            unrealized_pnl = quantity * (current_price - avg_price);
        } else if (quantity < 0) {
            unrealized_pnl = (-quantity) * (avg_price - current_price);
        } else {
            unrealized_pnl = 0.0;
        }
    }
};

// ============================================================================
// STRATEGY BASE CLASS
// ============================================================================

class Strategy {
protected:
    std::string name_;

public:
    explicit Strategy(const std::string& name) : name_(name) {}
    virtual ~Strategy() = default;

    virtual void onBar(const std::string& symbol, const Bar& bar) = 0;
    virtual std::vector<Order> generateOrders(const std::string& timestamp) = 0;

    const std::string& getName() const { return name_; }
};

// ============================================================================
// EXAMPLE STRATEGIES
// ============================================================================

// Simple Moving Average Crossover Strategy
class SMAStrategy : public Strategy {
private:
    std::string symbol_;
    int short_period_;
    int long_period_;
    std::vector<double> prices_;
    bool in_position_;

public:
    SMAStrategy(const std::string& name, const std::string& symbol,
                int short_p, int long_p)
        : Strategy(name), symbol_(symbol), short_period_(short_p),
          long_period_(long_p), in_position_(false) {}

    void onBar(const std::string& symbol, const Bar& bar) override {
        if (symbol == symbol_) {
            prices_.push_back(bar.close);
        }
    }

    std::vector<Order> generateOrders(const std::string& timestamp) override {
        std::vector<Order> orders;

        if (prices_.size() < long_period_) {
            return orders;
        }

        // Calculate SMAs
        double short_sma = 0.0;
        for (int i = prices_.size() - short_period_; i < prices_.size(); ++i) {
            short_sma += prices_[i];
        }
        short_sma /= short_period_;

        double long_sma = 0.0;
        for (int i = prices_.size() - long_period_; i < prices_.size(); ++i) {
            long_sma += prices_[i];
        }
        long_sma /= long_period_;

        // Generate signals
        if (short_sma > long_sma && !in_position_) {
            // Golden cross - buy
            orders.emplace_back(1, symbol_, OrderType::MARKET, OrderSide::BUY,
                              100, 0.0, timestamp);
            in_position_ = true;
        } else if (short_sma < long_sma && in_position_) {
            // Death cross - sell
            orders.emplace_back(2, symbol_, OrderType::MARKET, OrderSide::SELL,
                              100, 0.0, timestamp);
            in_position_ = false;
        }

        return orders;
    }
};

// Mean Reversion Strategy
class MeanReversionStrategy : public Strategy {
private:
    std::string symbol_;
    int lookback_;
    double z_threshold_;
    std::vector<double> prices_;
    bool in_position_;

public:
    MeanReversionStrategy(const std::string& name, const std::string& symbol,
                         int lookback, double threshold)
        : Strategy(name), symbol_(symbol), lookback_(lookback),
          z_threshold_(threshold), in_position_(false) {}

    void onBar(const std::string& symbol, const Bar& bar) override {
        if (symbol == symbol_) {
            prices_.push_back(bar.close);
            if (prices_.size() > lookback_) {
                prices_.erase(prices_.begin());
            }
        }
    }

    std::vector<Order> generateOrders(const std::string& timestamp) override {
        std::vector<Order> orders;

        if (prices_.size() < lookback_) {
            return orders;
        }

        // Calculate mean and stddev
        double mean = 0.0;
        for (double p : prices_) mean += p;
        mean /= prices_.size();

        double variance = 0.0;
        for (double p : prices_) {
            variance += (p - mean) * (p - mean);
        }
        double stddev = std::sqrt(variance / prices_.size());

        if (stddev == 0) return orders;

        // Calculate z-score
        double z_score = (prices_.back() - mean) / stddev;

        // Generate signals
        if (z_score < -z_threshold_ && !in_position_) {
            // Oversold - buy
            orders.emplace_back(1, symbol_, OrderType::MARKET, OrderSide::BUY,
                              100, 0.0, timestamp);
            in_position_ = true;
        } else if (z_score > z_threshold_ && in_position_) {
            // Overbought - sell
            orders.emplace_back(2, symbol_, OrderType::MARKET, OrderSide::SELL,
                              100, 0.0, timestamp);
            in_position_ = false;
        }

        return orders;
    }
};

// ============================================================================
// BACKTESTING ENGINE
// ============================================================================

class BacktestEngine {
private:
    // Market data
    std::map<std::string, std::vector<Bar>> historical_data_;

    // Strategies
    std::vector<std::unique_ptr<Strategy>> strategies_;

    // Portfolio
    std::map<std::string, Position> positions_;
    double cash_;
    double initial_capital_;

    // Transaction costs
    double commission_rate_;  // Per share

    // Performance tracking
    std::vector<double> equity_curve_;
    std::vector<Fill> fills_;

    int next_order_id_;

public:
    explicit BacktestEngine(double initial_capital, double commission_rate = 0.01)
        : cash_(initial_capital), initial_capital_(initial_capital),
          commission_rate_(commission_rate), next_order_id_(1) {}

    void addData(const std::string& symbol, const std::vector<Bar>& bars) {
        historical_data_[symbol] = bars;
    }

    void addStrategy(std::unique_ptr<Strategy> strategy) {
        strategies_.push_back(std::move(strategy));
    }

    void run() {
        std::cout << "\n=== Running Backtest ===" << std::endl;
        std::cout << "Initial Capital: $" << initial_capital_ << std::endl;
        std::cout << "Commission Rate: $" << commission_rate_ << " per share\n" << std::endl;

        // Assuming all symbols have same date range
        if (historical_data_.empty()) return;

        size_t num_bars = historical_data_.begin()->second.size();

        for (size_t i = 0; i < num_bars; ++i) {
            // Feed bar to all strategies
            for (const auto& [symbol, bars] : historical_data_) {
                for (auto& strategy : strategies_) {
                    strategy->onBar(symbol, bars[i]);
                }
            }

            // Generate orders from strategies
            const auto& timestamp = historical_data_.begin()->second[i].timestamp;
            std::vector<Order> orders;

            for (auto& strategy : strategies_) {
                auto strategy_orders = strategy->generateOrders(timestamp);
                orders.insert(orders.end(), strategy_orders.begin(), strategy_orders.end());
            }

            // Execute orders
            for (auto& order : orders) {
                executeOrder(order, i);
            }

            // Update portfolio value
            updateEquityCurve(i);
        }

        // Print results
        printResults();
    }

private:
    void executeOrder(Order& order, size_t bar_index) {
        const auto& bar = historical_data_[order.symbol][bar_index];

        // Determine execution price
        double exec_price = 0.0;
        if (order.type == OrderType::MARKET) {
            // Assume execution at close price
            exec_price = bar.close;
        } else {
            // Limit order logic
            exec_price = order.price;
        }

        // Calculate commission
        double commission = order.quantity * commission_rate_;

        // Check if we have enough cash
        if (order.side == OrderSide::BUY) {
            double cost = order.quantity * exec_price + commission;
            if (cost > cash_) {
                std::cout << "Insufficient cash for order. Required: $" << cost
                          << ", Available: $" << cash_ << std::endl;
                return;
            }
            cash_ -= cost;
        } else {
            cash_ += order.quantity * exec_price - commission;
        }

        // Create fill
        Fill fill(order.order_id, order.symbol, order.side, order.quantity,
                 exec_price, commission, order.timestamp);
        fills_.push_back(fill);

        // Update position
        if (positions_.find(order.symbol) == positions_.end()) {
            positions_[order.symbol] = Position(order.symbol);
        }
        positions_[order.symbol].update(fill, bar.close);

        std::cout << order.timestamp << " - "
                  << (order.side == OrderSide::BUY ? "BUY" : "SELL") << " "
                  << order.quantity << " " << order.symbol
                  << " @ $" << std::fixed << std::setprecision(2) << exec_price
                  << " (Commission: $" << commission << ")" << std::endl;
    }

    void updateEquityCurve(size_t bar_index) {
        double total_value = cash_;

        for (auto& [symbol, position] : positions_) {
            const auto& bar = historical_data_[symbol][bar_index];
            position.update(Fill(0, symbol, OrderSide::BUY, 0, 0, 0, ""),
                          bar.close);
            total_value += position.quantity * bar.close;
        }

        equity_curve_.push_back(total_value);
    }

    void printResults() {
        std::cout << "\n=== Backtest Results ===" << std::endl;

        // Final values
        double final_value = equity_curve_.back();
        double total_return = (final_value - initial_capital_) / initial_capital_ * 100;

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Final Portfolio Value: $" << final_value << std::endl;
        std::cout << "Total Return: " << total_return << "%" << std::endl;

        // Calculate max drawdown
        double max_equity = equity_curve_[0];
        double max_drawdown = 0.0;

        for (double equity : equity_curve_) {
            max_equity = std::max(max_equity, equity);
            double drawdown = (max_equity - equity) / max_equity;
            max_drawdown = std::max(max_drawdown, drawdown);
        }

        std::cout << "Max Drawdown: " << (max_drawdown * 100) << "%" << std::endl;

        // Calculate Sharpe ratio (simplified - assuming daily data)
        std::vector<double> returns;
        for (size_t i = 1; i < equity_curve_.size(); ++i) {
            returns.push_back((equity_curve_[i] - equity_curve_[i-1]) / equity_curve_[i-1]);
        }

        if (!returns.empty()) {
            double mean_return = 0.0;
            for (double r : returns) mean_return += r;
            mean_return /= returns.size();

            double variance = 0.0;
            for (double r : returns) {
                variance += (r - mean_return) * (r - mean_return);
            }
            double stddev = std::sqrt(variance / returns.size());

            double sharpe = (stddev > 0) ? (mean_return / stddev * std::sqrt(252)) : 0.0;
            std::cout << "Sharpe Ratio: " << std::setprecision(3) << sharpe << std::endl;
        }

        // Trade statistics
        int num_trades = fills_.size();
        double total_commission = 0.0;
        for (const auto& fill : fills_) {
            total_commission += fill.commission;
        }

        std::cout << "\nTotal Trades: " << num_trades << std::endl;
        std::cout << "Total Commissions: $" << total_commission << std::endl;

        // Position summary
        std::cout << "\n=== Final Positions ===" << std::endl;
        for (const auto& [symbol, position] : positions_) {
            if (position.quantity != 0) {
                std::cout << symbol << ": " << position.quantity << " shares"
                          << ", Avg Price: $" << position.avg_price
                          << ", Unrealized P&L: $" << position.unrealized_pnl << std::endl;
            }
        }
    }
};

// ============================================================================
// DEMONSTRATION
// ============================================================================

void demonstrate_backtesting() {
    // Create sample data
    std::vector<Bar> aapl_data;
    double base_price = 150.0;

    // Generate 100 days of synthetic data with trend and noise
    for (int i = 0; i < 100; ++i) {
        double trend = i * 0.1;
        double noise = (rand() % 100 - 50) / 100.0;
        double price = base_price + trend + noise;

        std::string timestamp = "2024-01-" + std::to_string(i + 1);
        aapl_data.emplace_back(timestamp, price, price + 1, price - 1, price, 1000000);
    }

    // Create backtesting engine
    BacktestEngine engine(100000.0, 0.01);
    engine.addData("AAPL", aapl_data);

    // Add strategies
    engine.addStrategy(std::make_unique<SMAStrategy>("SMA_Strategy", "AAPL", 10, 20));
    engine.addStrategy(std::make_unique<MeanReversionStrategy>("MeanRev_Strategy", "AAPL", 20, 2.0));

    // Run backtest
    engine.run();
}

/**
 * KEY INTERVIEW POINTS:
 *
 * 1. Event-Driven Architecture:
 *    - Process data chronologically
 *    - Avoid look-ahead bias
 *    - Realistic order execution
 *    - Track state over time
 *
 * 2. Strategy Design:
 *    - Base class with virtual methods
 *    - Strategy can maintain state
 *    - Signals generated from historical data
 *    - Multiple strategies can coexist
 *
 * 3. Portfolio Management:
 *    - Track positions per symbol
 *    - Calculate P&L (realized/unrealized)
 *    - Average price calculation
 *    - Cash management
 *
 * 4. Transaction Costs:
 *    - Commission per share/trade
 *    - Slippage modeling
 *    - Spread costs
 *    - Impact on profitability
 *
 * 5. Performance Metrics:
 *    - Total return
 *    - Sharpe ratio (risk-adjusted)
 *    - Max drawdown (risk measure)
 *    - Win rate, profit factor
 *
 * 6. Common Pitfalls:
 *    - Look-ahead bias
 *    - Survivorship bias
 *    - Overfitting
 *    - Ignoring transaction costs
 *    - Unrealistic fills
 *
 * 7. Enhancements:
 *    - Multi-asset support
 *    - Order types (stop, limit)
 *    - Partial fills
 *    - Slippage models
 *    - Market impact
 *    - Risk management
 *    - Position sizing
 *
 * 8. Interview Questions:
 *    Q: How to prevent look-ahead bias?
 *    A: Process data chronologically
 *       Only use past data for decisions
 *       Realistic order execution times
 *
 *    Q: How to handle multiple strategies?
 *    A: Each strategy generates orders
 *       Portfolio allocates capital
 *       Risk checks before execution
 *
 *    Q: What makes a good backtest?
 *    A: Realistic costs and slippage
 *       Out-of-sample testing
 *       Multiple market conditions
 *       Robustness checks
 *
 * Time Complexity:
 * - O(n * s) where n = bars, s = strategies
 *
 * Space Complexity:
 * - O(n + p) where p = positions
 */

int main() {
    demonstrate_backtesting();
    return 0;
}

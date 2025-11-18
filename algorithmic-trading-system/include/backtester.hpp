#pragma once

#include "order.hpp"
#include "market_data_handler.hpp"
#include "order_manager.hpp"
#include "strategy.hpp"
#include "risk_manager.hpp"
#include <vector>
#include <string>
#include <fstream>

namespace trading {

// Backtest configuration
struct BacktestConfig {
    Timestamp start_date;
    Timestamp end_date;
    double initial_capital{100000.0};
    double commission_rate{0.001};  // 0.1%
    double slippage_bps{1.0};       // 1 basis point
    bool enable_shorting{true};
    RiskLimits risk_limits;
};

// Backtest results
struct BacktestResults {
    double final_equity{0.0};
    double total_return{0.0};
    double annualized_return{0.0};
    double sharpe_ratio{0.0};
    double sortino_ratio{0.0};
    double max_drawdown{0.0};
    double calmar_ratio{0.0};

    size_t total_trades{0};
    size_t winning_trades{0};
    size_t losing_trades{0};
    double win_rate{0.0};
    double avg_win{0.0};
    double avg_loss{0.0};
    double profit_factor{0.0};

    double max_position_size{0.0};
    double avg_position_holding_time{0.0};

    std::vector<double> equity_curve;
    std::vector<double> returns;
    std::vector<double> drawdown_curve;

    void print() const;
    void save_to_csv(const std::string& filename) const;
};

// Trade record for backtesting
struct BacktestTrade {
    Timestamp entry_time;
    Timestamp exit_time;
    Symbol symbol;
    OrderSide side;
    Price entry_price;
    Price exit_price;
    Quantity quantity;
    double pnl;
    double return_pct;
    std::string strategy;
};

// Backtester
class Backtester {
public:
    Backtester(const BacktestConfig& config);

    // Add historical data
    void load_market_data(const std::string& filename);
    void add_market_data(const MarketData& data);

    // Add strategies
    void add_strategy(std::shared_ptr<IStrategy> strategy);

    // Run backtest
    BacktestResults run();

    // Get detailed trade history
    std::vector<BacktestTrade> get_trade_history() const { return trade_history_; }

private:
    void initialize();
    void process_bar(const MarketData& data);
    void calculate_results();
    double calculate_sharpe_ratio(const std::vector<double>& returns);
    double calculate_sortino_ratio(const std::vector<double>& returns);
    double calculate_max_drawdown(const std::vector<double>& equity_curve);

    BacktestConfig config_;
    BacktestResults results_;

    std::shared_ptr<Portfolio> portfolio_;
    std::shared_ptr<OrderManager> order_manager_;
    std::shared_ptr<MarketDataHandler> market_data_handler_;
    std::shared_ptr<SimulatedExecutionEngine> execution_engine_;
    std::shared_ptr<RiskManager> risk_manager_;
    std::shared_ptr<StrategyManager> strategy_manager_;

    std::vector<MarketData> historical_data_;
    std::vector<BacktestTrade> trade_history_;
    std::vector<double> daily_equity_;

    std::unordered_map<OrderId, Timestamp> order_entry_times_;
};

// Walk-forward optimization
class WalkForwardOptimizer {
public:
    WalkForwardOptimizer(size_t in_sample_periods, size_t out_sample_periods);

    struct OptimizationResult {
        std::unordered_map<std::string, double> best_parameters;
        double in_sample_performance;
        double out_sample_performance;
        BacktestResults results;
    };

    std::vector<OptimizationResult> optimize(
        const std::vector<MarketData>& data,
        const std::function<std::shared_ptr<IStrategy>(
            const std::unordered_map<std::string, double>&)>& strategy_factory,
        const std::vector<std::unordered_map<std::string, double>>& parameter_sets
    );

private:
    size_t in_sample_periods_;
    size_t out_sample_periods_;
};

// Monte Carlo simulator for strategy robustness testing
class MonteCarloSimulator {
public:
    struct SimulationConfig {
        size_t num_simulations{1000};
        double price_randomness{0.01};  // 1% random noise
        double volume_randomness{0.1};  // 10% random noise
        bool resample_returns{false};
    };

    MonteCarloSimulator(const SimulationConfig& config);

    std::vector<BacktestResults> run_simulations(
        const std::vector<MarketData>& base_data,
        std::shared_ptr<IStrategy> strategy,
        const BacktestConfig& backtest_config
    );

    // Risk of ruin calculation
    double calculate_risk_of_ruin(const std::vector<BacktestResults>& results,
                                  double ruin_threshold);

private:
    SimulationConfig config_;
};

} // namespace trading

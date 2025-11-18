#pragma once

#include "order.hpp"
#include "market_data_handler.hpp"
#include "order_manager.hpp"
#include <vector>
#include <memory>
#include <string>

namespace trading {

// Signal structure
struct Signal {
    Symbol symbol;
    double strength;  // -1.0 (strong sell) to 1.0 (strong buy)
    double confidence; // 0.0 to 1.0
    Timestamp timestamp;
    std::string reason;

    Signal() : strength(0.0), confidence(0.0), timestamp(now()) {}
    Signal(const Symbol& sym, double str, double conf)
        : symbol(sym), strength(str), confidence(conf), timestamp(now()) {}

    bool is_buy() const { return strength > 0.0; }
    bool is_sell() const { return strength < 0.0; }
    bool is_strong() const { return std::abs(strength) > 0.5 && confidence > 0.7; }
};

// Strategy base class
class IStrategy {
public:
    virtual ~IStrategy() = default;

    virtual void initialize() = 0;
    virtual void on_market_data(const MarketData& data) = 0;
    virtual void on_tick(const TickData& tick) = 0;
    virtual std::vector<Signal> generate_signals() = 0;
    virtual std::string get_name() const = 0;

    void set_enabled(bool enabled) { enabled_ = enabled; }
    bool is_enabled() const { return enabled_; }

protected:
    bool enabled_{true};
};

// Moving Average Crossover Strategy
class MovingAverageCrossoverStrategy : public IStrategy {
public:
    MovingAverageCrossoverStrategy(const std::vector<Symbol>& symbols,
                                   size_t short_period, size_t long_period);

    void initialize() override;
    void on_market_data(const MarketData& data) override;
    void on_tick(const TickData& tick) override;
    std::vector<Signal> generate_signals() override;
    std::string get_name() const override { return "MA_Crossover"; }

private:
    double calculate_ma(const std::deque<double>& prices, size_t period);

    std::vector<Symbol> symbols_;
    size_t short_period_;
    size_t long_period_;
    std::unordered_map<Symbol, std::deque<double>> price_history_;
    std::unordered_map<Symbol, Signal> current_signals_;
};

// Mean Reversion Strategy
class MeanReversionStrategy : public IStrategy {
public:
    MeanReversionStrategy(const std::vector<Symbol>& symbols,
                         size_t lookback_period, double entry_threshold,
                         double exit_threshold);

    void initialize() override;
    void on_market_data(const MarketData& data) override;
    void on_tick(const TickData& tick) override;
    std::vector<Signal> generate_signals() override;
    std::string get_name() const override { return "Mean_Reversion"; }

private:
    double calculate_zscore(const Symbol& symbol, double current_price);

    std::vector<Symbol> symbols_;
    size_t lookback_period_;
    double entry_threshold_;
    double exit_threshold_;
    std::unordered_map<Symbol, std::deque<double>> price_history_;
    std::unordered_map<Symbol, double> entry_prices_;
};

// Momentum Strategy
class MomentumStrategy : public IStrategy {
public:
    MomentumStrategy(const std::vector<Symbol>& symbols, size_t lookback_period);

    void initialize() override;
    void on_market_data(const MarketData& data) override;
    void on_tick(const TickData& tick) override;
    std::vector<Signal> generate_signals() override;
    std::string get_name() const override { return "Momentum"; }

private:
    double calculate_momentum(const Symbol& symbol);

    std::vector<Symbol> symbols_;
    size_t lookback_period_;
    std::unordered_map<Symbol, std::deque<double>> price_history_;
};

// ML-based strategy using external model
class MLStrategy : public IStrategy {
public:
    MLStrategy(const std::vector<Symbol>& symbols, const std::string& model_path);
    ~MLStrategy() override;

    void initialize() override;
    void on_market_data(const MarketData& data) override;
    void on_tick(const TickData& tick) override;
    std::vector<Signal> generate_signals() override;
    std::string get_name() const override { return "ML_Strategy"; }

    // Feature engineering
    std::vector<double> extract_features(const Symbol& symbol);

    // Model management
    bool load_model(const std::string& model_path);
    void reload_model();

private:
    std::vector<double> predict(const std::vector<double>& features);
    void update_features(const Symbol& symbol, const MarketData& data);

    std::vector<Symbol> symbols_;
    std::string model_path_;
    void* model_handle_{nullptr};  // Placeholder for model

    // Feature storage
    struct Features {
        std::deque<double> prices;
        std::deque<double> volumes;
        std::deque<double> spreads;
        double rsi{0.0};
        double macd{0.0};
        double bollinger_upper{0.0};
        double bollinger_lower{0.0};
    };
    std::unordered_map<Symbol, Features> features_;

    static constexpr size_t FEATURE_WINDOW = 100;
};

// Strategy manager to coordinate multiple strategies
class StrategyManager {
public:
    StrategyManager(std::shared_ptr<MarketDataHandler> market_data_handler,
                   std::shared_ptr<OrderManager> order_manager);

    // Strategy management
    void add_strategy(std::shared_ptr<IStrategy> strategy);
    void remove_strategy(const std::string& name);
    void enable_strategy(const std::string& name);
    void disable_strategy(const std::string& name);

    // Execution
    void start();
    void stop();
    void process_signals();

    // Configuration
    void set_position_sizer(const std::function<Quantity(const Signal&, double capital)>& sizer) {
        position_sizer_ = sizer;
    }

    // Statistics
    std::vector<Signal> get_recent_signals(size_t count) const;
    size_t get_total_signals() const { return total_signals_.load(); }

private:
    void on_market_data_update(const MarketData& data);
    void on_tick_update(const TickData& tick);
    void execute_signal(const Signal& signal);
    Quantity calculate_position_size(const Signal& signal);

    std::shared_ptr<MarketDataHandler> market_data_handler_;
    std::shared_ptr<OrderManager> order_manager_;

    std::vector<std::shared_ptr<IStrategy>> strategies_;
    mutable std::mutex strategies_mutex_;

    std::atomic<bool> running_{false};
    std::thread processing_thread_;

    // Position sizing
    std::function<Quantity(const Signal&, double capital)> position_sizer_;

    // Signal history
    std::deque<Signal> signal_history_;
    mutable std::mutex history_mutex_;
    std::atomic<size_t> total_signals_{0};

    static constexpr size_t MAX_SIGNAL_HISTORY = 10000;
};

// Advanced indicators
class TechnicalIndicators {
public:
    static double calculate_rsi(const std::deque<double>& prices, size_t period = 14);
    static double calculate_macd(const std::deque<double>& prices);
    static std::pair<double, double> calculate_bollinger_bands(
        const std::deque<double>& prices, size_t period = 20, double num_std = 2.0);
    static double calculate_atr(const std::deque<MarketData>& data, size_t period = 14);
    static double calculate_vwap(const std::deque<TickData>& ticks);

private:
    static double calculate_mean(const std::deque<double>& data);
    static double calculate_std_dev(const std::deque<double>& data);
};

} // namespace trading

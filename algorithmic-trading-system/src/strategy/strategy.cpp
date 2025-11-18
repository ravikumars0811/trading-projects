#include "../../include/strategy.hpp"
#include <numeric>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace trading {

// TechnicalIndicators implementation
double TechnicalIndicators::calculate_rsi(const std::deque<double>& prices, size_t period) {
    if (prices.size() < period + 1) return 50.0;

    double gains = 0.0;
    double losses = 0.0;

    for (size_t i = prices.size() - period; i < prices.size(); ++i) {
        double change = prices[i] - prices[i-1];
        if (change > 0) {
            gains += change;
        } else {
            losses += std::abs(change);
        }
    }

    double avg_gain = gains / period;
    double avg_loss = losses / period;

    if (avg_loss == 0.0) return 100.0;

    double rs = avg_gain / avg_loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

double TechnicalIndicators::calculate_macd(const std::deque<double>& prices) {
    if (prices.size() < 26) return 0.0;

    // Calculate 12-period EMA
    double ema12 = prices[0];
    double multiplier12 = 2.0 / (12.0 + 1.0);
    for (size_t i = 1; i < prices.size(); ++i) {
        ema12 = (prices[i] - ema12) * multiplier12 + ema12;
    }

    // Calculate 26-period EMA
    double ema26 = prices[0];
    double multiplier26 = 2.0 / (26.0 + 1.0);
    for (size_t i = 1; i < prices.size(); ++i) {
        ema26 = (prices[i] - ema26) * multiplier26 + ema26;
    }

    return ema12 - ema26;
}

std::pair<double, double> TechnicalIndicators::calculate_bollinger_bands(
    const std::deque<double>& prices, size_t period, double num_std) {

    if (prices.size() < period) {
        return {0.0, 0.0};
    }

    double mean = calculate_mean(prices);
    double std_dev = calculate_std_dev(prices);

    return {mean + num_std * std_dev, mean - num_std * std_dev};
}

double TechnicalIndicators::calculate_atr(const std::deque<MarketData>& data, size_t period) {
    if (data.size() < period) return 0.0;

    double atr = 0.0;
    for (size_t i = data.size() - period; i < data.size(); ++i) {
        double true_range = data[i].ask - data[i].bid;
        atr += true_range;
    }

    return atr / period;
}

double TechnicalIndicators::calculate_vwap(const std::deque<TickData>& ticks) {
    if (ticks.empty()) return 0.0;

    double sum_pv = 0.0;
    double sum_v = 0.0;

    for (const auto& tick : ticks) {
        sum_pv += tick.price * tick.quantity;
        sum_v += tick.quantity;
    }

    return sum_v > 0 ? sum_pv / sum_v : 0.0;
}

double TechnicalIndicators::calculate_mean(const std::deque<double>& data) {
    return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
}

double TechnicalIndicators::calculate_std_dev(const std::deque<double>& data) {
    double mean = calculate_mean(data);
    double sq_sum = 0.0;
    for (double val : data) {
        sq_sum += (val - mean) * (val - mean);
    }
    return std::sqrt(sq_sum / data.size());
}

// MovingAverageCrossoverStrategy implementation
MovingAverageCrossoverStrategy::MovingAverageCrossoverStrategy(
    const std::vector<Symbol>& symbols, size_t short_period, size_t long_period)
    : symbols_(symbols), short_period_(short_period), long_period_(long_period) {}

void MovingAverageCrossoverStrategy::initialize() {
    for (const auto& symbol : symbols_) {
        price_history_[symbol] = std::deque<double>();
        current_signals_[symbol] = Signal();
    }
}

void MovingAverageCrossoverStrategy::on_market_data(const MarketData& data) {
    auto& history = price_history_[data.symbol];
    history.push_back(data.mid_price());

    if (history.size() > long_period_ * 2) {
        history.pop_front();
    }
}

void MovingAverageCrossoverStrategy::on_tick(const TickData& tick) {
    // Can be used for more granular analysis
}

std::vector<Signal> MovingAverageCrossoverStrategy::generate_signals() {
    std::vector<Signal> signals;

    for (const auto& symbol : symbols_) {
        const auto& history = price_history_[symbol];
        if (history.size() < long_period_) continue;

        double short_ma = calculate_ma(history, short_period_);
        double long_ma = calculate_ma(history, long_period_);

        Signal signal(symbol, 0.0, 0.0);

        if (short_ma > long_ma) {
            signal.strength = 0.8;
            signal.confidence = 0.75;
            signal.reason = "Short MA crossed above Long MA";
        } else if (short_ma < long_ma) {
            signal.strength = -0.8;
            signal.confidence = 0.75;
            signal.reason = "Short MA crossed below Long MA";
        }

        if (std::abs(signal.strength) > 0.1) {
            signals.push_back(signal);
        }
    }

    return signals;
}

double MovingAverageCrossoverStrategy::calculate_ma(const std::deque<double>& prices, size_t period) {
    if (prices.size() < period) return 0.0;

    double sum = 0.0;
    for (size_t i = prices.size() - period; i < prices.size(); ++i) {
        sum += prices[i];
    }
    return sum / period;
}

// MeanReversionStrategy implementation
MeanReversionStrategy::MeanReversionStrategy(
    const std::vector<Symbol>& symbols, size_t lookback_period,
    double entry_threshold, double exit_threshold)
    : symbols_(symbols), lookback_period_(lookback_period),
      entry_threshold_(entry_threshold), exit_threshold_(exit_threshold) {}

void MeanReversionStrategy::initialize() {
    for (const auto& symbol : symbols_) {
        price_history_[symbol] = std::deque<double>();
    }
}

void MeanReversionStrategy::on_market_data(const MarketData& data) {
    auto& history = price_history_[data.symbol];
    history.push_back(data.mid_price());

    if (history.size() > lookback_period_ * 2) {
        history.pop_front();
    }
}

void MeanReversionStrategy::on_tick(const TickData& tick) {}

std::vector<Signal> MeanReversionStrategy::generate_signals() {
    std::vector<Signal> signals;

    for (const auto& symbol : symbols_) {
        const auto& history = price_history_[symbol];
        if (history.size() < lookback_period_) continue;

        double current_price = history.back();
        double zscore = calculate_zscore(symbol, current_price);

        Signal signal(symbol, 0.0, 0.0);

        if (zscore < -entry_threshold_) {
            signal.strength = 0.7;
            signal.confidence = std::min(std::abs(zscore) / 3.0, 0.95);
            signal.reason = "Price below mean (z-score: " + std::to_string(zscore) + ")";
        } else if (zscore > entry_threshold_) {
            signal.strength = -0.7;
            signal.confidence = std::min(std::abs(zscore) / 3.0, 0.95);
            signal.reason = "Price above mean (z-score: " + std::to_string(zscore) + ")";
        }

        if (std::abs(signal.strength) > 0.1) {
            signals.push_back(signal);
        }
    }

    return signals;
}

double MeanReversionStrategy::calculate_zscore(const Symbol& symbol, double current_price) {
    const auto& history = price_history_[symbol];
    if (history.size() < lookback_period_) return 0.0;

    double mean = TechnicalIndicators::calculate_mean(history);
    double std_dev = TechnicalIndicators::calculate_std_dev(history);

    if (std_dev < 1e-6) return 0.0;

    return (current_price - mean) / std_dev;
}

// MLStrategy implementation
MLStrategy::MLStrategy(const std::vector<Symbol>& symbols, const std::string& model_path)
    : symbols_(symbols), model_path_(model_path) {}

MLStrategy::~MLStrategy() {
    // Clean up model resources
}

void MLStrategy::initialize() {
    load_model(model_path_);
    for (const auto& symbol : symbols_) {
        features_[symbol] = Features();
    }
}

void MLStrategy::on_market_data(const MarketData& data) {
    update_features(data.symbol, data);
}

void MLStrategy::on_tick(const TickData& tick) {
    auto& features = features_[tick.symbol];
    features.volumes.push_back(tick.quantity);
    if (features.volumes.size() > FEATURE_WINDOW) {
        features.volumes.pop_front();
    }
}

std::vector<Signal> MLStrategy::generate_signals() {
    std::vector<Signal> signals;

    for (const auto& symbol : symbols_) {
        auto feature_vec = extract_features(symbol);
        if (feature_vec.empty()) continue;

        auto predictions = predict(feature_vec);
        if (predictions.empty()) continue;

        // predictions[0] = probability of up move
        double signal_strength = (predictions[0] - 0.5) * 2.0; // Scale to [-1, 1]

        Signal signal(symbol, signal_strength, std::abs(predictions[0] - 0.5) * 2.0);
        signal.reason = "ML model prediction: " + std::to_string(predictions[0]);

        if (std::abs(signal.strength) > 0.3) {
            signals.push_back(signal);
        }
    }

    return signals;
}

std::vector<double> MLStrategy::extract_features(const Symbol& symbol) {
    auto& f = features_[symbol];
    if (f.prices.size() < 50) return {};

    std::vector<double> features;

    // Price-based features
    features.push_back(f.rsi);
    features.push_back(f.macd);

    // Recent price changes
    for (size_t i = f.prices.size() - 10; i < f.prices.size(); ++i) {
        double ret = (f.prices[i] - f.prices[i-1]) / f.prices[i-1];
        features.push_back(ret);
    }

    // Volume features
    if (!f.volumes.empty()) {
        double avg_volume = std::accumulate(f.volumes.begin(), f.volumes.end(), 0.0) / f.volumes.size();
        features.push_back(avg_volume);
    }

    // Bollinger band position
    double current_price = f.prices.back();
    if (f.bollinger_upper > f.bollinger_lower) {
        double bb_position = (current_price - f.bollinger_lower) / (f.bollinger_upper - f.bollinger_lower);
        features.push_back(bb_position);
    }

    return features;
}

bool MLStrategy::load_model(const std::string& model_path) {
    // In production, load model using TensorFlow C++ API, ONNX Runtime, or similar
    std::cout << "Loading ML model from: " << model_path << "\n";
    // model_handle_ = load_tensorflow_model(model_path);
    return true;
}

void MLStrategy::reload_model() {
    load_model(model_path_);
}

std::vector<double> MLStrategy::predict(const std::vector<double>& features) {
    // In production, run inference using the loaded model
    // For now, return a dummy prediction
    return {0.55}; // Slight bullish bias
}

void MLStrategy::update_features(const Symbol& symbol, const MarketData& data) {
    auto& f = features_[symbol];

    f.prices.push_back(data.mid_price());
    f.spreads.push_back(data.spread());

    if (f.prices.size() > FEATURE_WINDOW) {
        f.prices.pop_front();
    }
    if (f.spreads.size() > FEATURE_WINDOW) {
        f.spreads.pop_front();
    }

    // Calculate indicators
    if (f.prices.size() >= 20) {
        f.rsi = TechnicalIndicators::calculate_rsi(f.prices);
        f.macd = TechnicalIndicators::calculate_macd(f.prices);
        auto [upper, lower] = TechnicalIndicators::calculate_bollinger_bands(f.prices);
        f.bollinger_upper = upper;
        f.bollinger_lower = lower;
    }
}

// StrategyManager implementation
StrategyManager::StrategyManager(
    std::shared_ptr<MarketDataHandler> market_data_handler,
    std::shared_ptr<OrderManager> order_manager)
    : market_data_handler_(market_data_handler), order_manager_(order_manager) {

    // Default position sizer (1% of capital per trade)
    position_sizer_ = [this](const Signal& signal, double capital) {
        return (capital * 0.01) / 100.0; // Assuming $100 per unit
    };
}

void StrategyManager::add_strategy(std::shared_ptr<IStrategy> strategy) {
    std::lock_guard lock(strategies_mutex_);
    strategies_.push_back(strategy);
    strategy->initialize();
    std::cout << "Strategy added: " << strategy->get_name() << "\n";
}

void StrategyManager::remove_strategy(const std::string& name) {
    std::lock_guard lock(strategies_mutex_);
    strategies_.erase(
        std::remove_if(strategies_.begin(), strategies_.end(),
            [&name](const auto& s) { return s->get_name() == name; }),
        strategies_.end()
    );
}

void StrategyManager::enable_strategy(const std::string& name) {
    std::lock_guard lock(strategies_mutex_);
    for (auto& strategy : strategies_) {
        if (strategy->get_name() == name) {
            strategy->set_enabled(true);
        }
    }
}

void StrategyManager::disable_strategy(const std::string& name) {
    std::lock_guard lock(strategies_mutex_);
    for (auto& strategy : strategies_) {
        if (strategy->get_name() == name) {
            strategy->set_enabled(false);
        }
    }
}

void StrategyManager::start() {
    if (running_.load()) return;

    running_.store(true);

    // Register callbacks
    market_data_handler_->register_market_data_callback(
        [this](const MarketData& data) { on_market_data_update(data); }
    );

    market_data_handler_->register_tick_callback(
        [this](const TickData& tick) { on_tick_update(tick); }
    );

    // Start processing thread
    processing_thread_ = std::thread([this]() {
        while (running_.load()) {
            process_signals();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    std::cout << "StrategyManager started\n";
}

void StrategyManager::stop() {
    if (!running_.load()) return;

    running_.store(false);

    if (processing_thread_.joinable()) {
        processing_thread_.join();
    }

    std::cout << "StrategyManager stopped\n";
}

void StrategyManager::process_signals() {
    std::lock_guard lock(strategies_mutex_);

    for (auto& strategy : strategies_) {
        if (!strategy->is_enabled()) continue;

        try {
            auto signals = strategy->generate_signals();
            for (const auto& signal : signals) {
                if (signal.is_strong()) {
                    execute_signal(signal);

                    // Store signal
                    {
                        std::lock_guard h_lock(history_mutex_);
                        signal_history_.push_back(signal);
                        if (signal_history_.size() > MAX_SIGNAL_HISTORY) {
                            signal_history_.pop_front();
                        }
                    }
                    total_signals_.fetch_add(1);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in strategy " << strategy->get_name()
                     << ": " << e.what() << "\n";
        }
    }
}

void StrategyManager::on_market_data_update(const MarketData& data) {
    std::lock_guard lock(strategies_mutex_);
    for (auto& strategy : strategies_) {
        if (strategy->is_enabled()) {
            strategy->on_market_data(data);
        }
    }
}

void StrategyManager::on_tick_update(const TickData& tick) {
    std::lock_guard lock(strategies_mutex_);
    for (auto& strategy : strategies_) {
        if (strategy->is_enabled()) {
            strategy->on_tick(tick);
        }
    }
}

void StrategyManager::execute_signal(const Signal& signal) {
    Quantity quantity = calculate_position_size(signal);

    OrderSide side = signal.is_buy() ? OrderSide::BUY : OrderSide::SELL;
    Order order(signal.symbol, side, OrderType::MARKET, 0.0, quantity);
    order.strategy_id = "StrategyManager";

    order_manager_->submit_order(order);

    std::cout << "Executing signal: " << signal.symbol << " "
              << (signal.is_buy() ? "BUY" : "SELL") << " " << quantity
              << " (strength: " << signal.strength << ", confidence: "
              << signal.confidence << ")\n";
}

Quantity StrategyManager::calculate_position_size(const Signal& signal) {
    double capital = order_manager_->get_portfolio()->get_cash();
    return position_sizer_(signal, capital);
}

std::vector<Signal> StrategyManager::get_recent_signals(size_t count) const {
    std::lock_guard lock(history_mutex_);
    size_t start = signal_history_.size() > count ? signal_history_.size() - count : 0;
    return std::vector<Signal>(signal_history_.begin() + start, signal_history_.end());
}

} // namespace trading

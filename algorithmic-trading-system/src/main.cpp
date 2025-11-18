#include "../include/market_data_handler.hpp"
#include "../include/order_manager.hpp"
#include "../include/strategy.hpp"
#include "../include/risk_manager.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>

using namespace trading;

// Global flag for graceful shutdown
std::atomic<bool> running{true};

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down...\n";
    running.store(false);
}

int main(int argc, char* argv[]) {
    std::cout << "=================================================\n";
    std::cout << "   Algorithmic Trading System - Production Mode\n";
    std::cout << "=================================================\n\n";

    // Register signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    try {
        // Configuration
        const double INITIAL_CAPITAL = 100000.0;
        const std::vector<Symbol> SYMBOLS = {"AAPL", "MSFT", "GOOGL", "AMZN", "TSLA"};

        // Initialize components
        std::cout << "Initializing trading system...\n";

        // Market data handler
        auto market_data_handler = std::make_shared<MarketDataHandler>();
        market_data_handler->start();

        // Portfolio and order manager
        auto portfolio = std::make_shared<Portfolio>(INITIAL_CAPITAL);
        auto order_manager = std::make_shared<OrderManager>(portfolio);

        // Execution engine (simulated for demo)
        auto execution_engine = std::make_unique<SimulatedExecutionEngine>(*order_manager);
        execution_engine->set_slippage(1.0);  // 1 bps slippage
        execution_engine->set_latency(1000);  // 1ms latency

        // Risk manager
        RiskLimits risk_limits;
        risk_limits.max_position_size = 50000.0;
        risk_limits.max_daily_loss = 5000.0;
        risk_limits.max_drawdown = 0.15;
        risk_limits.max_leverage = 2.0;

        auto risk_manager = std::make_shared<RiskManager>(
            order_manager, market_data_handler, risk_limits);

        // Strategy manager
        auto strategy_manager = std::make_shared<StrategyManager>(
            market_data_handler, order_manager);

        // Add strategies
        std::cout << "Adding trading strategies...\n";

        // 1. Moving Average Crossover
        auto ma_strategy = std::make_shared<MovingAverageCrossoverStrategy>(
            SYMBOLS, 20, 50);
        strategy_manager->add_strategy(ma_strategy);

        // 2. Mean Reversion
        auto mr_strategy = std::make_shared<MeanReversionStrategy>(
            SYMBOLS, 50, 2.0, 0.5);
        strategy_manager->add_strategy(mr_strategy);

        // 3. ML Strategy (if model exists)
        auto ml_strategy = std::make_shared<MLStrategy>(
            SYMBOLS, "../python/models/lstm_predictor.pth");
        strategy_manager->add_strategy(ml_strategy);

        // Set up callbacks
        order_manager->register_order_callback([](const Order& order) {
            std::cout << "[ORDER] " << order.symbol << " "
                     << to_string(order.side) << " "
                     << order.quantity << " @ " << order.price
                     << " - " << to_string(order.status) << "\n";
        });

        order_manager->register_trade_callback([](const Trade& trade) {
            std::cout << "[TRADE] " << trade.symbol << " "
                     << to_string(trade.side) << " "
                     << trade.quantity << " @ " << trade.price << "\n";
        });

        risk_manager->register_violation_callback([](const std::string& violation) {
            std::cerr << "[RISK VIOLATION] " << violation << "\n";
        });

        // Market data callback to update execution engine
        market_data_handler->register_market_data_callback(
            [&execution_engine](const MarketData& data) {
                execution_engine->process_market_data(data);
            });

        // Connect to market data feed
        std::cout << "Connecting to market data feed...\n";

        // Example: Connect to Alpaca (requires API keys)
        // const char* api_key = std::getenv("ALPACA_API_KEY");
        // const char* api_secret = std::getenv("ALPACA_API_SECRET");
        //
        // if (api_key && api_secret) {
        //     auto alpaca_feed = std::make_unique<AlpacaMarketDataFeed>(
        //         api_key, api_secret, *market_data_handler);
        //     alpaca_feed->connect();
        //     alpaca_feed->subscribe(SYMBOLS);
        // }

        // For demo: Subscribe to symbols
        market_data_handler->subscribe_all(SYMBOLS);

        // Start strategy manager
        strategy_manager->start();

        std::cout << "\n";
        std::cout << "Trading system is now running...\n";
        std::cout << "Initial Capital: $" << INITIAL_CAPITAL << "\n";
        std::cout << "Symbols: ";
        for (const auto& symbol : SYMBOLS) {
            std::cout << symbol << " ";
        }
        std::cout << "\n\n";
        std::cout << "Press Ctrl+C to stop\n\n";

        // Main loop
        auto last_status_time = std::chrono::system_clock::now();

        while (running.load()) {
            // Print status every 10 seconds
            auto now_time = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now_time - last_status_time).count();

            if (elapsed >= 10) {
                std::cout << "--- Status Update ---\n";
                std::cout << "Market Data Updates/sec: "
                         << market_data_handler->get_updates_per_second() << "\n";
                std::cout << "Active Orders: "
                         << order_manager->get_active_order_count() << "\n";

                auto positions = portfolio->get_all_positions();
                std::cout << "Open Positions: " << positions.size() << "\n";

                // Calculate current equity
                std::unordered_map<Symbol, Price> current_prices;
                for (const auto& symbol : SYMBOLS) {
                    MarketData md;
                    if (market_data_handler->get_market_data(symbol, md)) {
                        current_prices[symbol] = md.mid_price();
                    }
                }

                double equity = portfolio->get_equity(current_prices);
                double pnl = portfolio->get_total_pnl(current_prices);

                std::cout << "Current Equity: $" << equity << "\n";
                std::cout << "Total P&L: $" << pnl
                         << " (" << (pnl / INITIAL_CAPITAL * 100.0) << "%)\n";

                // Risk metrics
                auto risk_metrics = risk_manager->calculate_risk_metrics();
                std::cout << "Drawdown: " << (risk_metrics.current_drawdown * 100.0) << "%\n";
                std::cout << "Leverage: " << risk_metrics.current_leverage << "x\n";

                std::cout << "\n";
                last_status_time = now_time;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Shutdown
        std::cout << "\nShutting down trading system...\n";
        strategy_manager->stop();
        market_data_handler->stop();

        // Final report
        std::cout << "\n=== Final Report ===\n";
        std::unordered_map<Symbol, Price> final_prices;
        for (const auto& symbol : SYMBOLS) {
            MarketData md;
            if (market_data_handler->get_market_data(symbol, md)) {
                final_prices[symbol] = md.mid_price();
            }
        }

        double final_equity = portfolio->get_equity(final_prices);
        double total_pnl = portfolio->get_total_pnl(final_prices);

        std::cout << "Initial Capital: $" << INITIAL_CAPITAL << "\n";
        std::cout << "Final Equity: $" << final_equity << "\n";
        std::cout << "Total P&L: $" << total_pnl
                 << " (" << (total_pnl / INITIAL_CAPITAL * 100.0) << "%)\n";
        std::cout << "Total Orders: " << order_manager->get_total_orders() << "\n";
        std::cout << "Total Commission: $" << portfolio->get_total_commission() << "\n";

        std::cout << "\nTrading system stopped successfully.\n";

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

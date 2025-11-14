#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

// Core
#include "core/config.hpp"
#include "core/logger.hpp"
#include "core/timer.hpp"

// Market Data
#include "market_data/market_data_handler.hpp"
#include "market_data/order_book.hpp"

// OMS
#include "oms/order_manager.hpp"
#include "oms/position_manager.hpp"

// Strategy
#include "strategy/market_making_strategy.hpp"
#include "strategy/stat_arb_strategy.hpp"

// Gateway
#include "gateway/exchange_gateway.hpp"

// Risk
#include "risk/risk_manager.hpp"

// Metrics
#include "metrics/performance_monitor.hpp"

using namespace hft;

// Global flag for graceful shutdown
std::atomic<bool> running{true};

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down gracefully...\n";
    running = false;
}

class HFTSystem {
public:
    HFTSystem() :
        order_manager_(),
        position_manager_(),
        risk_manager_(position_manager_),
        market_data_handler_(),
        exchange_gateway_(order_manager_) {}

    bool initialize(const std::string& config_file) {
        try {
            // Load configuration
            LOG_INFO("Loading configuration from: ", config_file);
            core::Config::instance().load(config_file);

            // Initialize logger
            std::string log_file = core::Config::instance().getString("log_file", "hft_system.log");
            core::Logger::instance().init(log_file, core::LogLevel::INFO);
            LOG_INFO("HFT System initializing...");

            // Setup risk limits
            setupRiskLimits();

            // Initialize market data handler
            market_data_handler_.start();

            // Connect to exchange
            std::string exchange_host = core::Config::instance().getString("exchange_host", "localhost");
            int exchange_port = core::Config::instance().getInt("exchange_port", 9001);
            exchange_gateway_.connect(exchange_host, exchange_port);

            // Initialize strategies
            initializeStrategies();

            LOG_INFO("HFT System initialized successfully");
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Initialization failed: " << e.what() << std::endl;
            return false;
        }
    }

    void run() {
        LOG_INFO("HFT System running...");

        // Start simulated market data feed
        std::string symbol = core::Config::instance().getString("symbol", "AAPL");
        market_data::SimulatedFeed feed(market_data_handler_);
        feed.setSymbol(symbol);
        feed.setTickInterval(1000);  // 1ms between ticks
        feed.start();

        // Subscribe to market data updates
        market_data_handler_.subscribe(symbol, [this](const std::string& sym, const market_data::OrderBook& ob) {
            onMarketData(sym, ob);
        });

        // Performance monitoring loop
        auto last_print_time = std::chrono::steady_clock::now();

        while (running) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_print_time).count();

            // Print metrics every 10 seconds
            if (elapsed >= 10) {
                printStatus();
                last_print_time = now;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Shutdown
        feed.stop();
        shutdown();
    }

    void shutdown() {
        LOG_INFO("HFT System shutting down...");

        // Stop strategies
        for (auto& strategy : strategies_) {
            strategy->stop();
            strategy->shutdown();
        }

        // Disconnect from exchange
        exchange_gateway_.disconnect();

        // Stop market data handler
        market_data_handler_.stop();

        // Print final metrics
        printStatus();

        // Shutdown logger
        core::Logger::instance().shutdown();

        std::cout << "HFT System shutdown complete\n";
    }

private:
    void setupRiskLimits() {
        risk::RiskLimits limits;
        limits.max_position = core::Config::instance().getInt("max_position", 1000);
        limits.max_order_size = core::Config::instance().getInt("max_order_size", 500);
        limits.max_loss_per_symbol = core::Config::instance().getDouble("max_loss", 10000.0);
        limits.price_collar_percent = core::Config::instance().getDouble("price_collar", 5.0);
        limits.max_orders_per_second = core::Config::instance().getInt("max_orders_per_sec", 100);

        risk_manager_.setGlobalRiskLimits(limits);
        LOG_INFO("Risk limits configured");
    }

    void initializeStrategies() {
        std::string strategy_type = core::Config::instance().getString("strategy", "market_making");
        std::string symbol = core::Config::instance().getString("symbol", "AAPL");

        if (strategy_type == "market_making") {
            strategy::MarketMakingStrategy::Parameters params;
            params.spread_bps = core::Config::instance().getDouble("spread_bps", 10.0);
            params.quote_size = core::Config::instance().getInt("quote_size", 100);
            params.max_position = core::Config::instance().getInt("max_position", 1000);

            auto strategy = std::make_unique<strategy::MarketMakingStrategy>(
                order_manager_, position_manager_, params);
            strategy->setSymbol(symbol);
            strategy->initialize();
            strategy->start();

            strategies_.push_back(std::move(strategy));
            LOG_INFO("Market Making strategy initialized for ", symbol);
        }
        else if (strategy_type == "stat_arb") {
            strategy::StatArbStrategy::Parameters params;
            params.lookback_period = core::Config::instance().getInt("lookback_period", 100);
            params.entry_threshold = core::Config::instance().getDouble("entry_threshold", 2.0);
            params.exit_threshold = core::Config::instance().getDouble("exit_threshold", 0.5);

            auto strategy = std::make_unique<strategy::StatArbStrategy>(
                order_manager_, position_manager_, params);
            strategy->setSymbol(symbol);
            strategy->initialize();
            strategy->start();

            strategies_.push_back(std::move(strategy));
            LOG_INFO("Statistical Arbitrage strategy initialized for ", symbol);
        }
    }

    void onMarketData(const std::string& symbol, const market_data::OrderBook& order_book) {
        // Forward market data to strategies
        for (auto& strategy : strategies_) {
            if (strategy->getSymbol() == symbol && strategy->isEnabled()) {
                strategy->onMarketData(symbol, order_book);
            }
        }

        metrics::PerformanceMonitor::instance().recordMarketDataMessage();
    }

    void printStatus() {
        std::cout << "\n========================================\n";
        std::cout << "HFT System Status\n";
        std::cout << "========================================\n";

        // Print performance metrics
        metrics::PerformanceMonitor::instance().printMetrics();

        // Print positions
        auto positions = position_manager_.getAllPositions();
        if (!positions.empty()) {
            std::cout << "Positions:\n";
            for (const auto& pos : positions) {
                std::cout << "  " << pos.symbol << ": "
                         << pos.quantity << " @ " << pos.average_price
                         << " | PnL: " << pos.realized_pnl << "\n";
            }
        }

        // Print order book status
        std::string symbol = core::Config::instance().getString("symbol", "AAPL");
        const auto* ob = market_data_handler_.getOrderBook(symbol);
        if (ob) {
            std::cout << "\nOrder Book (" << symbol << "):\n";
            std::cout << "  Best Bid: " << ob->getBestBid() << "\n";
            std::cout << "  Best Ask: " << ob->getBestAsk() << "\n";
            std::cout << "  Mid Price: " << ob->getMidPrice() << "\n";
            std::cout << "  Spread: " << ob->getSpread() << "\n";
            std::cout << "  Total Volume: " << ob->getTotalVolume() << "\n";
        }

        std::cout << "========================================\n";
    }

    // Components
    oms::OrderManager order_manager_;
    oms::PositionManager position_manager_;
    risk::RiskManager risk_manager_;
    market_data::MarketDataHandler market_data_handler_;
    gateway::ExchangeGateway exchange_gateway_;

    std::vector<std::unique_ptr<strategy::StrategyBase>> strategies_;
};

int main(int argc, char* argv[]) {
    std::cout << R"(
╔═══════════════════════════════════════════════════════════╗
║       High Frequency Trading System v1.0                  ║
║       Production-Ready Low Latency C++ Implementation     ║
╚═══════════════════════════════════════════════════════════╝
)" << std::endl;

    // Setup signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Get config file path
    std::string config_file = "config/hft_config.txt";
    if (argc > 1) {
        config_file = argv[1];
    }

    // Create and initialize system
    HFTSystem system;

    if (!system.initialize(config_file)) {
        std::cerr << "Failed to initialize HFT system\n";
        return 1;
    }

    // Run system
    system.run();

    return 0;
}

#include "../include/market_data/order_book.hpp"
#include "../include/core/timer.hpp"
#include <iostream>
#include <cassert>

using namespace hft;

void testBasicOrdering() {
    std::cout << "Testing basic order book operations...\n";

    market_data::OrderBook ob;

    // Add buy order
    market_data::Order buy_order{1, 10000, 100, market_data::Side::BUY, core::Timer::timestamp_ns()};
    ob.addOrder(buy_order);

    assert(ob.getBestBid() == 10000);
    std::cout << "✓ Buy order added correctly\n";

    // Add sell order
    market_data::Order sell_order{2, 10010, 100, market_data::Side::SELL, core::Timer::timestamp_ns()};
    ob.addOrder(sell_order);

    assert(ob.getBestAsk() == 10010);
    assert(ob.getSpread() == 10);
    std::cout << "✓ Sell order added correctly\n";
    std::cout << "✓ Spread calculated correctly\n";
}

void testMatching() {
    std::cout << "\nTesting order matching...\n";

    market_data::OrderBook ob;

    int trade_count = 0;
    ob.setTradeCallback([&trade_count](const market_data::Trade& trade) {
        trade_count++;
        std::cout << "  Trade: " << trade.quantity << " @ " << trade.price << "\n";
    });

    // Add passive sell order
    market_data::Order sell_order{1, 10000, 100, market_data::Side::SELL, core::Timer::timestamp_ns()};
    ob.addOrder(sell_order);

    // Add aggressive buy order that should match
    market_data::Order buy_order{2, 10000, 50, market_data::Side::BUY, core::Timer::timestamp_ns()};
    ob.addOrder(buy_order);

    assert(trade_count == 1);
    assert(ob.getTotalVolume() == 50);
    std::cout << "✓ Order matching works correctly\n";
}

void testCancellation() {
    std::cout << "\nTesting order cancellation...\n";

    market_data::OrderBook ob;

    market_data::Order order{1, 10000, 100, market_data::Side::BUY, core::Timer::timestamp_ns()};
    ob.addOrder(order);

    assert(ob.getBestBid() == 10000);

    bool cancelled = ob.cancelOrder(1);
    assert(cancelled);
    assert(ob.getBestBid() == 0);

    std::cout << "✓ Order cancellation works correctly\n";
}

void testPerformance() {
    std::cout << "\nTesting order book performance...\n";

    market_data::OrderBook ob;
    const int num_orders = 10000;

    core::Timer timer;

    // Add orders
    for (int i = 0; i < num_orders; ++i) {
        market_data::Order order{
            static_cast<uint64_t>(i),
            static_cast<uint64_t>(10000 + (i % 100)),
            100,
            (i % 2 == 0) ? market_data::Side::BUY : market_data::Side::SELL,
            core::Timer::timestamp_ns()
        };
        ob.addOrder(order);
    }

    int64_t elapsed_us = timer.elapsed_us();
    double avg_latency = static_cast<double>(elapsed_us) / num_orders;

    std::cout << "  Processed " << num_orders << " orders in " << elapsed_us << " us\n";
    std::cout << "  Average latency: " << avg_latency << " us per order\n";
    std::cout << "✓ Performance test completed\n";
}

int main() {
    std::cout << "=== Order Book Tests ===\n\n";

    try {
        testBasicOrdering();
        testMatching();
        testCancellation();
        testPerformance();

        std::cout << "\n✅ All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << "\n";
        return 1;
    }
}

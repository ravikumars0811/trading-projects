"""
Example 2: High-Frequency Trading Signal Generation

This example demonstrates:
- Real-time order book analysis
- Ultra-low latency signal generation
- Risk controls
- Performance monitoring

Advanced example for HFT applications
"""

import numpy as np
import sys
import os
from datetime import datetime
import time

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from src.hft.signal_generator import (
    HFTSignalGenerator,
    OrderBookFeatures,
    MarketRegimeDetector,
    RiskController
)


def simulate_order_book(base_price=100.0, volatility=0.001):
    """
    Simulate a realistic order book

    In production, use real order book data from:
    - Exchange direct feeds
    - Market data vendors
    - Broker APIs
    """
    # Generate bid levels
    bids = []
    for i in range(10):
        price = base_price - i * 0.01 - np.random.rand() * 0.001
        size = np.random.randint(100, 2000)
        bids.append([price, size])

    # Generate ask levels
    asks = []
    for i in range(10):
        price = base_price + i * 0.01 + np.random.rand() * 0.001
        size = np.random.randint(100, 2000)
        asks.append([price, size])

    return {
        'bids': np.array(bids),
        'asks': np.array(asks)
    }


def simulate_market_tick(tick_num, base_price=100.0):
    """Simulate a single market tick"""
    # Add some realistic price movement
    price_change = np.random.randn() * 0.02
    current_price = base_price + price_change + np.sin(tick_num / 100) * 0.1

    # Generate order book
    order_book = simulate_order_book(current_price)

    # Market data
    market_data = {
        'order_book': order_book,
        'last_price': current_price,
        'volume': np.random.randint(10000, 50000),
        'timestamp': datetime.now()
    }

    return market_data, current_price


def main():
    """Main execution"""
    print("=" * 70)
    print("Example 2: High-Frequency Trading Signal Generation")
    print("=" * 70)

    # Step 1: Analyze Order Book Features
    print("\n[Step 1] Understanding Order Book Features...")

    sample_market_data, _ = simulate_market_tick(0, base_price=100.0)

    print(f"\n  Sample Order Book:")
    print(f"  Top 5 Bids:")
    for i, (price, size) in enumerate(sample_market_data['order_book']['bids'][:5]):
        print(f"    Level {i+1}: ${price:.2f} x {size:,}")

    print(f"\n  Top 5 Asks:")
    for i, (price, size) in enumerate(sample_market_data['order_book']['asks'][:5]):
        print(f"    Level {i+1}: ${price:.2f} x {size:,}")

    # Extract features
    features_calculator = OrderBookFeatures()
    features = features_calculator.calculate_features(
        sample_market_data['order_book']
    )

    print(f"\n  Extracted Features:")
    feature_names = [
        'Spread',
        'Microprice',
        'Order Imbalance',
        'Depth Imbalance',
        'Weighted Mid',
        'Bid Volume (Best)',
        'Ask Volume (Best)'
    ]

    for name, value in zip(feature_names, features):
        print(f"    {name:<20}: {value:.4f}")

    # Step 2: Market Regime Detection
    print(f"\n[Step 2] Market Regime Detection...")

    regime_detector = MarketRegimeDetector(lookback_period=100)

    # Simulate price history
    print(f"  Simulating price movements...")
    regimes_detected = []

    for i in range(150):
        _, price = simulate_market_tick(i, base_price=100.0)
        regime = regime_detector.detect_regime(price)
        if i >= 100:  # After warmup
            regimes_detected.append(regime)

    # Count regimes
    from collections import Counter
    regime_counts = Counter(regimes_detected)

    print(f"\n  Detected Market Regimes:")
    for regime, count in regime_counts.items():
        percentage = count / len(regimes_detected) * 100
        print(f"    {regime:<20}: {percentage:.1f}%")

    # Step 3: Risk Controls
    print(f"\n[Step 3] Risk Controls...")

    risk_controller = RiskController(
        max_position=10000,
        max_notional=1_000_000,
        max_daily_loss=50_000
    )

    print(f"  Risk Limits:")
    print(f"    Max Position Size: {risk_controller.max_position:,}")
    print(f"    Max Notional: ${risk_controller.max_notional:,}")
    print(f"    Max Daily Loss: ${risk_controller.max_daily_loss:,}")

    # Test risk assessment
    risk_score = risk_controller.assess_risk(
        symbol='AAPL',
        signal='BUY',
        confidence=0.75,
        price=100.0,
        size=5000,
        current_position=0
    )

    print(f"\n  Example Risk Assessment:")
    print(f"    Signal: BUY 5000 shares @ $100")
    print(f"    Risk Score: {risk_score:.2f} (0=low, 1=high)")

    if risk_score < 0.5:
        print(f"    Decision: ✓ APPROVED - Risk within limits")
    else:
        print(f"    Decision: ✗ REJECTED - Risk too high")

    # Step 4: Simulated Trading Session
    print(f"\n[Step 4] Simulated HFT Trading Session...")
    print(f"  (Demonstrating signal generation without actual model)")
    print(f"  Note: In production, load a trained model\n")

    # Simulate trading
    num_ticks = 50
    signals_generated = []
    latencies = []

    print(f"  {'Tick':<6} {'Price':<10} {'Signal':<8} {'Imbalance':<12} {'Latency':<12}")
    print(f"  {'-' * 60}")

    base_price = 100.0

    for tick in range(num_ticks):
        # Generate market data
        start_time = time.time()
        market_data, current_price = simulate_market_tick(tick, base_price)

        # Extract features
        features = features_calculator.calculate_features(
            market_data['order_book']
        )

        # Simple rule-based signal (replace with AI model in production)
        order_imbalance = features[2]

        if order_imbalance > 0.1:
            signal = 'BUY'
        elif order_imbalance < -0.1:
            signal = 'SELL'
        else:
            signal = 'HOLD'

        # Calculate latency
        end_time = time.time()
        latency_ms = (end_time - start_time) * 1000

        signals_generated.append(signal)
        latencies.append(latency_ms)

        # Print every 5 ticks
        if tick % 5 == 0:
            print(f"  {tick:<6} ${current_price:<9.2f} {signal:<8} "
                  f"{order_imbalance:<12.4f} {latency_ms:<12.3f}ms")

    # Step 5: Performance Analysis
    print(f"\n[Step 5] Performance Analysis...")

    latencies_array = np.array(latencies)
    signal_counts = Counter(signals_generated)

    print(f"\n  Latency Statistics:")
    print(f"    Mean: {latencies_array.mean():.3f}ms")
    print(f"    Median: {np.median(latencies_array):.3f}ms")
    print(f"    P95: {np.percentile(latencies_array, 95):.3f}ms")
    print(f"    P99: {np.percentile(latencies_array, 99):.3f}ms")
    print(f"    Max: {latencies_array.max():.3f}ms")
    print(f"    Min: {latencies_array.min():.3f}ms")

    print(f"\n  Signal Distribution:")
    for signal, count in signal_counts.items():
        percentage = count / len(signals_generated) * 100
        print(f"    {signal:<8}: {count:3d} ({percentage:.1f}%)")

    # Step 6: Optimization Tips
    print(f"\n{'=' * 70}")
    print(f"OPTIMIZATION TIPS FOR PRODUCTION HFT")
    print(f"{'=' * 70}")

    print(f"\n  1. Model Optimization:")
    print(f"     • Use INT8 quantization (4x speedup)")
    print(f"     • Deploy on GPU (10-100x speedup)")
    print(f"     • Batch predictions when possible")
    print(f"     • Use TorchScript compilation")

    print(f"\n  2. Feature Optimization:")
    print(f"     • Pre-compute static features")
    print(f"     • Cache frequently used calculations")
    print(f"     • Use vectorized operations")
    print(f"     • Minimize data copies")

    print(f"\n  3. Infrastructure:")
    print(f"     • Co-locate servers at exchange")
    print(f"     • Use kernel bypass networking")
    print(f"     • Pin processes to CPU cores")
    print(f"     • Use low-latency message queues")

    print(f"\n  4. Risk Management:")
    print(f"     • Implement circuit breakers")
    print(f"     • Set position limits")
    print(f"     • Monitor P&L in real-time")
    print(f"     • Have kill switches")

    # Step 7: Real-World Considerations
    print(f"\n{'=' * 70}")
    print(f"REAL-WORLD DEPLOYMENT CHECKLIST")
    print(f"{'=' * 70}")

    checklist = [
        ("Model Training", "Train on years of historical data"),
        ("Backtesting", "Extensive backtesting with realistic costs"),
        ("Paper Trading", "Live paper trading for weeks/months"),
        ("Risk Limits", "Implement comprehensive risk controls"),
        ("Monitoring", "24/7 monitoring and alerting"),
        ("Compliance", "Ensure regulatory compliance"),
        ("Disaster Recovery", "Have failover and recovery procedures"),
        ("Performance", "Achieve <100μs latency target"),
    ]

    for item, description in checklist:
        print(f"  ☐ {item:<20} {description}")

    print(f"\n{'=' * 70}")
    print(f"Example completed successfully!")
    print(f"{'=' * 70}")

    print(f"\nKey Takeaways:")
    print(f"  • Order book features are critical for HFT")
    print(f"  • Latency optimization is paramount")
    print(f"  • Risk controls must be comprehensive")
    print(f"  • Market regime detection improves performance")
    print(f"  • Production deployment requires extensive testing")

    print(f"\nNext steps:")
    print(f"  1. Train actual AI models on historical data")
    print(f"  2. Implement model quantization for speed")
    print(f"  3. Set up real-time market data feeds")
    print(f"  4. Deploy on co-located infrastructure")
    print(f"  5. Start with paper trading before going live")


if __name__ == "__main__":
    main()

"""
Example 1: Basic Market Prediction

This example demonstrates:
- Loading market data
- Creating features
- Training a prediction model
- Making predictions
- Evaluating performance

Perfect for beginners!
"""

import numpy as np
import sys
import os

# Add parent directory to path
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from src.models.market_predictor import MarketPredictor
from datetime import datetime, timedelta


def generate_sample_market_data(num_days=500):
    """
    Generate synthetic market data for demonstration

    In production, replace with real market data from:
    - yfinance
    - Alpha Vantage
    - Bloomberg API
    - Your broker's API
    """
    print("Generating sample market data...")

    # Generate price series with trend and noise
    np.random.seed(42)

    # Trend component
    trend = np.linspace(100, 150, num_days)

    # Seasonal component
    seasonal = 10 * np.sin(np.linspace(0, 8 * np.pi, num_days))

    # Random walk component
    random_walk = np.cumsum(np.random.randn(num_days) * 0.5)

    # Combine components
    prices = trend + seasonal + random_walk

    # Volume
    volumes = np.random.randint(1_000_000, 10_000_000, num_days)

    return {
        'prices': prices,
        'volumes': volumes,
        'dates': [datetime.now() - timedelta(days=num_days-i) for i in range(num_days)]
    }


def calculate_technical_features(prices, volumes, window=100):
    """
    Calculate technical indicators as features

    Features:
    - Price returns
    - Moving averages
    - Volatility
    - Volume indicators
    - Momentum
    """
    print("Calculating technical features...")

    features_list = []

    for i in range(window, len(prices)):
        # Extract window
        price_window = prices[i-window:i]
        volume_window = volumes[i-window:i]

        # Returns
        returns = np.diff(price_window) / price_window[:-1]
        current_return = returns[-1] if len(returns) > 0 else 0

        # Moving averages
        sma_10 = price_window[-10:].mean() if len(price_window) >= 10 else 0
        sma_50 = price_window[-50:].mean() if len(price_window) >= 50 else 0
        sma_ratio = (sma_10 / sma_50 - 1) if sma_50 > 0 else 0

        # Volatility
        volatility = returns.std() if len(returns) > 1 else 0

        # Volume indicators
        volume_ma = volume_window.mean()
        volume_ratio = volume_window[-1] / volume_ma if volume_ma > 0 else 1

        # Momentum
        momentum_5 = (price_window[-1] / price_window[-5] - 1) if len(price_window) >= 5 else 0
        momentum_20 = (price_window[-1] / price_window[-20] - 1) if len(price_window) >= 20 else 0

        # Create feature vector (pad to 128 dimensions)
        features = np.array([
            current_return,
            sma_ratio,
            volatility,
            volume_ratio,
            momentum_5,
            momentum_20,
            # Pad with zeros to reach 128 dimensions
            *([0] * 122)
        ])

        features_list.append(features)

    return np.array(features_list)


def create_labels(prices, window=100, horizon=1):
    """
    Create labels for supervised learning

    Label:
    - 0: DOWN (price decreases)
    - 1: NEUTRAL (price unchanged)
    - 2: UP (price increases)

    Also predict magnitude
    """
    print("Creating labels...")

    labels_list = []

    for i in range(window, len(prices) - horizon):
        current_price = prices[i]
        future_price = prices[i + horizon]

        # Calculate return
        future_return = (future_price - current_price) / current_price

        # Classify direction
        if future_return < -0.002:  # Down more than 0.2%
            direction = 0  # DOWN
        elif future_return > 0.002:  # Up more than 0.2%
            direction = 2  # UP
        else:
            direction = 1  # NEUTRAL

        # Magnitude
        magnitude = abs(future_return)

        labels_list.append([direction, magnitude])

    return np.array(labels_list)


def main():
    """Main execution"""
    print("=" * 70)
    print("Example 1: Basic Market Prediction with Generative AI")
    print("=" * 70)

    # Step 1: Generate sample data
    print("\n[Step 1] Generating market data...")
    market_data = generate_sample_market_data(num_days=500)
    print(f"  ✓ Generated {len(market_data['prices'])} days of data")
    print(f"  ✓ Price range: ${market_data['prices'].min():.2f} - ${market_data['prices'].max():.2f}")

    # Step 2: Calculate features
    print("\n[Step 2] Calculating technical features...")
    features = calculate_technical_features(
        market_data['prices'],
        market_data['volumes'],
        window=100
    )
    print(f"  ✓ Created {len(features)} feature vectors")
    print(f"  ✓ Feature dimension: {features[0].shape[0]}")

    # Step 3: Create labels
    print("\n[Step 3] Creating labels...")
    labels = create_labels(market_data['prices'], window=100, horizon=1)
    print(f"  ✓ Created {len(labels)} labels")

    # Make sure features and labels align
    min_len = min(len(features), len(labels))
    features = features[:min_len]
    labels = labels[:min_len]

    # Step 4: Split data
    print("\n[Step 4] Splitting data into train/validation sets...")
    split_idx = int(len(features) * 0.8)

    train_features = features[:split_idx]
    train_labels = labels[:split_idx]
    val_features = features[split_idx:]
    val_labels = labels[split_idx:]

    print(f"  ✓ Training set: {len(train_features)} samples")
    print(f"  ✓ Validation set: {len(val_features)} samples")

    # Step 5: Create and train model
    print("\n[Step 5] Creating AI model...")
    predictor = MarketPredictor(
        model_type='transformer',
        device='cpu'  # Use 'cuda' if GPU available
    )
    print(f"  ✓ Model created: Transformer-based")
    print(f"  ✓ Device: {predictor.device}")

    print("\n[Step 6] Training model...")
    print("  (This may take a few minutes...)")

    # Reshape features for sequence model
    train_features_seq = train_features.reshape(-1, 1, 128)
    val_features_seq = val_features.reshape(-1, 1, 128)

    predictor.train(
        train_data=train_features_seq,
        train_labels=train_labels,
        val_data=val_features_seq,
        val_labels=val_labels,
        epochs=5,  # Use more epochs (50+) for better results
        batch_size=32,
        learning_rate=0.001
    )

    # Step 7: Make predictions
    print("\n[Step 7] Making predictions on validation set...")

    predictions = []
    actuals = []

    for i in range(len(val_features_seq)):
        # Predict
        prediction = predictor.predict(
            val_features_seq[i],
            horizon='1d'
        )

        # Store
        predictions.append(prediction.direction)
        actual_direction = ['DOWN', 'NEUTRAL', 'UP'][int(val_labels[i, 0])]
        actuals.append(actual_direction)

    # Step 8: Evaluate performance
    print("\n[Step 8] Evaluating model performance...")

    correct = sum([1 for p, a in zip(predictions, actuals) if p == a])
    accuracy = correct / len(predictions)

    print(f"\n{'=' * 70}")
    print(f"RESULTS")
    print(f"{'=' * 70}")
    print(f"  Accuracy: {accuracy:.2%}")
    print(f"  Correct predictions: {correct}/{len(predictions)}")

    # Show some example predictions
    print(f"\n  Sample Predictions:")
    print(f"  {'Actual':<10} {'Predicted':<10} {'Confidence':<12}")
    print(f"  {'-' * 35}")

    for i in range(min(10, len(predictions))):
        # Get prediction details
        pred = predictor.predict(val_features_seq[i], horizon='1d')
        print(f"  {actuals[i]:<10} {predictions[i]:<10} {pred.confidence:<12.1%}")

    # Step 9: Live prediction example
    print(f"\n{'=' * 70}")
    print(f"LIVE PREDICTION EXAMPLE")
    print(f"{'=' * 70}")

    # Use the last available data point
    latest_features = val_features_seq[-1]
    prediction = predictor.predict(latest_features, horizon='1d')

    print(f"\nPrediction for tomorrow:")
    print(f"  Direction: {prediction.direction}")
    print(f"  Confidence: {prediction.confidence:.1%}")
    print(f"  Expected Move: {prediction.magnitude:.2%}")

    print(f"\n  Probability Distribution:")
    for direction, prob in prediction.probabilities.items():
        bar = '█' * int(prob * 50)
        print(f"    {direction:<8} {bar} {prob:.1%}")

    print(f"\n  Feature Importance:")
    for feature, importance in sorted(
        prediction.features_importance.items(),
        key=lambda x: x[1],
        reverse=True
    ):
        bar = '█' * int(importance * 50)
        print(f"    {feature:<12} {bar} {importance:.1%}")

    # Step 10: Trading signal
    print(f"\n{'=' * 70}")
    print(f"TRADING SIGNAL")
    print(f"{'=' * 70}")

    if prediction.confidence > 0.65 and prediction.direction == 'UP':
        signal = "🟢 BUY"
        rationale = f"High confidence ({prediction.confidence:.1%}) upward prediction"
    elif prediction.confidence > 0.65 and prediction.direction == 'DOWN':
        signal = "🔴 SELL"
        rationale = f"High confidence ({prediction.confidence:.1%}) downward prediction"
    else:
        signal = "🟡 HOLD"
        rationale = "Insufficient confidence or neutral prediction"

    print(f"\n  Signal: {signal}")
    print(f"  Rationale: {rationale}")
    print(f"  Recommended Position Size: {int(prediction.confidence * 100)}%")

    print(f"\n{'=' * 70}")
    print(f"Example completed successfully!")
    print(f"{'=' * 70}")

    print(f"\nNext steps:")
    print(f"  1. Try example 02_hft_signal_generation.py for HFT")
    print(f"  2. Try example 03_portfolio_optimization.py for investment banking")
    print(f"  3. Replace sample data with real market data")
    print(f"  4. Tune hyperparameters for better performance")
    print(f"  5. Add more features (sentiment, fundamentals, etc.)")


if __name__ == "__main__":
    main()

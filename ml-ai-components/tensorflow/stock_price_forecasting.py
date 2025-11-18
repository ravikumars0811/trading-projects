"""
TensorFlow: Stock Price Forecasting with LSTM
==============================================
Real-world example: Time-series forecasting for algorithmic trading

Industry Use Case: Hedge funds and trading firms use deep learning models
for price prediction, risk management, and automated trading strategies.
"""

import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
from sklearn.preprocessing import MinMaxScaler
from sklearn.metrics import mean_absolute_error, mean_squared_error
import matplotlib
matplotlib.use('Agg')  # Non-interactive backend
import matplotlib.pyplot as plt
from datetime import datetime, timedelta
import warnings
warnings.filterwarnings('ignore')

# Set random seeds for reproducibility
np.random.seed(42)
tf.random.set_seed(42)


class StockPriceForecaster:
    """
    Production-grade LSTM model for stock price forecasting
    Implements best practices for time-series deep learning
    """

    def __init__(self, sequence_length=60, forecast_horizon=5):
        self.sequence_length = sequence_length  # Days to look back
        self.forecast_horizon = forecast_horizon  # Days to forecast
        self.model = None
        self.scaler = MinMaxScaler(feature_range=(0, 1))
        self.history = None
        self.metadata = {
            'created_at': datetime.now().isoformat(),
            'model_type': 'LSTM',
            'version': '1.0.0'
        }

    def generate_stock_data(self, symbol='AAPL', days=1000):
        """
        Generate synthetic stock price data with realistic patterns
        In production, use yfinance, Alpha Vantage, or Bloomberg API
        """
        np.random.seed(42)

        # Generate dates
        end_date = datetime.now()
        start_date = end_date - timedelta(days=days)
        dates = pd.date_range(start=start_date, end=end_date, freq='D')

        # Generate price with trend + seasonality + noise
        t = np.arange(len(dates))

        # Components
        trend = 100 + 0.05 * t  # Upward trend
        seasonality = 10 * np.sin(2 * np.pi * t / 365)  # Annual cycle
        noise = np.random.normal(0, 5, len(t))  # Random volatility
        momentum = np.cumsum(np.random.randn(len(t))) * 0.5  # Random walk

        # Combine components
        close_price = trend + seasonality + noise + momentum

        # Generate OHLCV data
        data = pd.DataFrame({
            'date': dates,
            'open': close_price * (1 + np.random.uniform(-0.02, 0.02, len(dates))),
            'high': close_price * (1 + np.random.uniform(0, 0.05, len(dates))),
            'low': close_price * (1 + np.random.uniform(-0.05, 0, len(dates))),
            'close': close_price,
            'volume': np.random.randint(1000000, 50000000, len(dates))
        })

        # Add technical indicators
        data = self._add_technical_indicators(data)

        return data

    def _add_technical_indicators(self, df):
        """Add technical analysis features"""
        # Moving averages
        df['ma_7'] = df['close'].rolling(window=7).mean()
        df['ma_21'] = df['close'].rolling(window=21).mean()
        df['ma_50'] = df['close'].rolling(window=50).mean()

        # Exponential moving average
        df['ema_12'] = df['close'].ewm(span=12).mean()
        df['ema_26'] = df['close'].ewm(span=26).mean()

        # MACD
        df['macd'] = df['ema_12'] - df['ema_26']
        df['macd_signal'] = df['macd'].ewm(span=9).mean()

        # RSI (Relative Strength Index)
        delta = df['close'].diff()
        gain = (delta.where(delta > 0, 0)).rolling(window=14).mean()
        loss = (-delta.where(delta < 0, 0)).rolling(window=14).mean()
        rs = gain / loss
        df['rsi'] = 100 - (100 / (1 + rs))

        # Bollinger Bands
        df['bb_middle'] = df['close'].rolling(window=20).mean()
        bb_std = df['close'].rolling(window=20).std()
        df['bb_upper'] = df['bb_middle'] + (bb_std * 2)
        df['bb_lower'] = df['bb_middle'] - (bb_std * 2)

        # Price momentum
        df['momentum'] = df['close'].pct_change(periods=10)

        # Volatility
        df['volatility'] = df['close'].rolling(window=20).std()

        # Drop NaN values
        df = df.dropna()

        return df

    def prepare_sequences(self, data, target_col='close'):
        """
        Create sequences for LSTM training
        X: [samples, sequence_length, features]
        y: [samples, forecast_horizon]
        """
        features = ['close', 'volume', 'ma_7', 'ma_21', 'rsi', 'macd', 'volatility']
        feature_data = data[features].values

        # Normalize data
        scaled_data = self.scaler.fit_transform(feature_data)

        X, y = [], []

        for i in range(self.sequence_length, len(scaled_data) - self.forecast_horizon):
            # Input: past sequence_length days
            X.append(scaled_data[i - self.sequence_length:i])

            # Output: next forecast_horizon days (close price only)
            y.append(scaled_data[i:i + self.forecast_horizon, 0])

        return np.array(X), np.array(y)

    def build_model(self, input_shape):
        """
        Build LSTM architecture optimized for time-series forecasting
        """
        model = keras.Sequential([
            # First LSTM layer with return sequences
            layers.LSTM(
                128,
                return_sequences=True,
                input_shape=input_shape,
                dropout=0.2,
                recurrent_dropout=0.2
            ),

            # Second LSTM layer
            layers.LSTM(
                64,
                return_sequences=False,
                dropout=0.2,
                recurrent_dropout=0.2
            ),

            # Dense layers with dropout for regularization
            layers.Dense(64, activation='relu'),
            layers.Dropout(0.3),

            layers.Dense(32, activation='relu'),
            layers.Dropout(0.2),

            # Output layer (forecast_horizon predictions)
            layers.Dense(self.forecast_horizon)
        ])

        # Compile with appropriate loss and optimizer
        model.compile(
            optimizer=keras.optimizers.Adam(learning_rate=0.001),
            loss='huber',  # Robust to outliers
            metrics=['mae', 'mse']
        )

        return model

    def train(self, X_train, y_train, X_val, y_val, epochs=50, batch_size=32):
        """Train the LSTM model"""
        print("\nBuilding LSTM model...")
        self.model = self.build_model(input_shape=(X_train.shape[1], X_train.shape[2]))

        print(f"\nModel Architecture:")
        self.model.summary()

        # Callbacks for training
        callbacks = [
            keras.callbacks.EarlyStopping(
                monitor='val_loss',
                patience=10,
                restore_best_weights=True,
                verbose=1
            ),
            keras.callbacks.ReduceLROnPlateau(
                monitor='val_loss',
                factor=0.5,
                patience=5,
                min_lr=1e-7,
                verbose=1
            )
        ]

        print(f"\nTraining model for up to {epochs} epochs...")
        self.history = self.model.fit(
            X_train, y_train,
            validation_data=(X_val, y_val),
            epochs=epochs,
            batch_size=batch_size,
            callbacks=callbacks,
            verbose=1
        )

        print("\n✓ Model training completed")

    def evaluate(self, X_test, y_test):
        """Evaluate model performance"""
        print("\n" + "=" * 70)
        print("MODEL EVALUATION REPORT")
        print("=" * 70)

        # Make predictions
        y_pred = self.model.predict(X_test, verbose=0)

        # Calculate metrics for each forecast horizon
        print("\nForecast Performance by Day:")
        print("-" * 70)

        for day in range(self.forecast_horizon):
            mae = mean_absolute_error(y_test[:, day], y_pred[:, day])
            mse = mean_squared_error(y_test[:, day], y_pred[:, day])
            rmse = np.sqrt(mse)

            print(f"Day {day + 1}:")
            print(f"  MAE:  {mae:.6f}")
            print(f"  RMSE: {rmse:.6f}")

        # Overall metrics
        overall_mae = mean_absolute_error(y_test.flatten(), y_pred.flatten())
        overall_rmse = np.sqrt(mean_squared_error(y_test.flatten(), y_pred.flatten()))

        print(f"\nOverall Performance:")
        print(f"  MAE:  {overall_mae:.6f}")
        print(f"  RMSE: {overall_rmse:.6f}")

        return {
            'mae': overall_mae,
            'rmse': overall_rmse,
            'predictions': y_pred
        }

    def forecast(self, last_sequence):
        """
        Make a forecast for the next forecast_horizon days
        last_sequence: [sequence_length, features] - most recent data
        """
        # Ensure correct shape
        if last_sequence.ndim == 2:
            last_sequence = np.expand_dims(last_sequence, axis=0)

        # Predict
        forecast = self.model.predict(last_sequence, verbose=0)[0]

        return forecast

    def plot_training_history(self, save_path='training_history.png'):
        """Visualize training progress"""
        if self.history is None:
            print("No training history available")
            return

        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 5))

        # Loss
        ax1.plot(self.history.history['loss'], label='Training Loss')
        ax1.plot(self.history.history['val_loss'], label='Validation Loss')
        ax1.set_title('Model Loss')
        ax1.set_xlabel('Epoch')
        ax1.set_ylabel('Loss')
        ax1.legend()
        ax1.grid(True)

        # MAE
        ax2.plot(self.history.history['mae'], label='Training MAE')
        ax2.plot(self.history.history['val_mae'], label='Validation MAE')
        ax2.set_title('Model MAE')
        ax2.set_xlabel('Epoch')
        ax2.set_ylabel('MAE')
        ax2.legend()
        ax2.grid(True)

        plt.tight_layout()
        plt.savefig(save_path)
        print(f"\n✓ Training history saved to {save_path}")

    def save_model(self, filepath='stock_forecaster.keras'):
        """Save model for production deployment"""
        self.model.save(filepath)
        print(f"\n✓ Model saved to {filepath}")


def main():
    """Demo: Complete stock price forecasting pipeline"""
    print("=" * 70)
    print("STOCK PRICE FORECASTING WITH LSTM")
    print("=" * 70)

    # Initialize forecaster
    forecaster = StockPriceForecaster(sequence_length=60, forecast_horizon=5)

    # Generate data
    print("\n1. GENERATING STOCK DATA...")
    df = forecaster.generate_stock_data(symbol='AAPL', days=1000)
    print(f"   Generated {len(df)} days of data")
    print(f"   Features: {df.columns.tolist()}")

    # Prepare sequences
    print("\n2. PREPARING SEQUENCES...")
    X, y = forecaster.prepare_sequences(df)
    print(f"   Input shape: {X.shape}")
    print(f"   Output shape: {y.shape}")

    # Split data (80% train, 10% val, 10% test)
    train_size = int(0.8 * len(X))
    val_size = int(0.1 * len(X))

    X_train = X[:train_size]
    y_train = y[:train_size]

    X_val = X[train_size:train_size + val_size]
    y_val = y[train_size:train_size + val_size]

    X_test = X[train_size + val_size:]
    y_test = y[train_size + val_size:]

    print(f"   Training samples: {len(X_train)}")
    print(f"   Validation samples: {len(X_val)}")
    print(f"   Test samples: {len(X_test)}")

    # Train model
    print("\n3. TRAINING LSTM MODEL...")
    forecaster.train(X_train, y_train, X_val, y_val, epochs=30, batch_size=32)

    # Evaluate model
    print("\n4. EVALUATING MODEL...")
    metrics = forecaster.evaluate(X_test, y_test)

    # Plot training history
    print("\n5. PLOTTING TRAINING HISTORY...")
    forecaster.plot_training_history()

    # Make a forecast
    print("\n6. MAKING FORECAST FOR NEXT 5 DAYS...")
    print("-" * 70)

    last_sequence = X_test[-1:, :, :]
    forecast = forecaster.forecast(last_sequence)

    print(f"Forecasted normalized prices:")
    for i, price in enumerate(forecast, 1):
        print(f"  Day {i}: {price:.6f}")

    # Save model
    print("\n7. SAVING MODEL...")
    forecaster.save_model()

    print("\n" + "=" * 70)
    print("STOCK FORECASTING PIPELINE COMPLETE")
    print("=" * 70)

    # Production recommendations
    print("\nPRODUCTION RECOMMENDATIONS:")
    print("- Use real market data APIs (yfinance, Alpha Vantage)")
    print("- Implement walk-forward validation")
    print("- Add attention mechanisms for better performance")
    print("- Monitor prediction drift in production")
    print("- Implement ensemble models for robustness")
    print("- Add confidence intervals to forecasts")


if __name__ == "__main__":
    main()

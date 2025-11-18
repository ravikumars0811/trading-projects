"""
Machine Learning Model Training Pipeline for Algorithmic Trading

This module provides a complete pipeline for training ML models on market data.
Supports multiple model types: LSTM, Transformer, GRU, and traditional ML models.
"""

import numpy as np
import pandas as pd
import torch
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import TimeSeriesSplit
import joblib
from pathlib import Path
from typing import Dict, List, Tuple, Optional
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class MarketDataset(Dataset):
    """PyTorch Dataset for market data"""

    def __init__(self, features: np.ndarray, labels: np.ndarray, sequence_length: int = 50):
        self.features = torch.FloatTensor(features)
        self.labels = torch.FloatTensor(labels)
        self.sequence_length = sequence_length

    def __len__(self):
        return len(self.features) - self.sequence_length

    def __getitem__(self, idx):
        return (
            self.features[idx:idx + self.sequence_length],
            self.labels[idx + self.sequence_length]
        )


class LSTMPricePredictor(nn.Module):
    """LSTM model for price prediction"""

    def __init__(self, input_size: int, hidden_size: int = 128, num_layers: int = 2,
                 dropout: float = 0.2):
        super().__init__()
        self.hidden_size = hidden_size
        self.num_layers = num_layers

        self.lstm = nn.LSTM(
            input_size, hidden_size, num_layers,
            batch_first=True, dropout=dropout
        )

        self.fc_layers = nn.Sequential(
            nn.Linear(hidden_size, 64),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(64, 32),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(32, 1),
            nn.Sigmoid()  # Output probability of price increase
        )

    def forward(self, x):
        # LSTM forward pass
        lstm_out, _ = self.lstm(x)

        # Take the last output
        last_output = lstm_out[:, -1, :]

        # Fully connected layers
        output = self.fc_layers(last_output)

        return output


class TransformerPredictor(nn.Module):
    """Transformer model for price prediction"""

    def __init__(self, input_size: int, d_model: int = 128, nhead: int = 8,
                 num_layers: int = 3, dropout: float = 0.1):
        super().__init__()

        self.input_projection = nn.Linear(input_size, d_model)

        encoder_layer = nn.TransformerEncoderLayer(
            d_model=d_model, nhead=nhead, dropout=dropout,
            batch_first=True
        )
        self.transformer = nn.TransformerEncoder(encoder_layer, num_layers=num_layers)

        self.output_layer = nn.Sequential(
            nn.Linear(d_model, 64),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(64, 1),
            nn.Sigmoid()
        )

    def forward(self, x):
        # Project input to d_model dimensions
        x = self.input_projection(x)

        # Transformer forward pass
        transformer_out = self.transformer(x)

        # Take mean of sequence outputs
        pooled = transformer_out.mean(dim=1)

        # Output layer
        output = self.output_layer(pooled)

        return output


class FeatureEngineer:
    """Feature engineering for market data"""

    @staticmethod
    def calculate_technical_indicators(df: pd.DataFrame) -> pd.DataFrame:
        """Calculate technical indicators"""

        # Price-based features
        df['returns'] = df['close'].pct_change()
        df['log_returns'] = np.log(df['close'] / df['close'].shift(1))

        # Moving averages
        for period in [5, 10, 20, 50, 100]:
            df[f'sma_{period}'] = df['close'].rolling(period).mean()
            df[f'ema_{period}'] = df['close'].ewm(span=period).mean()

        # RSI
        delta = df['close'].diff()
        gain = (delta.where(delta > 0, 0)).rolling(window=14).mean()
        loss = (-delta.where(delta < 0, 0)).rolling(window=14).mean()
        rs = gain / loss
        df['rsi'] = 100 - (100 / (1 + rs))

        # MACD
        exp1 = df['close'].ewm(span=12, adjust=False).mean()
        exp2 = df['close'].ewm(span=26, adjust=False).mean()
        df['macd'] = exp1 - exp2
        df['macd_signal'] = df['macd'].ewm(span=9, adjust=False).mean()

        # Bollinger Bands
        df['bb_middle'] = df['close'].rolling(20).mean()
        bb_std = df['close'].rolling(20).std()
        df['bb_upper'] = df['bb_middle'] + 2 * bb_std
        df['bb_lower'] = df['bb_middle'] - 2 * bb_std
        df['bb_width'] = (df['bb_upper'] - df['bb_lower']) / df['bb_middle']

        # ATR
        high_low = df['high'] - df['low']
        high_close = np.abs(df['high'] - df['close'].shift())
        low_close = np.abs(df['low'] - df['close'].shift())
        ranges = pd.concat([high_low, high_close, low_close], axis=1)
        df['atr'] = ranges.max(axis=1).rolling(14).mean()

        # Volume features
        df['volume_sma_20'] = df['volume'].rolling(20).mean()
        df['volume_ratio'] = df['volume'] / df['volume_sma_20']

        # Price position in range
        df['price_position'] = (df['close'] - df['low']) / (df['high'] - df['low'])

        # Momentum
        for period in [5, 10, 20]:
            df[f'momentum_{period}'] = df['close'] / df['close'].shift(period) - 1

        return df

    @staticmethod
    def create_target(df: pd.DataFrame, horizon: int = 1, threshold: float = 0.0) -> pd.Series:
        """Create target variable (1 if price increases, 0 otherwise)"""
        future_return = df['close'].shift(-horizon) / df['close'] - 1
        return (future_return > threshold).astype(int)


class ModelTrainer:
    """Train and evaluate ML models"""

    def __init__(self, model_type: str = 'lstm', device: str = 'cuda' if torch.cuda.is_available() else 'cpu'):
        self.model_type = model_type
        self.device = device
        self.model = None
        self.scaler = StandardScaler()
        logger.info(f"Using device: {device}")

    def prepare_data(self, df: pd.DataFrame, target_col: str = 'target',
                    sequence_length: int = 50) -> Tuple[DataLoader, DataLoader]:
        """Prepare data for training"""

        # Select feature columns (exclude non-numeric and target)
        feature_cols = [col for col in df.columns if col not in
                       ['target', 'date', 'timestamp', 'symbol'] and
                       df[col].dtype in [np.float64, np.int64]]

        # Remove NaN values
        df = df.dropna()

        # Split data (80-20 split for time series)
        split_idx = int(len(df) * 0.8)
        train_df = df.iloc[:split_idx]
        test_df = df.iloc[split_idx:]

        # Scale features
        train_features = self.scaler.fit_transform(train_df[feature_cols])
        test_features = self.scaler.transform(test_df[feature_cols])

        train_labels = train_df[target_col].values
        test_labels = test_df[target_col].values

        # Create datasets
        train_dataset = MarketDataset(train_features, train_labels, sequence_length)
        test_dataset = MarketDataset(test_features, test_labels, sequence_length)

        # Create dataloaders
        train_loader = DataLoader(train_dataset, batch_size=64, shuffle=False)
        test_loader = DataLoader(test_dataset, batch_size=64, shuffle=False)

        return train_loader, test_loader

    def create_model(self, input_size: int):
        """Create model based on type"""

        if self.model_type == 'lstm':
            self.model = LSTMPricePredictor(input_size)
        elif self.model_type == 'transformer':
            self.model = TransformerPredictor(input_size)
        else:
            raise ValueError(f"Unknown model type: {self.model_type}")

        self.model = self.model.to(self.device)
        logger.info(f"Created {self.model_type} model with {sum(p.numel() for p in self.model.parameters())} parameters")

    def train(self, train_loader: DataLoader, test_loader: DataLoader,
             epochs: int = 100, learning_rate: float = 0.001) -> Dict[str, List[float]]:
        """Train the model"""

        criterion = nn.BCELoss()
        optimizer = torch.optim.Adam(self.model.parameters(), lr=learning_rate)
        scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(
            optimizer, mode='min', factor=0.5, patience=5
        )

        history = {'train_loss': [], 'test_loss': [], 'test_accuracy': []}
        best_loss = float('inf')
        patience_counter = 0
        max_patience = 10

        for epoch in range(epochs):
            # Training
            self.model.train()
            train_loss = 0.0

            for features, labels in train_loader:
                features = features.to(self.device)
                labels = labels.to(self.device).unsqueeze(1)

                optimizer.zero_grad()
                outputs = self.model(features)
                loss = criterion(outputs, labels)
                loss.backward()
                optimizer.step()

                train_loss += loss.item()

            train_loss /= len(train_loader)

            # Evaluation
            test_loss, test_accuracy = self.evaluate(test_loader, criterion)

            history['train_loss'].append(train_loss)
            history['test_loss'].append(test_loss)
            history['test_accuracy'].append(test_accuracy)

            scheduler.step(test_loss)

            # Early stopping
            if test_loss < best_loss:
                best_loss = test_loss
                patience_counter = 0
            else:
                patience_counter += 1

            if patience_counter >= max_patience:
                logger.info(f"Early stopping at epoch {epoch}")
                break

            if (epoch + 1) % 10 == 0:
                logger.info(f"Epoch {epoch+1}/{epochs} - "
                          f"Train Loss: {train_loss:.4f}, "
                          f"Test Loss: {test_loss:.4f}, "
                          f"Test Accuracy: {test_accuracy:.4f}")

        return history

    def evaluate(self, test_loader: DataLoader, criterion) -> Tuple[float, float]:
        """Evaluate the model"""

        self.model.eval()
        test_loss = 0.0
        correct = 0
        total = 0

        with torch.no_grad():
            for features, labels in test_loader:
                features = features.to(self.device)
                labels = labels.to(self.device).unsqueeze(1)

                outputs = self.model(features)
                loss = criterion(outputs, labels)
                test_loss += loss.item()

                predictions = (outputs > 0.5).float()
                correct += (predictions == labels).sum().item()
                total += labels.size(0)

        test_loss /= len(test_loader)
        accuracy = correct / total

        return test_loss, accuracy

    def save_model(self, path: str):
        """Save model and scaler"""
        Path(path).parent.mkdir(parents=True, exist_ok=True)

        torch.save({
            'model_state_dict': self.model.state_dict(),
            'model_type': self.model_type
        }, path)

        scaler_path = path.replace('.pth', '_scaler.pkl')
        joblib.dump(self.scaler, scaler_path)

        logger.info(f"Model saved to {path}")

    def load_model(self, path: str, input_size: int):
        """Load model and scaler"""
        checkpoint = torch.load(path, map_location=self.device)
        self.model_type = checkpoint['model_type']

        self.create_model(input_size)
        self.model.load_state_dict(checkpoint['model_state_dict'])

        scaler_path = path.replace('.pth', '_scaler.pkl')
        self.scaler = joblib.load(scaler_path)

        logger.info(f"Model loaded from {path}")


def main():
    """Example training pipeline"""

    # Load data
    logger.info("Loading market data...")
    df = pd.read_csv('market_data.csv')
    df['date'] = pd.to_datetime(df['date'])

    # Feature engineering
    logger.info("Engineering features...")
    feature_engineer = FeatureEngineer()
    df = feature_engineer.calculate_technical_indicators(df)
    df['target'] = feature_engineer.create_target(df, horizon=1)

    # Train model
    logger.info("Training model...")
    trainer = ModelTrainer(model_type='lstm')
    train_loader, test_loader = trainer.prepare_data(df)

    # Get input size from first batch
    features, _ = next(iter(train_loader))
    input_size = features.shape[2]

    trainer.create_model(input_size)
    history = trainer.train(train_loader, test_loader, epochs=100)

    # Save model
    trainer.save_model('../models/lstm_predictor.pth')
    logger.info("Training complete!")


if __name__ == '__main__':
    main()

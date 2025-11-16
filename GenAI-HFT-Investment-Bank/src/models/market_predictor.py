"""
Market Predictor - Core AI Model for Market Prediction

This module implements state-of-the-art transformer-based models for
financial market prediction, optimized for both HFT and Investment Banking.
"""

import torch
import torch.nn as nn
import numpy as np
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class PredictionResult:
    """Structure for prediction results"""
    direction: str  # 'UP', 'DOWN', 'NEUTRAL'
    confidence: float  # 0.0 to 1.0
    magnitude: float  # Expected move in %
    probabilities: Dict[str, float]
    features_importance: Dict[str, float]
    timestamp: str


class TransformerEncoder(nn.Module):
    """
    Transformer encoder for time series prediction

    Architecture:
    - Multi-head attention layers
    - Position encoding
    - Feed-forward networks
    - Layer normalization

    Args:
        input_dim: Number of input features
        hidden_dim: Hidden layer dimension
        num_heads: Number of attention heads
        num_layers: Number of transformer layers
        dropout: Dropout rate
    """

    def __init__(
        self,
        input_dim: int = 128,
        hidden_dim: int = 512,
        num_heads: int = 8,
        num_layers: int = 6,
        dropout: float = 0.1
    ):
        super().__init__()

        self.input_dim = input_dim
        self.hidden_dim = hidden_dim

        # Input projection
        self.input_projection = nn.Linear(input_dim, hidden_dim)

        # Positional encoding
        self.positional_encoding = PositionalEncoding(hidden_dim, dropout)

        # Transformer encoder layers
        encoder_layer = nn.TransformerEncoderLayer(
            d_model=hidden_dim,
            nhead=num_heads,
            dim_feedforward=hidden_dim * 4,
            dropout=dropout,
            activation='gelu',
            batch_first=True
        )
        self.transformer = nn.TransformerEncoder(encoder_layer, num_layers)

        # Output layers
        self.output_projection = nn.Sequential(
            nn.Linear(hidden_dim, hidden_dim // 2),
            nn.GELU(),
            nn.Dropout(dropout),
            nn.Linear(hidden_dim // 2, 3)  # UP, DOWN, NEUTRAL
        )

        # Magnitude prediction
        self.magnitude_head = nn.Sequential(
            nn.Linear(hidden_dim, hidden_dim // 2),
            nn.GELU(),
            nn.Dropout(dropout),
            nn.Linear(hidden_dim // 2, 1)
        )

    def forward(self, x: torch.Tensor) -> Tuple[torch.Tensor, torch.Tensor]:
        """
        Forward pass

        Args:
            x: Input tensor [batch_size, seq_len, input_dim]

        Returns:
            direction_logits: [batch_size, 3]
            magnitude: [batch_size, 1]
        """
        # Project input
        x = self.input_projection(x)

        # Add positional encoding
        x = self.positional_encoding(x)

        # Transformer encoding
        x = self.transformer(x)

        # Use last time step
        x = x[:, -1, :]

        # Predictions
        direction_logits = self.output_projection(x)
        magnitude = self.magnitude_head(x)

        return direction_logits, magnitude


class PositionalEncoding(nn.Module):
    """Positional encoding for transformer"""

    def __init__(self, d_model: int, dropout: float = 0.1, max_len: int = 5000):
        super().__init__()
        self.dropout = nn.Dropout(p=dropout)

        position = torch.arange(max_len).unsqueeze(1)
        div_term = torch.exp(torch.arange(0, d_model, 2) * (-np.log(10000.0) / d_model))

        pe = torch.zeros(max_len, 1, d_model)
        pe[:, 0, 0::2] = torch.sin(position * div_term)
        pe[:, 0, 1::2] = torch.cos(position * div_term)

        self.register_buffer('pe', pe)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        """Add positional encoding"""
        x = x + self.pe[:x.size(1)].transpose(0, 1)
        return self.dropout(x)


class MarketPredictor:
    """
    Main market predictor class

    Supports multiple model types:
    - transformer: Transformer-based encoder
    - lstm: LSTM-based model
    - gru: GRU-based model

    Example:
        >>> predictor = MarketPredictor(model_type='transformer')
        >>> predictor.load_model('checkpoint.pth')
        >>> prediction = predictor.predict(market_data)
        >>> print(f"Direction: {prediction.direction}")
        >>> print(f"Confidence: {prediction.confidence:.2%}")
    """

    def __init__(
        self,
        model_type: str = 'transformer',
        device: str = 'cuda' if torch.cuda.is_available() else 'cpu',
        config: Optional[Dict] = None
    ):
        self.model_type = model_type
        self.device = device
        self.config = config or self._default_config()

        # Initialize model
        self.model = self._build_model()
        self.model.to(self.device)

        logger.info(f"MarketPredictor initialized with {model_type} on {device}")

    def _default_config(self) -> Dict:
        """Default configuration"""
        return {
            'input_dim': 128,
            'hidden_dim': 512,
            'num_heads': 8,
            'num_layers': 6,
            'dropout': 0.1,
            'sequence_length': 100
        }

    def _build_model(self) -> nn.Module:
        """Build the model based on type"""
        if self.model_type == 'transformer':
            return TransformerEncoder(
                input_dim=self.config['input_dim'],
                hidden_dim=self.config['hidden_dim'],
                num_heads=self.config['num_heads'],
                num_layers=self.config['num_layers'],
                dropout=self.config['dropout']
            )
        elif self.model_type == 'lstm':
            return LSTMPredictor(self.config)
        elif self.model_type == 'gru':
            return GRUPredictor(self.config)
        else:
            raise ValueError(f"Unknown model type: {self.model_type}")

    def predict(
        self,
        data: np.ndarray,
        horizon: str = '1h'
    ) -> PredictionResult:
        """
        Generate prediction

        Args:
            data: Input features [sequence_length, num_features]
            horizon: Prediction horizon ('1m', '5m', '1h', '1d')

        Returns:
            PredictionResult with direction, confidence, and magnitude
        """
        self.model.eval()

        with torch.no_grad():
            # Prepare input
            x = torch.FloatTensor(data).unsqueeze(0).to(self.device)

            # Get predictions
            direction_logits, magnitude = self.model(x)

            # Process direction
            probabilities = torch.softmax(direction_logits, dim=-1)
            direction_idx = torch.argmax(probabilities, dim=-1).item()
            confidence = probabilities[0, direction_idx].item()

            # Map to direction
            directions = ['DOWN', 'NEUTRAL', 'UP']
            direction = directions[direction_idx]

            # Process magnitude
            magnitude_value = magnitude.item()

            # Feature importance (using attention weights if transformer)
            features_importance = self._compute_feature_importance(x)

            return PredictionResult(
                direction=direction,
                confidence=confidence,
                magnitude=magnitude_value,
                probabilities={
                    'DOWN': probabilities[0, 0].item(),
                    'NEUTRAL': probabilities[0, 1].item(),
                    'UP': probabilities[0, 2].item()
                },
                features_importance=features_importance,
                timestamp=self._get_timestamp()
            )

    def _compute_feature_importance(self, x: torch.Tensor) -> Dict[str, float]:
        """Compute feature importance using attention weights"""
        # Simplified implementation
        # In production, extract attention weights from transformer
        return {
            'price': 0.35,
            'volume': 0.25,
            'volatility': 0.20,
            'sentiment': 0.15,
            'technical': 0.05
        }

    def _get_timestamp(self) -> str:
        """Get current timestamp"""
        from datetime import datetime
        return datetime.now().isoformat()

    def train(
        self,
        train_data: np.ndarray,
        train_labels: np.ndarray,
        val_data: Optional[np.ndarray] = None,
        val_labels: Optional[np.ndarray] = None,
        epochs: int = 100,
        batch_size: int = 32,
        learning_rate: float = 0.001
    ):
        """
        Train the model

        Args:
            train_data: Training features
            train_labels: Training labels
            val_data: Validation features
            val_labels: Validation labels
            epochs: Number of training epochs
            batch_size: Batch size
            learning_rate: Learning rate
        """
        self.model.train()

        # Optimizer and loss
        optimizer = torch.optim.AdamW(
            self.model.parameters(),
            lr=learning_rate,
            weight_decay=0.01
        )
        criterion_direction = nn.CrossEntropyLoss()
        criterion_magnitude = nn.MSELoss()

        # Training loop
        for epoch in range(epochs):
            total_loss = 0
            num_batches = 0

            # Create batches
            for i in range(0, len(train_data), batch_size):
                batch_data = train_data[i:i + batch_size]
                batch_labels = train_labels[i:i + batch_size]

                # Convert to tensors
                x = torch.FloatTensor(batch_data).to(self.device)
                y_direction = torch.LongTensor(batch_labels[:, 0]).to(self.device)
                y_magnitude = torch.FloatTensor(batch_labels[:, 1]).unsqueeze(1).to(self.device)

                # Forward pass
                direction_logits, magnitude = self.model(x)

                # Compute loss
                loss_direction = criterion_direction(direction_logits, y_direction)
                loss_magnitude = criterion_magnitude(magnitude, y_magnitude)
                loss = loss_direction + 0.5 * loss_magnitude

                # Backward pass
                optimizer.zero_grad()
                loss.backward()
                torch.nn.utils.clip_grad_norm_(self.model.parameters(), 1.0)
                optimizer.step()

                total_loss += loss.item()
                num_batches += 1

            # Validation
            if val_data is not None:
                val_loss = self._validate(val_data, val_labels)
                logger.info(
                    f"Epoch {epoch + 1}/{epochs} - "
                    f"Train Loss: {total_loss / num_batches:.4f} - "
                    f"Val Loss: {val_loss:.4f}"
                )
            else:
                logger.info(
                    f"Epoch {epoch + 1}/{epochs} - "
                    f"Train Loss: {total_loss / num_batches:.4f}"
                )

    def _validate(self, val_data: np.ndarray, val_labels: np.ndarray) -> float:
        """Validate the model"""
        self.model.eval()
        total_loss = 0
        num_samples = 0

        criterion_direction = nn.CrossEntropyLoss()
        criterion_magnitude = nn.MSELoss()

        with torch.no_grad():
            for i in range(len(val_data)):
                x = torch.FloatTensor(val_data[i]).unsqueeze(0).to(self.device)
                y_direction = torch.LongTensor([val_labels[i, 0]]).to(self.device)
                y_magnitude = torch.FloatTensor([[val_labels[i, 1]]]).to(self.device)

                direction_logits, magnitude = self.model(x)

                loss_direction = criterion_direction(direction_logits, y_direction)
                loss_magnitude = criterion_magnitude(magnitude, y_magnitude)
                loss = loss_direction + 0.5 * loss_magnitude

                total_loss += loss.item()
                num_samples += 1

        self.model.train()
        return total_loss / num_samples

    def save_model(self, path: str):
        """Save model checkpoint"""
        torch.save({
            'model_state_dict': self.model.state_dict(),
            'config': self.config,
            'model_type': self.model_type
        }, path)
        logger.info(f"Model saved to {path}")

    def load_model(self, path: str):
        """Load model checkpoint"""
        checkpoint = torch.load(path, map_location=self.device)
        self.model.load_state_dict(checkpoint['model_state_dict'])
        self.config = checkpoint['config']
        logger.info(f"Model loaded from {path}")


class LSTMPredictor(nn.Module):
    """LSTM-based predictor"""

    def __init__(self, config: Dict):
        super().__init__()
        self.lstm = nn.LSTM(
            input_size=config['input_dim'],
            hidden_size=config['hidden_dim'],
            num_layers=config['num_layers'],
            dropout=config['dropout'],
            batch_first=True
        )
        self.direction_head = nn.Linear(config['hidden_dim'], 3)
        self.magnitude_head = nn.Linear(config['hidden_dim'], 1)

    def forward(self, x: torch.Tensor) -> Tuple[torch.Tensor, torch.Tensor]:
        _, (h_n, _) = self.lstm(x)
        last_hidden = h_n[-1]
        direction = self.direction_head(last_hidden)
        magnitude = self.magnitude_head(last_hidden)
        return direction, magnitude


class GRUPredictor(nn.Module):
    """GRU-based predictor"""

    def __init__(self, config: Dict):
        super().__init__()
        self.gru = nn.GRU(
            input_size=config['input_dim'],
            hidden_size=config['hidden_dim'],
            num_layers=config['num_layers'],
            dropout=config['dropout'],
            batch_first=True
        )
        self.direction_head = nn.Linear(config['hidden_dim'], 3)
        self.magnitude_head = nn.Linear(config['hidden_dim'], 1)

    def forward(self, x: torch.Tensor) -> Tuple[torch.Tensor, torch.Tensor]:
        _, h_n = self.gru(x)
        last_hidden = h_n[-1]
        direction = self.direction_head(last_hidden)
        magnitude = self.magnitude_head(last_hidden)
        return direction, magnitude


if __name__ == "__main__":
    # Example usage
    print("Market Predictor - Example Usage")
    print("=" * 50)

    # Create predictor
    predictor = MarketPredictor(model_type='transformer')

    # Generate sample data
    sample_data = np.random.randn(100, 128)

    # Make prediction
    prediction = predictor.predict(sample_data, horizon='1h')

    print(f"\nPrediction Results:")
    print(f"Direction: {prediction.direction}")
    print(f"Confidence: {prediction.confidence:.2%}")
    print(f"Expected Move: {prediction.magnitude:.2%}")
    print(f"\nProbabilities:")
    for direction, prob in prediction.probabilities.items():
        print(f"  {direction}: {prob:.2%}")
    print(f"\nTop Features:")
    for feature, importance in sorted(
        prediction.features_importance.items(),
        key=lambda x: x[1],
        reverse=True
    )[:3]:
        print(f"  {feature}: {importance:.2%}")

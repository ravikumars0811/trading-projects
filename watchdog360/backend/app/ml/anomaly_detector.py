"""
AI/ML Anomaly Detection System
Uses Isolation Forest and statistical methods to detect anomalies in server metrics
"""
import numpy as np
import pandas as pd
from typing import Dict, List, Tuple, Any
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
from datetime import datetime, timedelta
import joblib
import json
import logging

logger = logging.getLogger(__name__)


class AnomalyDetector:
    """
    Detects anomalies in server metrics using machine learning
    """

    def __init__(self, contamination: float = 0.1):
        """
        Initialize anomaly detector

        Args:
            contamination: Expected proportion of outliers (0.1 = 10%)
        """
        self.contamination = contamination
        self.model = IsolationForest(
            contamination=contamination,
            random_state=42,
            n_estimators=100
        )
        self.scaler = StandardScaler()
        self.is_trained = False
        self.feature_columns = [
            'cpu_percent',
            'memory_percent',
            'load_average_1',
            'io_read_bytes',
            'io_write_bytes',
            'network_bytes_sent',
            'network_bytes_recv'
        ]

    def prepare_features(self, metrics: List[Dict[str, Any]]) -> pd.DataFrame:
        """
        Prepare features from raw metrics data

        Args:
            metrics: List of metric dictionaries

        Returns:
            DataFrame with prepared features
        """
        data = []

        for metric in metrics:
            features = {
                'cpu_percent': metric.get('cpu_percent', 0),
                'memory_percent': metric.get('memory_percent', 0),
                'load_average_1': metric.get('load_average_1', 0),
                'io_read_bytes': metric.get('io_read_bytes', 0),
                'io_write_bytes': metric.get('io_write_bytes', 0),
                'network_bytes_sent': metric.get('network_bytes_sent', 0),
                'network_bytes_recv': metric.get('network_bytes_recv', 0),
            }
            data.append(features)

        df = pd.DataFrame(data)
        return df[self.feature_columns]

    def train(self, historical_metrics: List[Dict[str, Any]]) -> bool:
        """
        Train the anomaly detection model

        Args:
            historical_metrics: List of historical metrics

        Returns:
            True if training successful
        """
        if len(historical_metrics) < 100:
            logger.warning("Insufficient data for training. Need at least 100 samples.")
            return False

        try:
            # Prepare features
            X = self.prepare_features(historical_metrics)

            # Scale features
            X_scaled = self.scaler.fit_transform(X)

            # Train model
            self.model.fit(X_scaled)
            self.is_trained = True

            logger.info(f"Anomaly detection model trained with {len(historical_metrics)} samples")
            return True

        except Exception as e:
            logger.error(f"Error training anomaly detection model: {e}")
            return False

    def detect(self, current_metrics: Dict[str, Any]) -> Tuple[bool, float, List[str]]:
        """
        Detect if current metrics are anomalous

        Args:
            current_metrics: Current metric values

        Returns:
            Tuple of (is_anomaly, anomaly_score, affected_metrics)
        """
        if not self.is_trained:
            return False, 0.0, []

        try:
            # Prepare features
            X = self.prepare_features([current_metrics])
            X_scaled = self.scaler.transform(X)

            # Predict
            prediction = self.model.predict(X_scaled)[0]
            anomaly_score = abs(self.model.score_samples(X_scaled)[0])

            is_anomaly = prediction == -1

            # Identify affected metrics
            affected_metrics = []
            if is_anomaly:
                affected_metrics = self._identify_affected_metrics(current_metrics)

            return is_anomaly, float(anomaly_score), affected_metrics

        except Exception as e:
            logger.error(f"Error detecting anomaly: {e}")
            return False, 0.0, []

    def _identify_affected_metrics(self, metrics: Dict[str, Any]) -> List[str]:
        """
        Identify which metrics are contributing to the anomaly

        Args:
            metrics: Metric values

        Returns:
            List of affected metric names
        """
        affected = []

        # CPU threshold
        if metrics.get('cpu_percent', 0) > 80:
            affected.append('CPU')

        # Memory threshold
        if metrics.get('memory_percent', 0) > 85:
            affected.append('Memory')

        # Load average (assuming 4 core system)
        if metrics.get('load_average_1', 0) > 4:
            affected.append('Load Average')

        # High I/O
        if metrics.get('io_read_bytes', 0) + metrics.get('io_write_bytes', 0) > 1e9:  # 1GB/s
            affected.append('Disk I/O')

        # High network traffic
        if metrics.get('network_bytes_sent', 0) + metrics.get('network_bytes_recv', 0) > 1e9:  # 1GB/s
            affected.append('Network')

        return affected or ['Unknown']

    def save_model(self, filepath: str):
        """Save trained model to disk"""
        if not self.is_trained:
            raise ValueError("Model not trained yet")

        model_data = {
            'model': self.model,
            'scaler': self.scaler,
            'feature_columns': self.feature_columns,
            'contamination': self.contamination
        }
        joblib.dump(model_data, filepath)
        logger.info(f"Model saved to {filepath}")

    def load_model(self, filepath: str):
        """Load trained model from disk"""
        model_data = joblib.load(filepath)
        self.model = model_data['model']
        self.scaler = model_data['scaler']
        self.feature_columns = model_data['feature_columns']
        self.contamination = model_data['contamination']
        self.is_trained = True
        logger.info(f"Model loaded from {filepath}")


class StatisticalAnomalyDetector:
    """
    Statistical anomaly detection using Z-score and moving averages
    """

    def __init__(self, threshold: float = 3.0):
        """
        Initialize statistical detector

        Args:
            threshold: Z-score threshold for anomaly detection
        """
        self.threshold = threshold

    def detect_zscore(self, values: List[float]) -> Tuple[bool, float]:
        """
        Detect anomaly using Z-score

        Args:
            values: Historical values with current value as last element

        Returns:
            Tuple of (is_anomaly, z_score)
        """
        if len(values) < 10:
            return False, 0.0

        current_value = values[-1]
        historical = values[:-1]

        mean = np.mean(historical)
        std = np.std(historical)

        if std == 0:
            return False, 0.0

        z_score = abs((current_value - mean) / std)
        is_anomaly = z_score > self.threshold

        return is_anomaly, float(z_score)

    def detect_iqr(self, values: List[float]) -> Tuple[bool, float]:
        """
        Detect anomaly using Interquartile Range (IQR)

        Args:
            values: Historical values with current value as last element

        Returns:
            Tuple of (is_anomaly, iqr_score)
        """
        if len(values) < 10:
            return False, 0.0

        current_value = values[-1]
        historical = values[:-1]

        q1 = np.percentile(historical, 25)
        q3 = np.percentile(historical, 75)
        iqr = q3 - q1

        lower_bound = q1 - 1.5 * iqr
        upper_bound = q3 + 1.5 * iqr

        is_anomaly = current_value < lower_bound or current_value > upper_bound
        iqr_score = min(abs(current_value - lower_bound), abs(current_value - upper_bound)) / iqr if iqr > 0 else 0

        return is_anomaly, float(iqr_score)

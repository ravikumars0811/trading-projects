"""
AI/ML Anomaly Detection Engine

Features:
1. Response time anomaly detection using Isolation Forest
2. Pattern recognition for regular outages
3. Predictive maintenance using historical data
4. Auto-threshold adjustment based on learned patterns
"""

import numpy as np
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
from typing import List, Dict, Optional, Tuple
import joblib
from datetime import datetime, timedelta
import os


class AnomalyDetector:
    """
    ML-based anomaly detector for API monitoring
    Uses Isolation Forest algorithm for unsupervised anomaly detection
    """

    def __init__(self, contamination: float = 0.1):
        """
        Initialize the anomaly detector

        Args:
            contamination: Expected proportion of outliers (default 10%)
        """
        self.contamination = contamination
        self.model = IsolationForest(
            contamination=contamination,
            random_state=42,
            n_estimators=100,
            max_samples='auto',
            n_jobs=-1
        )
        self.scaler = StandardScaler()
        self.is_trained = False
        self.feature_history: List[np.ndarray] = []

    def extract_features(self, health_checks: List[Dict]) -> np.ndarray:
        """
        Extract features from health check data

        Features:
        - Response time
        - Status code (encoded)
        - Hour of day
        - Day of week
        - Time since last check
        - Rolling average response time
        """
        if not health_checks:
            return np.array([])

        features = []
        for i, check in enumerate(health_checks):
            response_time = check.get('response_time', 0) or 0
            status_code = check.get('status_code', 0) or 0
            checked_at = check.get('checked_at', datetime.utcnow())

            if isinstance(checked_at, str):
                checked_at = datetime.fromisoformat(checked_at.replace('Z', '+00:00'))

            # Time-based features
            hour_of_day = checked_at.hour
            day_of_week = checked_at.weekday()

            # Calculate rolling average (last 5 checks)
            start_idx = max(0, i - 5)
            recent_response_times = [
                hc.get('response_time', 0) or 0
                for hc in health_checks[start_idx:i+1]
            ]
            rolling_avg = np.mean(recent_response_times) if recent_response_times else 0

            # Status code encoding (success=1, client_error=2, server_error=3, timeout=4)
            status_encoded = self._encode_status(status_code)

            feature_vector = [
                response_time,
                status_encoded,
                hour_of_day,
                day_of_week,
                rolling_avg,
                response_time - rolling_avg,  # Deviation from rolling average
            ]
            features.append(feature_vector)

        return np.array(features)

    def _encode_status(self, status_code: int) -> int:
        """Encode HTTP status codes into categories"""
        if status_code == 0:
            return 4  # Timeout or connection error
        elif 200 <= status_code < 300:
            return 1  # Success
        elif 400 <= status_code < 500:
            return 2  # Client error
        elif 500 <= status_code < 600:
            return 3  # Server error
        else:
            return 0  # Unknown

    def train(self, health_checks: List[Dict]) -> bool:
        """
        Train the anomaly detection model

        Args:
            health_checks: List of historical health check data

        Returns:
            True if training successful, False otherwise
        """
        if len(health_checks) < 50:  # Need minimum data for training
            return False

        features = self.extract_features(health_checks)
        if len(features) == 0:
            return False

        # Normalize features
        self.scaler.fit(features)
        features_scaled = self.scaler.transform(features)

        # Train model
        self.model.fit(features_scaled)
        self.is_trained = True

        return True

    def predict(self, health_check: Dict) -> Tuple[bool, float]:
        """
        Predict if a health check is anomalous

        Args:
            health_check: Single health check data point

        Returns:
            Tuple of (is_anomaly, anomaly_score)
            - is_anomaly: True if anomalous
            - anomaly_score: Anomaly score (lower = more anomalous)
        """
        if not self.is_trained:
            return False, 0.0

        # Add to history for rolling features
        self.feature_history.append(health_check)
        if len(self.feature_history) > 100:  # Keep last 100 checks
            self.feature_history.pop(0)

        features = self.extract_features([health_check])
        if len(features) == 0:
            return False, 0.0

        features_scaled = self.scaler.transform(features)

        # Predict
        prediction = self.model.predict(features_scaled)[0]
        anomaly_score = self.model.score_samples(features_scaled)[0]

        is_anomaly = prediction == -1  # -1 indicates anomaly in Isolation Forest

        return is_anomaly, float(anomaly_score)

    def get_dynamic_threshold(self, health_checks: List[Dict]) -> float:
        """
        Calculate dynamic response time threshold based on historical data

        Args:
            health_checks: Recent health check history

        Returns:
            Recommended threshold in milliseconds
        """
        if len(health_checks) < 10:
            return 5000.0  # Default threshold

        response_times = [
            check.get('response_time', 0)
            for check in health_checks
            if check.get('is_success', False)
        ]

        if not response_times:
            return 5000.0

        # Use 95th percentile + margin
        percentile_95 = np.percentile(response_times, 95)
        margin = np.std(response_times) * 1.5

        return float(percentile_95 + margin)

    def predict_downtime_probability(self, health_checks: List[Dict]) -> float:
        """
        Predict probability of upcoming downtime based on recent patterns

        Args:
            health_checks: Recent health check history

        Returns:
            Probability score between 0 and 1
        """
        if len(health_checks) < 20:
            return 0.0

        recent_checks = health_checks[-20:]  # Last 20 checks

        # Calculate degradation indicators
        response_times = [c.get('response_time', 0) or 0 for c in recent_checks]
        failure_rate = sum(1 for c in recent_checks if not c.get('is_success', False)) / len(recent_checks)

        # Check if response times are increasing
        if len(response_times) >= 10:
            first_half_avg = np.mean(response_times[:10])
            second_half_avg = np.mean(response_times[10:])
            response_time_trend = (second_half_avg - first_half_avg) / max(first_half_avg, 1)
        else:
            response_time_trend = 0

        # Combine indicators
        probability = min(1.0, (failure_rate * 0.6) + (max(0, response_time_trend) * 0.4))

        return float(probability)

    def save_model(self, endpoint_id: int, path: str = "./ml_models"):
        """Save trained model to disk"""
        if not self.is_trained:
            return

        os.makedirs(path, exist_ok=True)
        model_path = os.path.join(path, f"anomaly_detector_{endpoint_id}.joblib")
        joblib.dump({
            'model': self.model,
            'scaler': self.scaler,
            'contamination': self.contamination,
        }, model_path)

    def load_model(self, endpoint_id: int, path: str = "./ml_models") -> bool:
        """Load trained model from disk"""
        model_path = os.path.join(path, f"anomaly_detector_{endpoint_id}.joblib")

        if not os.path.exists(model_path):
            return False

        try:
            data = joblib.load(model_path)
            self.model = data['model']
            self.scaler = data['scaler']
            self.contamination = data['contamination']
            self.is_trained = True
            return True
        except Exception:
            return False


class SmartAlertManager:
    """
    Intelligent alert management to reduce alert fatigue

    Features:
    - Alert grouping and deduplication
    - Adaptive cooldown periods
    - Severity classification
    """

    def __init__(self):
        self.alert_history: Dict[int, List[datetime]] = {}

    def should_send_alert(
        self,
        endpoint_id: int,
        alert_type: str,
        cooldown_minutes: int = 5
    ) -> bool:
        """
        Determine if an alert should be sent based on recent history

        Args:
            endpoint_id: Endpoint ID
            alert_type: Type of alert
            cooldown_minutes: Cooldown period in minutes

        Returns:
            True if alert should be sent
        """
        key = f"{endpoint_id}_{alert_type}"

        if key not in self.alert_history:
            self.alert_history[key] = []

        # Remove old alerts outside cooldown window
        cutoff_time = datetime.utcnow() - timedelta(minutes=cooldown_minutes)
        self.alert_history[key] = [
            alert_time for alert_time in self.alert_history[key]
            if alert_time > cutoff_time
        ]

        # Check if we should send alert
        if len(self.alert_history[key]) == 0:
            self.alert_history[key].append(datetime.utcnow())
            return True

        return False

    def calculate_severity(
        self,
        failure_count: int,
        response_time: Optional[float],
        threshold: float
    ) -> str:
        """
        Calculate alert severity

        Returns:
            "critical", "high", "medium", or "low"
        """
        if failure_count >= 3:
            return "critical"
        elif response_time and response_time > threshold * 2:
            return "high"
        elif response_time and response_time > threshold:
            return "medium"
        else:
            return "low"

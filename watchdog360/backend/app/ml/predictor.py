"""
AI/ML Prediction System
Predicts future resource usage using time series forecasting
"""
import numpy as np
import pandas as pd
from typing import Dict, List, Tuple, Optional
from datetime import datetime, timedelta
from prophet import Prophet
import logging

logger = logging.getLogger(__name__)


class MetricsPredictor:
    """
    Predicts future metrics using Prophet time series forecasting
    """

    def __init__(self):
        """Initialize predictor"""
        self.models = {}
        self.metrics_to_predict = ['cpu_percent', 'memory_percent']

    def prepare_data(
        self,
        timestamps: List[datetime],
        values: List[float]
    ) -> pd.DataFrame:
        """
        Prepare data for Prophet

        Args:
            timestamps: List of timestamps
            values: List of metric values

        Returns:
            DataFrame in Prophet format (ds, y)
        """
        df = pd.DataFrame({
            'ds': timestamps,
            'y': values
        })
        return df

    def train(
        self,
        metric_name: str,
        historical_data: Dict[str, List]
    ) -> bool:
        """
        Train prediction model for a specific metric

        Args:
            metric_name: Name of the metric (cpu_percent, memory_percent)
            historical_data: Dict with 'timestamps' and 'values' lists

        Returns:
            True if training successful
        """
        if len(historical_data['timestamps']) < 100:
            logger.warning(f"Insufficient data for training {metric_name}. Need at least 100 samples.")
            return False

        try:
            # Prepare data
            df = self.prepare_data(
                historical_data['timestamps'],
                historical_data['values']
            )

            # Create and train model
            model = Prophet(
                daily_seasonality=True,
                weekly_seasonality=True,
                yearly_seasonality=False,
                changepoint_prior_scale=0.05
            )
            model.fit(df)

            self.models[metric_name] = model
            logger.info(f"Prediction model trained for {metric_name}")
            return True

        except Exception as e:
            logger.error(f"Error training prediction model for {metric_name}: {e}")
            return False

    def predict(
        self,
        metric_name: str,
        periods: int = 12,  # 12 periods = 2 minutes (10s intervals)
        freq: str = '10S'
    ) -> Optional[Dict[str, any]]:
        """
        Predict future values for a metric

        Args:
            metric_name: Name of the metric
            periods: Number of periods to predict
            freq: Frequency of predictions (10S = 10 seconds)

        Returns:
            Dict with predictions or None if model not trained
        """
        if metric_name not in self.models:
            logger.warning(f"No trained model for {metric_name}")
            return None

        try:
            model = self.models[metric_name]

            # Create future dataframe
            future = model.make_future_dataframe(periods=periods, freq=freq)

            # Make prediction
            forecast = model.predict(future)

            # Get last prediction
            last_prediction = forecast.iloc[-1]

            # Calculate trend
            recent_predictions = forecast.tail(periods)
            trend = self._calculate_trend(recent_predictions['yhat'].tolist())

            # Calculate confidence
            confidence = self._calculate_confidence(
                last_prediction['yhat'],
                last_prediction['yhat_lower'],
                last_prediction['yhat_upper']
            )

            return {
                'predicted_value': float(last_prediction['yhat']),
                'lower_bound': float(last_prediction['yhat_lower']),
                'upper_bound': float(last_prediction['yhat_upper']),
                'trend': trend,
                'confidence': confidence,
                'predicted_at': last_prediction['ds'],
                'all_predictions': recent_predictions[['ds', 'yhat', 'yhat_lower', 'yhat_upper']].to_dict('records')
            }

        except Exception as e:
            logger.error(f"Error predicting {metric_name}: {e}")
            return None

    def _calculate_trend(self, values: List[float]) -> str:
        """
        Calculate trend direction

        Args:
            values: List of predicted values

        Returns:
            'increasing', 'decreasing', or 'stable'
        """
        if len(values) < 2:
            return 'stable'

        # Calculate slope
        x = np.arange(len(values))
        slope = np.polyfit(x, values, 1)[0]

        if slope > 0.5:
            return 'increasing'
        elif slope < -0.5:
            return 'decreasing'
        else:
            return 'stable'

    def _calculate_confidence(
        self,
        prediction: float,
        lower: float,
        upper: float
    ) -> float:
        """
        Calculate prediction confidence

        Args:
            prediction: Predicted value
            lower: Lower bound
            upper: Upper bound

        Returns:
            Confidence score (0-1)
        """
        if prediction == 0:
            return 0.5

        interval_width = upper - lower
        relative_width = interval_width / abs(prediction)

        # Convert to confidence (narrower interval = higher confidence)
        confidence = max(0, min(1, 1 - (relative_width / 2)))

        return confidence

    def detect_future_issues(
        self,
        metric_name: str,
        threshold: float,
        periods: int = 12
    ) -> Optional[Dict[str, any]]:
        """
        Detect if a metric will exceed threshold in the near future

        Args:
            metric_name: Name of the metric
            threshold: Threshold value
            periods: Number of periods to check

        Returns:
            Dict with issue details or None if no issue predicted
        """
        prediction = self.predict(metric_name, periods)

        if not prediction:
            return None

        predicted_value = prediction['predicted_value']

        if predicted_value > threshold:
            # Calculate time until threshold is exceeded
            all_predictions = prediction['all_predictions']
            time_to_threshold = None

            for i, pred in enumerate(all_predictions):
                if pred['yhat'] > threshold:
                    time_to_threshold = pred['ds']
                    break

            return {
                'metric_name': metric_name,
                'predicted_value': predicted_value,
                'threshold': threshold,
                'will_exceed': True,
                'time_to_threshold': time_to_threshold,
                'confidence': prediction['confidence'],
                'trend': prediction['trend']
            }

        return None


class SimplePredictor:
    """
    Simple linear regression predictor for quick predictions
    """

    def __init__(self):
        """Initialize simple predictor"""
        pass

    def predict_linear(
        self,
        values: List[float],
        periods: int = 12
    ) -> Dict[str, any]:
        """
        Simple linear prediction

        Args:
            values: Historical values
            periods: Number of periods to predict

        Returns:
            Dict with prediction
        """
        if len(values) < 10:
            return {'predicted_value': values[-1] if values else 0, 'trend': 'stable'}

        # Fit linear model
        x = np.arange(len(values))
        coeffs = np.polyfit(x, values, 1)

        # Predict
        future_x = len(values) + periods - 1
        predicted_value = coeffs[0] * future_x + coeffs[1]

        # Ensure non-negative
        predicted_value = max(0, predicted_value)

        # Determine trend
        if coeffs[0] > 0.5:
            trend = 'increasing'
        elif coeffs[0] < -0.5:
            trend = 'decreasing'
        else:
            trend = 'stable'

        return {
            'predicted_value': float(predicted_value),
            'trend': trend,
            'slope': float(coeffs[0])
        }

    def predict_moving_average(
        self,
        values: List[float],
        window: int = 10
    ) -> float:
        """
        Predict using moving average

        Args:
            values: Historical values
            window: Window size for moving average

        Returns:
            Predicted value
        """
        if len(values) < window:
            return values[-1] if values else 0

        return float(np.mean(values[-window:]))

"""
Metrics processing service with AI/ML integration
"""
from datetime import datetime, timedelta
from typing import Dict, Any, List
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, and_

from ..models.models import ServerMetric, Server
from ..schemas.schemas import MetricsData
from ..ml.anomaly_detector import AnomalyDetector, StatisticalAnomalyDetector
from ..ml.predictor import MetricsPredictor, SimplePredictor
from ..core.config import settings
import logging

logger = logging.getLogger(__name__)


class MetricsService:
    """Service for processing and storing metrics with AI/ML"""

    def __init__(self, db: AsyncSession):
        self.db = db
        self.anomaly_detector = AnomalyDetector()
        self.stat_detector = StatisticalAnomalyDetector()
        self.predictor = SimplePredictor()

    async def process_and_store_metrics(
        self,
        server_id: int,
        metrics_data: MetricsData
    ) -> ServerMetric:
        """
        Process incoming metrics with AI/ML and store

        Args:
            server_id: Server ID
            metrics_data: Incoming metrics

        Returns:
            Created ServerMetric record
        """
        # Extract load average
        load_avg = metrics_data.cpu.load_average
        load_avg_1 = load_avg[0] if len(load_avg) > 0 else 0
        load_avg_5 = load_avg[1] if len(load_avg) > 1 else 0
        load_avg_15 = load_avg[2] if len(load_avg) > 2 else 0

        # Convert processes to dict
        processes_data = [p.dict() for p in metrics_data.processes]

        # Convert disks to dict
        disks_data = [d.dict() for d in metrics_data.disk]

        # Create base metric record
        metric = ServerMetric(
            server_id=server_id,
            timestamp=metrics_data.timestamp,
            # CPU
            cpu_percent=metrics_data.cpu.cpu_percent,
            cpu_count=metrics_data.cpu.cpu_count,
            load_average_1=load_avg_1,
            load_average_5=load_avg_5,
            load_average_15=load_avg_15,
            # Memory
            memory_total=metrics_data.memory.memory_total,
            memory_used=metrics_data.memory.memory_used,
            memory_available=metrics_data.memory.memory_available,
            memory_percent=metrics_data.memory.memory_percent,
            swap_total=metrics_data.memory.swap_total,
            swap_used=metrics_data.memory.swap_used,
            swap_percent=metrics_data.memory.swap_percent,
            # Disk
            disk_metrics=disks_data,
            # I/O
            io_read_count=metrics_data.io.read_count,
            io_write_count=metrics_data.io.write_count,
            io_read_bytes=metrics_data.io.read_bytes,
            io_write_bytes=metrics_data.io.write_bytes,
            # Network
            network_bytes_sent=metrics_data.network.bytes_sent,
            network_bytes_recv=metrics_data.network.bytes_recv,
            network_packets_sent=metrics_data.network.packets_sent,
            network_packets_recv=metrics_data.network.packets_recv,
            # Processes
            top_processes=processes_data
        )

        # AI/ML: Anomaly detection
        if settings.ANOMALY_DETECTION_ENABLED:
            try:
                is_anomaly, anomaly_score = await self._detect_anomaly(
                    server_id,
                    metrics_data
                )
                metric.is_anomaly = is_anomaly
                metric.anomaly_score = anomaly_score
            except Exception as e:
                logger.error(f"Error in anomaly detection: {e}")

        # AI/ML: Predictions
        if settings.PREDICTION_ENABLED:
            try:
                predictions = await self._predict_metrics(server_id)
                if predictions:
                    metric.predicted_cpu = predictions.get('cpu_percent')
                    metric.predicted_memory = predictions.get('memory_percent')
            except Exception as e:
                logger.error(f"Error in predictions: {e}")

        # Store metric
        self.db.add(metric)
        await self.db.commit()
        await self.db.refresh(metric)

        return metric

    async def _detect_anomaly(
        self,
        server_id: int,
        metrics_data: MetricsData
    ) -> tuple[bool, float]:
        """
        Detect anomalies using statistical methods

        Args:
            server_id: Server ID
            metrics_data: Current metrics

        Returns:
            Tuple of (is_anomaly, score)
        """
        # Get historical metrics (last 24 hours)
        time_threshold = datetime.utcnow() - timedelta(hours=24)

        result = await self.db.execute(
            select(ServerMetric)
            .where(
                and_(
                    ServerMetric.server_id == server_id,
                    ServerMetric.timestamp >= time_threshold
                )
            )
            .order_by(ServerMetric.timestamp)
        )
        historical_metrics = result.scalars().all()

        if len(historical_metrics) < 10:
            # Not enough data for anomaly detection
            return False, 0.0

        # Extract CPU values
        cpu_values = [m.cpu_percent for m in historical_metrics]
        cpu_values.append(metrics_data.cpu.cpu_percent)

        # Detect using Z-score
        is_cpu_anomaly, cpu_score = self.stat_detector.detect_zscore(cpu_values)

        # Extract memory values
        memory_values = [m.memory_percent for m in historical_metrics]
        memory_values.append(metrics_data.memory.memory_percent)

        # Detect using Z-score
        is_mem_anomaly, mem_score = self.stat_detector.detect_zscore(memory_values)

        # Combined detection
        is_anomaly = is_cpu_anomaly or is_mem_anomaly
        anomaly_score = max(cpu_score, mem_score)

        return is_anomaly, anomaly_score

    async def _predict_metrics(
        self,
        server_id: int
    ) -> Dict[str, float]:
        """
        Predict future metrics using simple linear prediction

        Args:
            server_id: Server ID

        Returns:
            Dict with predicted values
        """
        # Get recent metrics (last 2 hours for prediction)
        time_threshold = datetime.utcnow() - timedelta(hours=2)

        result = await self.db.execute(
            select(ServerMetric)
            .where(
                and_(
                    ServerMetric.server_id == server_id,
                    ServerMetric.timestamp >= time_threshold
                )
            )
            .order_by(ServerMetric.timestamp)
        )
        recent_metrics = result.scalars().all()

        if len(recent_metrics) < 10:
            return {}

        # Predict CPU
        cpu_values = [m.cpu_percent for m in recent_metrics]
        cpu_prediction = self.predictor.predict_linear(cpu_values, periods=12)

        # Predict Memory
        memory_values = [m.memory_percent for m in recent_metrics]
        memory_prediction = self.predictor.predict_linear(memory_values, periods=12)

        return {
            'cpu_percent': cpu_prediction['predicted_value'],
            'memory_percent': memory_prediction['predicted_value']
        }

    async def get_historical_metrics(
        self,
        server_id: int,
        hours: int = 24
    ) -> List[ServerMetric]:
        """
        Get historical metrics for a server

        Args:
            server_id: Server ID
            hours: Number of hours to retrieve

        Returns:
            List of metrics
        """
        time_threshold = datetime.utcnow() - timedelta(hours=hours)

        result = await self.db.execute(
            select(ServerMetric)
            .where(
                and_(
                    ServerMetric.server_id == server_id,
                    ServerMetric.timestamp >= time_threshold
                )
            )
            .order_by(ServerMetric.timestamp)
        )

        return result.scalars().all()

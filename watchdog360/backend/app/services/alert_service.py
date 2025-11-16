"""
Alert service for creating and managing alerts
"""
from datetime import datetime, timedelta
from typing import List, Optional
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, and_, desc

from ..models.models import Alert, Server, ServerMetric
from ..schemas.schemas import MetricsData, AlertCreate
from ..core.config import settings
import logging

logger = logging.getLogger(__name__)


class AlertService:
    """Service for managing alerts"""

    def __init__(self, db: AsyncSession):
        self.db = db

    async def check_and_create_alerts(
        self,
        server_id: int,
        metrics_data: MetricsData,
        metric_record: ServerMetric
    ):
        """
        Check metrics and create alerts if thresholds exceeded

        Args:
            server_id: Server ID
            metrics_data: Current metrics
            metric_record: Stored metric record with AI/ML data
        """
        alerts_to_create = []

        # Check CPU
        if metrics_data.cpu.cpu_percent > settings.ALERT_HIGH_CPU_THRESHOLD:
            alerts_to_create.append(
                AlertCreate(
                    server_id=server_id,
                    alert_type="cpu",
                    severity="critical" if metrics_data.cpu.cpu_percent > 90 else "warning",
                    title=f"High CPU Usage",
                    message=f"CPU usage is at {metrics_data.cpu.cpu_percent:.1f}%",
                    metric_value=metrics_data.cpu.cpu_percent,
                    threshold_value=settings.ALERT_HIGH_CPU_THRESHOLD
                )
            )

        # Check Memory
        if metrics_data.memory.memory_percent > settings.ALERT_HIGH_MEMORY_THRESHOLD:
            alerts_to_create.append(
                AlertCreate(
                    server_id=server_id,
                    alert_type="memory",
                    severity="critical" if metrics_data.memory.memory_percent > 95 else "warning",
                    title=f"High Memory Usage",
                    message=f"Memory usage is at {metrics_data.memory.memory_percent:.1f}%",
                    metric_value=metrics_data.memory.memory_percent,
                    threshold_value=settings.ALERT_HIGH_MEMORY_THRESHOLD
                )
            )

        # Check Disk
        for disk in metrics_data.disk:
            if disk.percent > settings.ALERT_HIGH_DISK_THRESHOLD:
                alerts_to_create.append(
                    AlertCreate(
                        server_id=server_id,
                        alert_type="disk",
                        severity="critical" if disk.percent > 95 else "warning",
                        title=f"High Disk Usage on {disk.mountpoint}",
                        message=f"Disk usage on {disk.mountpoint} is at {disk.percent:.1f}%",
                        metric_value=disk.percent,
                        threshold_value=settings.ALERT_HIGH_DISK_THRESHOLD
                    )
                )

        # Check for anomalies
        if metric_record.is_anomaly:
            alerts_to_create.append(
                AlertCreate(
                    server_id=server_id,
                    alert_type="anomaly",
                    severity="warning",
                    title="Anomaly Detected",
                    message=f"Unusual behavior detected with anomaly score {metric_record.anomaly_score:.2f}",
                    metric_value=metric_record.anomaly_score,
                    threshold_value=settings.ANOMALY_DETECTION_THRESHOLD
                )
            )

        # Check predictions (if predicted values exceed thresholds)
        if metric_record.predicted_cpu and metric_record.predicted_cpu > settings.ALERT_HIGH_CPU_THRESHOLD:
            alerts_to_create.append(
                AlertCreate(
                    server_id=server_id,
                    alert_type="cpu",
                    severity="info",
                    title="Predicted High CPU Usage",
                    message=f"CPU is predicted to reach {metric_record.predicted_cpu:.1f}% in the next 2 minutes",
                    metric_value=metric_record.predicted_cpu,
                    threshold_value=settings.ALERT_HIGH_CPU_THRESHOLD
                )
            )

        if metric_record.predicted_memory and metric_record.predicted_memory > settings.ALERT_HIGH_MEMORY_THRESHOLD:
            alerts_to_create.append(
                AlertCreate(
                    server_id=server_id,
                    alert_type="memory",
                    severity="info",
                    title="Predicted High Memory Usage",
                    message=f"Memory is predicted to reach {metric_record.predicted_memory:.1f}% in the next 2 minutes",
                    metric_value=metric_record.predicted_memory,
                    threshold_value=settings.ALERT_HIGH_MEMORY_THRESHOLD
                )
            )

        # Create alerts (avoid duplicates)
        for alert_data in alerts_to_create:
            await self._create_alert_if_not_exists(alert_data)

    async def _create_alert_if_not_exists(self, alert_data: AlertCreate):
        """
        Create alert only if similar alert doesn't exist in last 5 minutes

        Args:
            alert_data: Alert data
        """
        time_threshold = datetime.utcnow() - timedelta(minutes=5)

        # Check for existing similar alert
        result = await self.db.execute(
            select(Alert)
            .where(
                and_(
                    Alert.server_id == alert_data.server_id,
                    Alert.alert_type == alert_data.alert_type,
                    Alert.is_resolved == False,
                    Alert.created_at >= time_threshold
                )
            )
        )
        existing_alert = result.scalar_one_or_none()

        if existing_alert:
            logger.debug(f"Similar alert already exists for {alert_data.alert_type}")
            return

        # Create new alert
        new_alert = Alert(
            server_id=alert_data.server_id,
            alert_type=alert_data.alert_type,
            severity=alert_data.severity,
            title=alert_data.title,
            message=alert_data.message,
            metric_value=alert_data.metric_value,
            threshold_value=alert_data.threshold_value,
            is_prediction="Predicted" in alert_data.title
        )

        self.db.add(new_alert)
        await self.db.commit()
        logger.info(f"Created alert: {alert_data.title}")

    async def get_active_alerts(
        self,
        server_id: Optional[int] = None,
        user_id: Optional[int] = None
    ) -> List[Alert]:
        """
        Get active (unresolved) alerts

        Args:
            server_id: Optional server ID filter
            user_id: Optional user ID filter

        Returns:
            List of active alerts
        """
        query = select(Alert).where(Alert.is_resolved == False)

        if server_id:
            query = query.where(Alert.server_id == server_id)

        if user_id:
            query = query.join(Server).where(Server.user_id == user_id)

        query = query.order_by(desc(Alert.created_at))

        result = await self.db.execute(query)
        return result.scalars().all()

    async def resolve_alert(self, alert_id: int) -> Alert:
        """
        Resolve an alert

        Args:
            alert_id: Alert ID

        Returns:
            Updated alert
        """
        result = await self.db.execute(
            select(Alert).where(Alert.id == alert_id)
        )
        alert = result.scalar_one_or_none()

        if not alert:
            raise ValueError(f"Alert {alert_id} not found")

        alert.is_resolved = True
        alert.resolved_at = datetime.utcnow()

        await self.db.commit()
        await self.db.refresh(alert)

        return alert

    async def get_alert_summary(self, user_id: int) -> dict:
        """
        Get summary of alerts for a user

        Args:
            user_id: User ID

        Returns:
            Dict with alert counts
        """
        result = await self.db.execute(
            select(Alert)
            .join(Server)
            .where(
                and_(
                    Server.user_id == user_id,
                    Alert.is_resolved == False
                )
            )
        )
        alerts = result.scalars().all()

        return {
            "total": len(alerts),
            "critical": sum(1 for a in alerts if a.severity == "critical"),
            "warning": sum(1 for a in alerts if a.severity == "warning"),
            "info": sum(1 for a in alerts if a.severity == "info"),
            "by_type": {
                "cpu": sum(1 for a in alerts if a.alert_type == "cpu"),
                "memory": sum(1 for a in alerts if a.alert_type == "memory"),
                "disk": sum(1 for a in alerts if a.alert_type == "disk"),
                "anomaly": sum(1 for a in alerts if a.alert_type == "anomaly"),
            }
        }

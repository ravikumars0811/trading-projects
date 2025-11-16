"""
Metrics API endpoints
"""
from datetime import datetime, timedelta
from typing import List, Optional
from fastapi import APIRouter, Depends, HTTPException, status, Query
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, and_, desc, func
from sqlalchemy.orm import selectinload

from ..core.database import get_db
from ..core.security import get_current_user
from ..models.models import Server, ServerMetric, APIToken, Alert
from ..schemas.schemas import (
    MetricsData,
    MetricsResponse,
    ServerMetricResponse,
    TimeSeriesData
)
from ..services.metrics_service import MetricsService
from ..services.alert_service import AlertService

router = APIRouter(prefix="/metrics", tags=["metrics"])


@router.post("", response_model=MetricsResponse)
async def push_metrics(
    metrics: MetricsData,
    current_user: dict = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """
    Push server metrics (called by agents)
    """
    try:
        # Get or create server
        result = await db.execute(
            select(Server).where(
                and_(
                    Server.hostname == metrics.hostname,
                    Server.user_id == int(current_user['user_id'])
                )
            )
        )
        server = result.scalar_one_or_none()

        if not server:
            # Create new server
            server = Server(
                hostname=metrics.hostname,
                user_id=int(current_user['user_id']),
                is_active=True,
                last_seen_at=datetime.utcnow()
            )
            db.add(server)
            await db.commit()
            await db.refresh(server)
        else:
            # Update last seen
            server.last_seen_at = datetime.utcnow()
            await db.commit()

        # Process metrics with AI/ML
        metrics_service = MetricsService(db)
        metric_record = await metrics_service.process_and_store_metrics(
            server_id=server.id,
            metrics_data=metrics
        )

        # Check for alerts
        alert_service = AlertService(db)
        await alert_service.check_and_create_alerts(server.id, metrics, metric_record)

        return MetricsResponse(
            status="success",
            message="Metrics received and processed",
            server_id=server.id
        )

    except Exception as e:
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Error processing metrics: {str(e)}"
        )


@router.get("/server/{server_id}/latest", response_model=ServerMetricResponse)
async def get_latest_metrics(
    server_id: int,
    current_user: dict = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get latest metrics for a server"""

    # Verify server ownership
    result = await db.execute(
        select(Server).where(
            and_(
                Server.id == server_id,
                Server.user_id == int(current_user['user_id'])
            )
        )
    )
    server = result.scalar_one_or_none()

    if not server:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Server not found"
        )

    # Get latest metric
    result = await db.execute(
        select(ServerMetric)
        .where(ServerMetric.server_id == server_id)
        .order_by(desc(ServerMetric.timestamp))
        .limit(1)
    )
    latest_metric = result.scalar_one_or_none()

    if not latest_metric:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="No metrics found for this server"
        )

    # Calculate average disk usage
    disk_usage_avg = None
    if latest_metric.disk_metrics:
        disk_usages = [disk.get('percent', 0) for disk in latest_metric.disk_metrics]
        disk_usage_avg = sum(disk_usages) / len(disk_usages) if disk_usages else 0

    return ServerMetricResponse(
        id=latest_metric.id,
        server_id=latest_metric.server_id,
        timestamp=latest_metric.timestamp,
        cpu_percent=latest_metric.cpu_percent,
        memory_percent=latest_metric.memory_percent,
        disk_usage_avg=disk_usage_avg,
        anomaly_score=latest_metric.anomaly_score,
        is_anomaly=latest_metric.is_anomaly
    )


@router.get("/server/{server_id}/timeseries", response_model=TimeSeriesData)
async def get_metrics_timeseries(
    server_id: int,
    metric_name: str = Query(..., description="cpu_percent, memory_percent, etc."),
    hours: int = Query(24, description="Number of hours of data to retrieve"),
    current_user: dict = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get time series data for a specific metric"""

    # Verify server ownership
    result = await db.execute(
        select(Server).where(
            and_(
                Server.id == server_id,
                Server.user_id == int(current_user['user_id'])
            )
        )
    )
    server = result.scalar_one_or_none()

    if not server:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Server not found"
        )

    # Get metrics
    time_threshold = datetime.utcnow() - timedelta(hours=hours)

    result = await db.execute(
        select(ServerMetric)
        .where(
            and_(
                ServerMetric.server_id == server_id,
                ServerMetric.timestamp >= time_threshold
            )
        )
        .order_by(ServerMetric.timestamp)
    )
    metrics = result.scalars().all()

    # Extract values
    timestamps = [m.timestamp for m in metrics]
    values = [getattr(m, metric_name, 0) for m in metrics]

    return TimeSeriesData(
        timestamps=timestamps,
        values=values
    )


@router.get("/server/{server_id}/stats")
async def get_server_stats(
    server_id: int,
    current_user: dict = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get comprehensive statistics for a server"""

    # Verify server ownership
    result = await db.execute(
        select(Server).where(
            and_(
                Server.id == server_id,
                Server.user_id == int(current_user['user_id'])
            )
        )
    )
    server = result.scalar_one_or_none()

    if not server:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Server not found"
        )

    # Get stats for last 24 hours
    time_threshold = datetime.utcnow() - timedelta(hours=24)

    result = await db.execute(
        select(ServerMetric)
        .where(
            and_(
                ServerMetric.server_id == server_id,
                ServerMetric.timestamp >= time_threshold
            )
        )
    )
    metrics = result.scalars().all()

    if not metrics:
        return {
            "server_id": server_id,
            "metrics_count": 0,
            "avg_cpu": 0,
            "avg_memory": 0,
            "max_cpu": 0,
            "max_memory": 0
        }

    cpu_values = [m.cpu_percent for m in metrics]
    memory_values = [m.memory_percent for m in metrics]

    return {
        "server_id": server_id,
        "metrics_count": len(metrics),
        "avg_cpu": sum(cpu_values) / len(cpu_values),
        "avg_memory": sum(memory_values) / len(memory_values),
        "max_cpu": max(cpu_values),
        "max_memory": max(memory_values),
        "min_cpu": min(cpu_values),
        "min_memory": min(memory_values),
        "anomalies_detected": sum(1 for m in metrics if m.is_anomaly)
    }

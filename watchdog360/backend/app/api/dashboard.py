"""
Dashboard API endpoints
"""
from typing import List
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, and_, desc, func
from datetime import datetime, timedelta

from ..core.database import get_db
from ..core.security import get_current_user
from ..models.models import Server, ServerMetric, Alert
from ..schemas.schemas import (
    Server as ServerSchema,
    DashboardStats,
    ServerStats,
    Alert as AlertSchema
)
from ..services.alert_service import AlertService

router = APIRouter(prefix="/dashboard", tags=["dashboard"])


@router.get("/stats", response_model=DashboardStats)
async def get_dashboard_stats(
    current_user: dict = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get overall dashboard statistics"""

    user_id = int(current_user['user_id'])

    # Total servers
    result = await db.execute(
        select(func.count(Server.id)).where(Server.user_id == user_id)
    )
    total_servers = result.scalar()

    # Active servers (seen in last 5 minutes)
    time_threshold = datetime.utcnow() - timedelta(minutes=5)
    result = await db.execute(
        select(func.count(Server.id)).where(
            and_(
                Server.user_id == user_id,
                Server.last_seen_at >= time_threshold
            )
        )
    )
    active_servers = result.scalar()

    # Get alert summary
    alert_service = AlertService(db)
    alert_summary = await alert_service.get_alert_summary(user_id)

    # Metrics today
    today = datetime.utcnow().replace(hour=0, minute=0, second=0, microsecond=0)
    result = await db.execute(
        select(func.count(ServerMetric.id))
        .join(Server)
        .where(
            and_(
                Server.user_id == user_id,
                ServerMetric.timestamp >= today
            )
        )
    )
    metrics_today = result.scalar()

    return DashboardStats(
        total_servers=total_servers or 0,
        active_servers=active_servers or 0,
        critical_alerts=alert_summary.get('critical', 0),
        warning_alerts=alert_summary.get('warning', 0),
        total_metrics_today=metrics_today or 0
    )


@router.get("/servers", response_model=List[ServerSchema])
async def get_servers(
    current_user: dict = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get all servers for the current user"""

    user_id = int(current_user['user_id'])

    result = await db.execute(
        select(Server).where(Server.user_id == user_id).order_by(desc(Server.last_seen_at))
    )
    servers = result.scalars().all()

    return servers


@router.get("/servers/{server_id}", response_model=ServerStats)
async def get_server_detail(
    server_id: int,
    current_user: dict = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get detailed stats for a specific server"""

    user_id = int(current_user['user_id'])

    # Get server
    result = await db.execute(
        select(Server).where(
            and_(
                Server.id == server_id,
                Server.user_id == user_id
            )
        )
    )
    server = result.scalar_one_or_none()

    if not server:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Server not found"
        )

    # Get latest metrics
    result = await db.execute(
        select(ServerMetric)
        .where(ServerMetric.server_id == server_id)
        .order_by(desc(ServerMetric.timestamp))
        .limit(1)
    )
    latest_metric = result.scalar_one_or_none()

    # Get CPU and memory trends (last hour)
    time_threshold = datetime.utcnow() - timedelta(hours=1)
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
    recent_metrics = result.scalars().all()

    cpu_trend = [m.cpu_percent for m in recent_metrics]
    memory_trend = [m.memory_percent for m in recent_metrics]

    # Get active alerts count
    alert_service = AlertService(db)
    active_alerts = await alert_service.get_active_alerts(server_id=server_id)

    # Format response
    latest_metrics_response = None
    if latest_metric:
        disk_usage_avg = None
        if latest_metric.disk_metrics:
            disk_usages = [disk.get('percent', 0) for disk in latest_metric.disk_metrics]
            disk_usage_avg = sum(disk_usages) / len(disk_usages) if disk_usages else 0

        latest_metrics_response = {
            "id": latest_metric.id,
            "server_id": latest_metric.server_id,
            "timestamp": latest_metric.timestamp,
            "cpu_percent": latest_metric.cpu_percent,
            "memory_percent": latest_metric.memory_percent,
            "disk_usage_avg": disk_usage_avg,
            "anomaly_score": latest_metric.anomaly_score,
            "is_anomaly": latest_metric.is_anomaly
        }

    return ServerStats(
        server=server,
        latest_metrics=latest_metrics_response,
        cpu_trend=cpu_trend,
        memory_trend=memory_trend,
        active_alerts=len(active_alerts)
    )


@router.get("/alerts", response_model=List[AlertSchema])
async def get_alerts(
    server_id: int = None,
    current_user: dict = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get alerts for the current user"""

    user_id = int(current_user['user_id'])

    alert_service = AlertService(db)
    alerts = await alert_service.get_active_alerts(
        server_id=server_id,
        user_id=user_id
    )

    return alerts


@router.post("/alerts/{alert_id}/resolve")
async def resolve_alert(
    alert_id: int,
    current_user: dict = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Resolve an alert"""

    # Verify alert belongs to user
    result = await db.execute(
        select(Alert)
        .join(Server)
        .where(
            and_(
                Alert.id == alert_id,
                Server.user_id == int(current_user['user_id'])
            )
        )
    )
    alert = result.scalar_one_or_none()

    if not alert:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Alert not found"
        )

    alert_service = AlertService(db)
    resolved_alert = await alert_service.resolve_alert(alert_id)

    return {"status": "success", "message": "Alert resolved"}

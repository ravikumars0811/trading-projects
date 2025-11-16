"""
Health Check API routes
"""

from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from typing import List
from datetime import datetime, timedelta

from app.core.database import get_db
from app.core.security import get_current_user
from app.models.user import User
from app.models.endpoint import Endpoint
from app.models.health_check import HealthCheck
from app.schemas.health_check import HealthCheckResponse, HealthCheckStats

router = APIRouter(prefix="/health-checks", tags=["Health Checks"])


@router.get("/endpoint/{endpoint_id}", response_model=List[HealthCheckResponse])
async def get_health_checks(
    endpoint_id: int,
    limit: int = 100,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get health check history for an endpoint"""

    # Verify endpoint ownership
    endpoint_result = await db.execute(
        select(Endpoint).where(
            Endpoint.id == endpoint_id,
            Endpoint.user_id == current_user.id
        )
    )
    endpoint = endpoint_result.scalar_one_or_none()

    if not endpoint:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Endpoint not found"
        )

    # Get health checks
    result = await db.execute(
        select(HealthCheck)
        .where(HealthCheck.endpoint_id == endpoint_id)
        .order_by(HealthCheck.checked_at.desc())
        .limit(limit)
    )
    health_checks = result.scalars().all()

    return health_checks


@router.get("/endpoint/{endpoint_id}/stats", response_model=HealthCheckStats)
async def get_health_check_stats(
    endpoint_id: int,
    period: str = "24h",
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get health check statistics for an endpoint"""

    # Verify endpoint ownership
    endpoint_result = await db.execute(
        select(Endpoint).where(
            Endpoint.id == endpoint_id,
            Endpoint.user_id == current_user.id
        )
    )
    endpoint = endpoint_result.scalar_one_or_none()

    if not endpoint:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Endpoint not found"
        )

    # Map period to hours
    period_map = {
        "24h": 24,
        "7d": 168,
        "30d": 720,
    }
    period_hours = period_map.get(period, 24)
    cutoff_time = datetime.utcnow() - timedelta(hours=period_hours)

    # Get health checks for period
    result = await db.execute(
        select(HealthCheck)
        .where(
            HealthCheck.endpoint_id == endpoint_id,
            HealthCheck.checked_at >= cutoff_time
        )
        .order_by(HealthCheck.checked_at.asc())
    )
    health_checks = result.scalars().all()

    if not health_checks:
        return HealthCheckStats(
            period=period,
            uptime_percentage=0,
            avg_response_time=0,
            min_response_time=0,
            max_response_time=0,
            total_checks=0,
            successful_checks=0,
            failed_checks=0,
            anomalies_detected=0
        )

    # Calculate stats
    total_checks = len(health_checks)
    successful_checks = sum(1 for hc in health_checks if hc.is_success)
    failed_checks = total_checks - successful_checks
    uptime_percentage = (successful_checks / total_checks) * 100

    response_times = [
        hc.response_time for hc in health_checks
        if hc.response_time is not None
    ]

    avg_response_time = sum(response_times) / len(response_times) if response_times else 0
    min_response_time = min(response_times) if response_times else 0
    max_response_time = max(response_times) if response_times else 0

    anomalies_detected = sum(1 for hc in health_checks if hc.is_anomaly)

    return HealthCheckStats(
        period=period,
        uptime_percentage=uptime_percentage,
        avg_response_time=avg_response_time,
        min_response_time=min_response_time,
        max_response_time=max_response_time,
        total_checks=total_checks,
        successful_checks=successful_checks,
        failed_checks=failed_checks,
        anomalies_detected=anomalies_detected
    )

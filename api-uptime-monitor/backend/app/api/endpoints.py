"""
Endpoint management API routes
"""

from fastapi import APIRouter, Depends, HTTPException, status, BackgroundTasks
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from typing import List

from app.core.database import get_db
from app.core.security import get_current_user
from app.models.user import User
from app.models.endpoint import Endpoint
from app.schemas.endpoint import (
    EndpointCreate,
    EndpointUpdate,
    EndpointResponse,
    EndpointStats
)
from app.services.monitoring import HealthCheckService
from app.tasks.monitoring import check_single_endpoint

router = APIRouter(prefix="/endpoints", tags=["Endpoints"])


@router.post("", response_model=EndpointResponse, status_code=status.HTTP_201_CREATED)
async def create_endpoint(
    endpoint_data: EndpointCreate,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Create a new endpoint to monitor"""

    endpoint = Endpoint(
        user_id=current_user.id,
        **endpoint_data.model_dump()
    )

    db.add(endpoint)
    await db.commit()
    await db.refresh(endpoint)

    return endpoint


@router.get("", response_model=List[EndpointResponse])
async def list_endpoints(
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """List all endpoints for the current user"""

    result = await db.execute(
        select(Endpoint)
        .where(Endpoint.user_id == current_user.id)
        .order_by(Endpoint.created_at.desc())
    )
    endpoints = result.scalars().all()

    return endpoints


@router.get("/{endpoint_id}", response_model=EndpointResponse)
async def get_endpoint(
    endpoint_id: int,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get a specific endpoint"""

    result = await db.execute(
        select(Endpoint).where(
            Endpoint.id == endpoint_id,
            Endpoint.user_id == current_user.id
        )
    )
    endpoint = result.scalar_one_or_none()

    if not endpoint:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Endpoint not found"
        )

    return endpoint


@router.put("/{endpoint_id}", response_model=EndpointResponse)
async def update_endpoint(
    endpoint_id: int,
    endpoint_data: EndpointUpdate,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Update an endpoint"""

    result = await db.execute(
        select(Endpoint).where(
            Endpoint.id == endpoint_id,
            Endpoint.user_id == current_user.id
        )
    )
    endpoint = result.scalar_one_or_none()

    if not endpoint:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Endpoint not found"
        )

    # Update fields
    update_data = endpoint_data.model_dump(exclude_unset=True)
    for field, value in update_data.items():
        setattr(endpoint, field, value)

    await db.commit()
    await db.refresh(endpoint)

    return endpoint


@router.delete("/{endpoint_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_endpoint(
    endpoint_id: int,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Delete an endpoint"""

    result = await db.execute(
        select(Endpoint).where(
            Endpoint.id == endpoint_id,
            Endpoint.user_id == current_user.id
        )
    )
    endpoint = result.scalar_one_or_none()

    if not endpoint:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Endpoint not found"
        )

    await db.delete(endpoint)
    await db.commit()


@router.post("/{endpoint_id}/check", status_code=status.HTTP_202_ACCEPTED)
async def trigger_check(
    endpoint_id: int,
    background_tasks: BackgroundTasks,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Trigger an immediate health check for an endpoint"""

    result = await db.execute(
        select(Endpoint).where(
            Endpoint.id == endpoint_id,
            Endpoint.user_id == current_user.id
        )
    )
    endpoint = result.scalar_one_or_none()

    if not endpoint:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Endpoint not found"
        )

    # Trigger async check
    check_single_endpoint.delay(endpoint_id)

    return {"message": "Health check initiated"}


@router.get("/{endpoint_id}/stats", response_model=EndpointStats)
async def get_endpoint_stats(
    endpoint_id: int,
    period: str = "24h",  # 24h, 7d, 30d
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get statistics for an endpoint"""

    result = await db.execute(
        select(Endpoint).where(
            Endpoint.id == endpoint_id,
            Endpoint.user_id == current_user.id
        )
    )
    endpoint = result.scalar_one_or_none()

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

    health_service = HealthCheckService()
    stats = await health_service.get_endpoint_stats(endpoint_id, period_hours, db)

    return {
        "endpoint_id": endpoint_id,
        **stats
    }

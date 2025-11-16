"""
Alert configuration API routes
"""

from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from typing import List

from app.core.database import get_db
from app.core.security import get_current_user
from app.models.user import User
from app.models.alert_config import AlertConfig
from app.schemas.alert import AlertConfigCreate, AlertConfigUpdate, AlertConfigResponse
from app.services.alerts import AlertService

router = APIRouter(prefix="/alerts", tags=["Alerts"])


@router.post("", response_model=AlertConfigResponse, status_code=status.HTTP_201_CREATED)
async def create_alert_config(
    alert_data: AlertConfigCreate,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Create a new alert configuration"""

    alert_config = AlertConfig(
        user_id=current_user.id,
        **alert_data.model_dump()
    )

    db.add(alert_config)
    await db.commit()
    await db.refresh(alert_config)

    return alert_config


@router.get("", response_model=List[AlertConfigResponse])
async def list_alert_configs(
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """List all alert configurations for the current user"""

    result = await db.execute(
        select(AlertConfig)
        .where(AlertConfig.user_id == current_user.id)
        .order_by(AlertConfig.created_at.desc())
    )
    alert_configs = result.scalars().all()

    return alert_configs


@router.get("/{alert_id}", response_model=AlertConfigResponse)
async def get_alert_config(
    alert_id: int,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Get a specific alert configuration"""

    result = await db.execute(
        select(AlertConfig).where(
            AlertConfig.id == alert_id,
            AlertConfig.user_id == current_user.id
        )
    )
    alert_config = result.scalar_one_or_none()

    if not alert_config:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Alert configuration not found"
        )

    return alert_config


@router.put("/{alert_id}", response_model=AlertConfigResponse)
async def update_alert_config(
    alert_id: int,
    alert_data: AlertConfigUpdate,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Update an alert configuration"""

    result = await db.execute(
        select(AlertConfig).where(
            AlertConfig.id == alert_id,
            AlertConfig.user_id == current_user.id
        )
    )
    alert_config = result.scalar_one_or_none()

    if not alert_config:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Alert configuration not found"
        )

    # Update fields
    update_data = alert_data.model_dump(exclude_unset=True)
    for field, value in update_data.items():
        setattr(alert_config, field, value)

    await db.commit()
    await db.refresh(alert_config)

    return alert_config


@router.delete("/{alert_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_alert_config(
    alert_id: int,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Delete an alert configuration"""

    result = await db.execute(
        select(AlertConfig).where(
            AlertConfig.id == alert_id,
            AlertConfig.user_id == current_user.id
        )
    )
    alert_config = result.scalar_one_or_none()

    if not alert_config:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Alert configuration not found"
        )

    await db.delete(alert_config)
    await db.commit()


@router.post("/{alert_id}/test", status_code=status.HTTP_200_OK)
async def test_alert_channel(
    alert_id: int,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db)
):
    """Test an alert channel"""

    result = await db.execute(
        select(AlertConfig).where(
            AlertConfig.id == alert_id,
            AlertConfig.user_id == current_user.id
        )
    )
    alert_config = result.scalar_one_or_none()

    if not alert_config:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Alert configuration not found"
        )

    alert_service = AlertService()
    success = await alert_service.test_alert_channel(
        alert_config.channel,
        alert_config
    )

    if success:
        return {"message": "Test alert sent successfully"}
    else:
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to send test alert"
        )

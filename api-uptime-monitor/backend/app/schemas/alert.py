from pydantic import BaseModel, EmailStr
from typing import Optional, List
from datetime import datetime
from app.models.alert_config import AlertChannel, AlertType


class AlertConfigBase(BaseModel):
    channel: AlertChannel
    alert_types: List[AlertType]
    is_active: bool = True
    email: Optional[EmailStr] = None
    telegram_chat_id: Optional[str] = None
    slack_webhook_url: Optional[str] = None
    cooldown_minutes: int = 5


class AlertConfigCreate(AlertConfigBase):
    pass


class AlertConfigUpdate(BaseModel):
    alert_types: Optional[List[AlertType]] = None
    is_active: Optional[bool] = None
    email: Optional[EmailStr] = None
    telegram_chat_id: Optional[str] = None
    slack_webhook_url: Optional[str] = None
    cooldown_minutes: Optional[int] = None


class AlertConfigResponse(AlertConfigBase):
    id: int
    user_id: int
    created_at: datetime

    class Config:
        from_attributes = True

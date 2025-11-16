from pydantic import BaseModel, HttpUrl, field_validator
from typing import Optional, Dict, List, Any
from datetime import datetime
from app.models.endpoint import HTTPMethod


class EndpointBase(BaseModel):
    name: str
    url: str
    method: HTTPMethod = HTTPMethod.GET
    headers: Optional[Dict[str, str]] = {}
    body: Optional[Dict[str, Any]] = None
    expected_status_codes: List[int] = [200]
    timeout: int = 30
    check_interval: int = 300  # 5 minutes
    is_active: bool = True
    verify_ssl: bool = True
    follow_redirects: bool = True
    response_time_threshold: int = 5000
    check_ssl_expiry: bool = True
    ssl_expiry_alert_days: int = 7

    @field_validator('check_interval')
    @classmethod
    def validate_interval(cls, v):
        valid_intervals = [60, 300, 900, 1800, 3600]
        if v not in valid_intervals:
            raise ValueError(f'Check interval must be one of {valid_intervals}')
        return v


class EndpointCreate(EndpointBase):
    pass


class EndpointUpdate(BaseModel):
    name: Optional[str] = None
    url: Optional[str] = None
    method: Optional[HTTPMethod] = None
    headers: Optional[Dict[str, str]] = None
    body: Optional[Dict[str, Any]] = None
    expected_status_codes: Optional[List[int]] = None
    timeout: Optional[int] = None
    check_interval: Optional[int] = None
    is_active: Optional[bool] = None
    verify_ssl: Optional[bool] = None
    follow_redirects: Optional[bool] = None
    response_time_threshold: Optional[int] = None
    check_ssl_expiry: Optional[bool] = None
    ssl_expiry_alert_days: Optional[int] = None


class EndpointResponse(EndpointBase):
    id: int
    user_id: int
    created_at: datetime
    updated_at: datetime
    last_checked_at: Optional[datetime] = None

    class Config:
        from_attributes = True


class EndpointStats(BaseModel):
    endpoint_id: int
    uptime_percentage: float
    avg_response_time: float
    total_checks: int
    failed_checks: int
    last_check: Optional[datetime]
    current_status: str  # "up", "down", "degraded"

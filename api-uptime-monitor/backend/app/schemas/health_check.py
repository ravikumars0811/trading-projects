from pydantic import BaseModel
from typing import Optional
from datetime import datetime


class HealthCheckResponse(BaseModel):
    id: int
    endpoint_id: int
    status_code: Optional[int]
    response_time: Optional[float]
    is_success: bool
    error_message: Optional[str]
    ssl_expiry_date: Optional[datetime]
    ssl_days_remaining: Optional[int]
    is_anomaly: bool
    anomaly_score: Optional[float]
    checked_at: datetime

    class Config:
        from_attributes = True


class HealthCheckStats(BaseModel):
    period: str  # "24h", "7d", "30d"
    uptime_percentage: float
    avg_response_time: float
    min_response_time: float
    max_response_time: float
    total_checks: int
    successful_checks: int
    failed_checks: int
    anomalies_detected: int

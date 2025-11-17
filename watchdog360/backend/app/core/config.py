"""
Application configuration
"""
from typing import List
from pydantic_settings import BaseSettings
from pydantic import AnyHttpUrl, validator


class Settings(BaseSettings):
    """Application settings"""

    # Project
    PROJECT_NAME: str = "Watchdog360"
    API_V1_PREFIX: str = "/api/v1"
    DEBUG: bool = False

    # Database
    DATABASE_URL: str
    TIMESCALEDB_ENABLED: bool = True

    # Redis
    REDIS_URL: str

    # Security
    SECRET_KEY: str
    ALGORITHM: str = "HS256"
    ACCESS_TOKEN_EXPIRE_MINUTES: int = 30

    # CORS
    ALLOWED_ORIGINS: List[str] = [
        "http://localhost:3000",
        "https://dashboard.watchdog360.com"
    ]

    @validator("ALLOWED_ORIGINS", pre=True)
    def parse_cors_origins(cls, v):
        if isinstance(v, str):
            return [origin.strip() for origin in v.split(",")]
        return v

    # AI/ML
    ANOMALY_DETECTION_ENABLED: bool = True
    ANOMALY_DETECTION_THRESHOLD: float = 0.85
    PREDICTION_ENABLED: bool = True

    # Alerts
    ALERT_HIGH_CPU_THRESHOLD: float = 80.0
    ALERT_HIGH_MEMORY_THRESHOLD: float = 85.0
    ALERT_HIGH_DISK_THRESHOLD: float = 90.0

    # Metrics retention (days)
    METRICS_RETENTION_DAYS: int = 30

    class Config:
        env_file = ".env"
        case_sensitive = True


settings = Settings()

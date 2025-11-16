from pydantic_settings import BaseSettings
from typing import Optional


class Settings(BaseSettings):
    # App
    APP_NAME: str = "API Uptime Monitor"
    ENVIRONMENT: str = "development"
    DEBUG: bool = True
    API_V1_STR: str = "/api/v1"

    # Database
    DATABASE_URL: str

    # Redis
    REDIS_URL: str

    # Security
    SECRET_KEY: str
    ALGORITHM: str = "HS256"
    ACCESS_TOKEN_EXPIRE_MINUTES: int = 30

    # Email
    SENDGRID_API_KEY: Optional[str] = None
    FROM_EMAIL: Optional[str] = None

    # Telegram
    TELEGRAM_BOT_TOKEN: Optional[str] = None

    # Slack
    SLACK_WEBHOOK_URL: Optional[str] = None

    # Monitoring
    CHECK_INTERVAL_SECONDS: int = 60
    MAX_WORKERS: int = 4

    class Config:
        env_file = ".env"
        case_sensitive = True


settings = Settings()

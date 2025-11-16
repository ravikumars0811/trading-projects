from sqlalchemy import Column, Integer, String, Boolean, DateTime, ForeignKey, JSON, Enum
from sqlalchemy.orm import relationship
from datetime import datetime
import enum
from app.core.database import Base


class AlertChannel(str, enum.Enum):
    EMAIL = "email"
    TELEGRAM = "telegram"
    SLACK = "slack"


class AlertType(str, enum.Enum):
    DOWNTIME = "downtime"
    SLOW_RESPONSE = "slow_response"
    SSL_EXPIRY = "ssl_expiry"
    ANOMALY = "anomaly"


class AlertConfig(Base):
    __tablename__ = "alert_configs"

    id = Column(Integer, primary_key=True, index=True)
    user_id = Column(Integer, ForeignKey("users.id"), nullable=False)

    # Alert settings
    channel = Column(Enum(AlertChannel), nullable=False)
    alert_types = Column(JSON, default=[])  # List of AlertType values
    is_active = Column(Boolean, default=True)

    # Channel-specific config
    email = Column(String, nullable=True)  # For email alerts
    telegram_chat_id = Column(String, nullable=True)  # For Telegram
    slack_webhook_url = Column(String, nullable=True)  # For Slack

    # Alert preferences
    cooldown_minutes = Column(Integer, default=5)  # Prevent alert spam

    # Metadata
    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)

    # Relationships
    user = relationship("User", back_populates="alert_configs")

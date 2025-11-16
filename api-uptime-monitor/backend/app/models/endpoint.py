from sqlalchemy import Column, Integer, String, JSON, Boolean, DateTime, ForeignKey, Enum
from sqlalchemy.orm import relationship
from datetime import datetime
import enum
from app.core.database import Base


class HTTPMethod(str, enum.Enum):
    GET = "GET"
    POST = "POST"
    PUT = "PUT"
    DELETE = "DELETE"
    PATCH = "PATCH"
    HEAD = "HEAD"
    OPTIONS = "OPTIONS"


class CheckInterval(int, enum.Enum):
    ONE_MINUTE = 60
    FIVE_MINUTES = 300
    FIFTEEN_MINUTES = 900
    THIRTY_MINUTES = 1800
    ONE_HOUR = 3600


class Endpoint(Base):
    __tablename__ = "endpoints"

    id = Column(Integer, primary_key=True, index=True)
    user_id = Column(Integer, ForeignKey("users.id"), nullable=False)
    name = Column(String, nullable=False)
    url = Column(String, nullable=False)
    method = Column(Enum(HTTPMethod), default=HTTPMethod.GET)
    headers = Column(JSON, default=dict)  # Auth headers, custom headers
    body = Column(JSON, nullable=True)  # For POST/PUT requests
    expected_status_codes = Column(JSON, default=[200])  # List of acceptable status codes
    timeout = Column(Integer, default=30)  # Request timeout in seconds
    check_interval = Column(Integer, default=CheckInterval.FIVE_MINUTES.value)  # In seconds
    is_active = Column(Boolean, default=True)
    verify_ssl = Column(Boolean, default=True)
    follow_redirects = Column(Boolean, default=True)

    # Thresholds
    response_time_threshold = Column(Integer, default=5000)  # milliseconds

    # SSL monitoring
    check_ssl_expiry = Column(Boolean, default=True)
    ssl_expiry_alert_days = Column(Integer, default=7)  # Alert N days before expiry

    # Metadata
    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    last_checked_at = Column(DateTime, nullable=True)

    # Relationships
    owner = relationship("User", back_populates="endpoints")
    health_checks = relationship("HealthCheck", back_populates="endpoint", cascade="all, delete-orphan")
    incidents = relationship("Incident", back_populates="endpoint", cascade="all, delete-orphan")

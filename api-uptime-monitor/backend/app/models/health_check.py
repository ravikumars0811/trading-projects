from sqlalchemy import Column, Integer, String, Boolean, DateTime, ForeignKey, Float, Text, Index
from sqlalchemy.orm import relationship
from datetime import datetime
from app.core.database import Base


class HealthCheck(Base):
    __tablename__ = "health_checks"

    id = Column(Integer, primary_key=True, index=True)
    endpoint_id = Column(Integer, ForeignKey("endpoints.id"), nullable=False, index=True)

    # Check results
    status_code = Column(Integer, nullable=True)
    response_time = Column(Float, nullable=True)  # in milliseconds
    is_success = Column(Boolean, default=False, index=True)
    error_message = Column(Text, nullable=True)

    # SSL info
    ssl_expiry_date = Column(DateTime, nullable=True)
    ssl_days_remaining = Column(Integer, nullable=True)

    # Anomaly detection
    is_anomaly = Column(Boolean, default=False)  # Detected by ML
    anomaly_score = Column(Float, nullable=True)  # Anomaly score from ML model

    # Metadata
    checked_at = Column(DateTime, default=datetime.utcnow, index=True)

    # Relationships
    endpoint = relationship("Endpoint", back_populates="health_checks")

    # Create composite index for time-series queries
    __table_args__ = (
        Index('idx_endpoint_checked_at', 'endpoint_id', 'checked_at'),
    )

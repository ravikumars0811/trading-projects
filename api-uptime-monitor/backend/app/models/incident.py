from sqlalchemy import Column, Integer, String, Boolean, DateTime, ForeignKey, Text, Enum
from sqlalchemy.orm import relationship
from datetime import datetime
from app.core.database import Base
from app.models.alert_config import AlertType


class IncidentStatus(str, enum.Enum):
    OPEN = "open"
    ACKNOWLEDGED = "acknowledged"
    RESOLVED = "resolved"


import enum


class Incident(Base):
    __tablename__ = "incidents"

    id = Column(Integer, primary_key=True, index=True)
    endpoint_id = Column(Integer, ForeignKey("endpoints.id"), nullable=False)

    # Incident details
    incident_type = Column(Enum(AlertType), nullable=False)
    status = Column(Enum(IncidentStatus), default=IncidentStatus.OPEN)
    title = Column(String, nullable=False)
    description = Column(Text, nullable=True)

    # Timing
    started_at = Column(DateTime, default=datetime.utcnow, index=True)
    resolved_at = Column(DateTime, nullable=True)
    acknowledged_at = Column(DateTime, nullable=True)

    # Metadata
    alert_sent = Column(Boolean, default=False)
    alert_sent_at = Column(DateTime, nullable=True)

    # Relationships
    endpoint = relationship("Endpoint", back_populates="incidents")

"""
Transaction Service Database Models
"""
from sqlalchemy import Column, String, Numeric, DateTime, Enum, JSON
from sqlalchemy.sql import func
from decimal import Decimal
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
from shared.database import Base
from shared.models import (
    TransactionType,
    TransactionStatus,
    PaymentMethod,
    Currency
)


class Transaction(Base):
    """Transaction database model"""
    __tablename__ = "transactions"

    id = Column(String, primary_key=True, index=True)
    reference_id = Column(String, unique=True, index=True, nullable=False)
    transaction_type = Column(Enum(TransactionType), nullable=False)
    status = Column(Enum(TransactionStatus), default=TransactionStatus.PENDING, nullable=False)
    amount = Column(Numeric(precision=15, scale=2), nullable=False)
    currency = Column(Enum(Currency), default=Currency.USD, nullable=False)
    from_wallet_id = Column(String, nullable=True)
    to_wallet_id = Column(String, nullable=True)
    payment_method = Column(Enum(PaymentMethod), nullable=True)
    description = Column(String, nullable=True)
    metadata = Column(JSON, nullable=True)
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now(), server_default=func.now())
    completed_at = Column(DateTime(timezone=True), nullable=True)

    def to_dict(self):
        """Convert model to dictionary"""
        return {
            "id": self.id,
            "reference_id": self.reference_id,
            "transaction_type": self.transaction_type.value,
            "status": self.status.value,
            "amount": float(self.amount),
            "currency": self.currency.value,
            "from_wallet_id": self.from_wallet_id,
            "to_wallet_id": self.to_wallet_id,
            "payment_method": self.payment_method.value if self.payment_method else None,
            "description": self.description,
            "metadata": self.metadata,
            "created_at": self.created_at.isoformat() if self.created_at else None,
            "updated_at": self.updated_at.isoformat() if self.updated_at else None,
            "completed_at": self.completed_at.isoformat() if self.completed_at else None
        }

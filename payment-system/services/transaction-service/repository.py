"""
Transaction Service Repository Layer
"""
from sqlalchemy.orm import Session
from sqlalchemy import and_, or_, desc
from typing import Optional, List
from datetime import datetime
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
from models import Transaction
from shared.models import TransactionType, TransactionStatus, TransactionCreate
from shared.utils import generate_uuid, generate_reference_id


class TransactionRepository:
    """Repository for transaction database operations"""

    def __init__(self, db: Session):
        self.db = db

    def create_transaction(self, transaction_data: TransactionCreate) -> Transaction:
        """Create a new transaction"""
        transaction = Transaction(
            id=generate_uuid(),
            reference_id=generate_reference_id("TXN"),
            transaction_type=transaction_data.transaction_type,
            status=TransactionStatus.PENDING,
            amount=transaction_data.amount,
            currency=transaction_data.currency,
            from_wallet_id=transaction_data.from_wallet_id,
            to_wallet_id=transaction_data.to_wallet_id,
            payment_method=transaction_data.payment_method,
            description=transaction_data.description,
            metadata=transaction_data.metadata
        )
        self.db.add(transaction)
        self.db.commit()
        self.db.refresh(transaction)
        return transaction

    def get_transaction_by_id(self, transaction_id: str) -> Optional[Transaction]:
        """Get transaction by ID"""
        return self.db.query(Transaction).filter(Transaction.id == transaction_id).first()

    def get_transaction_by_reference(self, reference_id: str) -> Optional[Transaction]:
        """Get transaction by reference ID"""
        return self.db.query(Transaction).filter(
            Transaction.reference_id == reference_id
        ).first()

    def get_transactions_by_wallet(
        self,
        wallet_id: str,
        skip: int = 0,
        limit: int = 100
    ) -> List[Transaction]:
        """Get transactions for a wallet"""
        return self.db.query(Transaction).filter(
            or_(
                Transaction.from_wallet_id == wallet_id,
                Transaction.to_wallet_id == wallet_id
            )
        ).order_by(desc(Transaction.created_at)).offset(skip).limit(limit).all()

    def get_transactions_by_status(
        self,
        status: TransactionStatus,
        skip: int = 0,
        limit: int = 100
    ) -> List[Transaction]:
        """Get transactions by status"""
        return self.db.query(Transaction).filter(
            Transaction.status == status
        ).order_by(desc(Transaction.created_at)).offset(skip).limit(limit).all()

    def update_transaction_status(
        self,
        transaction_id: str,
        status: TransactionStatus
    ) -> Optional[Transaction]:
        """Update transaction status"""
        transaction = self.get_transaction_by_id(transaction_id)
        if not transaction:
            return None

        transaction.status = status
        if status == TransactionStatus.COMPLETED:
            transaction.completed_at = datetime.utcnow()

        self.db.commit()
        self.db.refresh(transaction)
        return transaction

    def get_wallet_transaction_history(
        self,
        wallet_id: str,
        transaction_type: Optional[TransactionType] = None,
        status: Optional[TransactionStatus] = None,
        skip: int = 0,
        limit: int = 100
    ) -> List[Transaction]:
        """Get filtered transaction history for a wallet"""
        query = self.db.query(Transaction).filter(
            or_(
                Transaction.from_wallet_id == wallet_id,
                Transaction.to_wallet_id == wallet_id
            )
        )

        if transaction_type:
            query = query.filter(Transaction.transaction_type == transaction_type)

        if status:
            query = query.filter(Transaction.status == status)

        return query.order_by(desc(Transaction.created_at)).offset(skip).limit(limit).all()

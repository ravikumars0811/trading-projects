"""
Wallet Service Repository Layer
"""
from sqlalchemy.orm import Session
from sqlalchemy import and_
from typing import Optional, List
from decimal import Decimal
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
from models import Wallet
from shared.models import Currency
from shared.utils import generate_uuid


class WalletRepository:
    """Repository for wallet database operations"""

    def __init__(self, db: Session):
        self.db = db

    def create_wallet(self, user_id: str, currency: Currency = Currency.USD) -> Wallet:
        """Create a new wallet for a user"""
        wallet = Wallet(
            id=generate_uuid(),
            user_id=user_id,
            balance=Decimal("0.00"),
            currency=currency,
            is_active=True
        )
        self.db.add(wallet)
        self.db.commit()
        self.db.refresh(wallet)
        return wallet

    def get_wallet_by_id(self, wallet_id: str) -> Optional[Wallet]:
        """Get wallet by ID"""
        return self.db.query(Wallet).filter(Wallet.id == wallet_id).first()

    def get_wallet_by_user_id(
        self,
        user_id: str,
        currency: Currency = Currency.USD
    ) -> Optional[Wallet]:
        """Get wallet by user ID and currency"""
        return self.db.query(Wallet).filter(
            and_(
                Wallet.user_id == user_id,
                Wallet.currency == currency
            )
        ).first()

    def get_user_wallets(self, user_id: str) -> List[Wallet]:
        """Get all wallets for a user"""
        return self.db.query(Wallet).filter(Wallet.user_id == user_id).all()

    def update_balance(
        self,
        wallet_id: str,
        amount: Decimal,
        operation: str = "add"
    ) -> Optional[Wallet]:
        """Update wallet balance"""
        wallet = self.get_wallet_by_id(wallet_id)
        if not wallet:
            return None

        if operation == "add":
            wallet.balance += amount
        elif operation == "subtract":
            wallet.balance -= amount
        else:
            raise ValueError(f"Invalid operation: {operation}")

        self.db.commit()
        self.db.refresh(wallet)
        return wallet

    def deactivate_wallet(self, wallet_id: str) -> bool:
        """Deactivate wallet"""
        wallet = self.get_wallet_by_id(wallet_id)
        if not wallet:
            return False

        wallet.is_active = False
        self.db.commit()
        return True

    def activate_wallet(self, wallet_id: str) -> bool:
        """Activate wallet"""
        wallet = self.get_wallet_by_id(wallet_id)
        if not wallet:
            return False

        wallet.is_active = True
        self.db.commit()
        return True

    def get_balance(self, wallet_id: str) -> Optional[Decimal]:
        """Get wallet balance"""
        wallet = self.get_wallet_by_id(wallet_id)
        return wallet.balance if wallet else None

    def wallet_exists(self, user_id: str, currency: Currency) -> bool:
        """Check if wallet exists for user and currency"""
        return self.db.query(Wallet).filter(
            and_(
                Wallet.user_id == user_id,
                Wallet.currency == currency
            )
        ).first() is not None

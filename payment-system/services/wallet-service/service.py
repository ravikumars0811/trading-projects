"""
Wallet Service Business Logic
"""
from typing import List, Dict, Any
from decimal import Decimal
from fastapi import HTTPException, status
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
from repository import WalletRepository
from shared.models import Currency, WalletResponse
from shared.utils import (
    RedisClient,
    WalletNotFoundException,
    InsufficientFundsException
)
from shared.kafka_client import KafkaProducerClient, KafkaTopics


class WalletService:
    """Wallet service business logic"""

    def __init__(self, repository: WalletRepository):
        self.repository = repository
        self.redis = RedisClient()
        self.kafka_producer = KafkaProducerClient()

    def create_wallet(self, user_id: str, currency: Currency = Currency.USD) -> WalletResponse:
        """Create a new wallet for a user"""
        # Check if wallet already exists
        if self.repository.wallet_exists(user_id, currency):
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Wallet already exists for user with currency {currency.value}"
            )

        # Create wallet
        wallet = self.repository.create_wallet(user_id, currency)

        # Send wallet creation event to Kafka
        self.kafka_producer.send_event(
            topic=KafkaTopics.USER_EVENTS,
            event_type="wallet.created",
            data={
                "wallet_id": wallet.id,
                "user_id": wallet.user_id,
                "currency": wallet.currency.value
            }
        )

        return WalletResponse(**wallet.to_dict())

    def get_wallet(self, wallet_id: str) -> WalletResponse:
        """Get wallet by ID"""
        # Try to get from cache first
        cached_wallet = self.redis.get(f"wallet:{wallet_id}")
        if cached_wallet:
            return WalletResponse(**cached_wallet)

        wallet = self.repository.get_wallet_by_id(wallet_id)
        if not wallet:
            raise WalletNotFoundException()

        # Cache wallet data
        wallet_dict = wallet.to_dict()
        self.redis.set(f"wallet:{wallet_id}", wallet_dict, expiration=300)

        return WalletResponse(**wallet_dict)

    def get_user_wallets(self, user_id: str) -> List[WalletResponse]:
        """Get all wallets for a user"""
        wallets = self.repository.get_user_wallets(user_id)
        return [WalletResponse(**wallet.to_dict()) for wallet in wallets]

    def get_wallet_by_user(self, user_id: str, currency: Currency = Currency.USD) -> WalletResponse:
        """Get wallet by user ID and currency"""
        wallet = self.repository.get_wallet_by_user_id(user_id, currency)
        if not wallet:
            raise WalletNotFoundException()

        return WalletResponse(**wallet.to_dict())

    def add_funds(self, wallet_id: str, amount: Decimal) -> WalletResponse:
        """Add funds to wallet"""
        if amount <= 0:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Amount must be greater than zero"
            )

        wallet = self.repository.update_balance(wallet_id, amount, "add")
        if not wallet:
            raise WalletNotFoundException()

        # Invalidate cache
        self.redis.delete(f"wallet:{wallet_id}")

        # Send event to Kafka
        self.kafka_producer.send_event(
            topic=KafkaTopics.TRANSACTION_EVENTS,
            event_type="wallet.funds_added",
            data={
                "wallet_id": wallet.id,
                "amount": float(amount),
                "new_balance": float(wallet.balance)
            }
        )

        return WalletResponse(**wallet.to_dict())

    def deduct_funds(self, wallet_id: str, amount: Decimal) -> WalletResponse:
        """Deduct funds from wallet"""
        if amount <= 0:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Amount must be greater than zero"
            )

        # Check if sufficient balance
        current_balance = self.repository.get_balance(wallet_id)
        if current_balance is None:
            raise WalletNotFoundException()

        if current_balance < amount:
            raise InsufficientFundsException(
                f"Insufficient funds. Available: {current_balance}, Required: {amount}"
            )

        wallet = self.repository.update_balance(wallet_id, amount, "subtract")

        # Invalidate cache
        self.redis.delete(f"wallet:{wallet_id}")

        # Send event to Kafka
        self.kafka_producer.send_event(
            topic=KafkaTopics.TRANSACTION_EVENTS,
            event_type="wallet.funds_deducted",
            data={
                "wallet_id": wallet.id,
                "amount": float(amount),
                "new_balance": float(wallet.balance)
            }
        )

        return WalletResponse(**wallet.to_dict())

    def get_balance(self, wallet_id: str) -> Dict[str, Any]:
        """Get wallet balance"""
        balance = self.repository.get_balance(wallet_id)
        if balance is None:
            raise WalletNotFoundException()

        wallet = self.repository.get_wallet_by_id(wallet_id)

        return {
            "wallet_id": wallet_id,
            "balance": float(balance),
            "currency": wallet.currency.value
        }

    def deactivate_wallet(self, wallet_id: str) -> bool:
        """Deactivate wallet"""
        result = self.repository.deactivate_wallet(wallet_id)
        if not result:
            raise WalletNotFoundException()

        # Invalidate cache
        self.redis.delete(f"wallet:{wallet_id}")

        # Send event to Kafka
        self.kafka_producer.send_event(
            topic=KafkaTopics.USER_EVENTS,
            event_type="wallet.deactivated",
            data={"wallet_id": wallet_id}
        )

        return True

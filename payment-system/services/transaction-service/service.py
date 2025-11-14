"""
Transaction Service Business Logic
"""
from typing import List, Optional
from decimal import Decimal
from fastapi import HTTPException, status
from datetime import datetime
import sys
import os
import requests

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
from repository import TransactionRepository
from shared.models import (
    TransactionCreate,
    TransactionResponse,
    TransactionType,
    TransactionStatus
)
from shared.utils import (
    RedisClient,
    TransactionFailedException
)
from shared.kafka_client import KafkaProducerClient, KafkaTopics
from shared.database import get_mongodb


class TransactionService:
    """Transaction service business logic"""

    def __init__(self, repository: TransactionRepository):
        self.repository = repository
        self.redis = RedisClient()
        self.kafka_producer = KafkaProducerClient()
        self.mongodb = get_mongodb()
        self.wallet_service_url = os.getenv("WALLET_SERVICE_URL", "http://wallet-service:8002")

    def create_transaction(self, transaction_data: TransactionCreate) -> TransactionResponse:
        """Create and process a new transaction"""
        # Create transaction
        transaction = self.repository.create_transaction(transaction_data)

        # Log to MongoDB
        self._log_to_mongo(transaction.to_dict(), "transaction_created")

        # Send event to Kafka
        self.kafka_producer.send_event(
            topic=KafkaTopics.TRANSACTION_EVENTS,
            event_type="transaction.created",
            data=transaction.to_dict()
        )

        # Process transaction based on type
        try:
            if transaction_data.transaction_type == TransactionType.TRANSFER:
                self._process_transfer(transaction)
            elif transaction_data.transaction_type == TransactionType.DEPOSIT:
                self._process_deposit(transaction)
            elif transaction_data.transaction_type == TransactionType.WITHDRAWAL:
                self._process_withdrawal(transaction)
            elif transaction_data.transaction_type == TransactionType.PAYMENT:
                self._process_payment(transaction)

            # Update status to completed
            transaction = self.repository.update_transaction_status(
                transaction.id,
                TransactionStatus.COMPLETED
            )

            # Send completion event
            self.kafka_producer.send_event(
                topic=KafkaTopics.TRANSACTION_EVENTS,
                event_type="transaction.completed",
                data=transaction.to_dict()
            )

            # Log completion
            self._log_to_mongo(transaction.to_dict(), "transaction_completed")

        except Exception as e:
            # Mark transaction as failed
            transaction = self.repository.update_transaction_status(
                transaction.id,
                TransactionStatus.FAILED
            )

            # Send failure event
            self.kafka_producer.send_event(
                topic=KafkaTopics.TRANSACTION_EVENTS,
                event_type="transaction.failed",
                data={
                    **transaction.to_dict(),
                    "error": str(e)
                }
            )

            # Log failure
            self._log_to_mongo(
                {**transaction.to_dict(), "error": str(e)},
                "transaction_failed"
            )

            raise TransactionFailedException(f"Transaction failed: {str(e)}")

        return TransactionResponse(**transaction.to_dict())

    def _process_transfer(self, transaction):
        """Process wallet-to-wallet transfer"""
        # Deduct from source wallet
        self._update_wallet_balance(
            transaction.from_wallet_id,
            transaction.amount,
            "deduct"
        )

        # Add to destination wallet
        self._update_wallet_balance(
            transaction.to_wallet_id,
            transaction.amount,
            "add"
        )

    def _process_deposit(self, transaction):
        """Process deposit to wallet"""
        self._update_wallet_balance(
            transaction.to_wallet_id,
            transaction.amount,
            "add"
        )

    def _process_withdrawal(self, transaction):
        """Process withdrawal from wallet"""
        self._update_wallet_balance(
            transaction.from_wallet_id,
            transaction.amount,
            "deduct"
        )

    def _process_payment(self, transaction):
        """Process payment transaction"""
        self._update_wallet_balance(
            transaction.from_wallet_id,
            transaction.amount,
            "deduct"
        )

    def _update_wallet_balance(self, wallet_id: str, amount: Decimal, operation: str):
        """Update wallet balance via wallet service"""
        try:
            endpoint = f"{self.wallet_service_url}/api/v1/wallets/{wallet_id}"

            if operation == "add":
                endpoint += "/add-funds"
            elif operation == "deduct":
                endpoint += "/deduct-funds"
            else:
                raise ValueError(f"Invalid operation: {operation}")

            response = requests.post(
                endpoint,
                json={"amount": float(amount)},
                timeout=10
            )

            if response.status_code != 200:
                raise Exception(f"Wallet update failed: {response.text}")

            return response.json()

        except requests.exceptions.RequestException as e:
            raise Exception(f"Error communicating with wallet service: {str(e)}")

    def get_transaction(self, transaction_id: str) -> TransactionResponse:
        """Get transaction by ID"""
        # Try cache first
        cached = self.redis.get(f"transaction:{transaction_id}")
        if cached:
            return TransactionResponse(**cached)

        transaction = self.repository.get_transaction_by_id(transaction_id)
        if not transaction:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="Transaction not found"
            )

        # Cache transaction
        tx_dict = transaction.to_dict()
        self.redis.set(f"transaction:{transaction_id}", tx_dict, expiration=600)

        return TransactionResponse(**tx_dict)

    def get_transaction_by_reference(self, reference_id: str) -> TransactionResponse:
        """Get transaction by reference ID"""
        transaction = self.repository.get_transaction_by_reference(reference_id)
        if not transaction:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="Transaction not found"
            )

        return TransactionResponse(**transaction.to_dict())

    def get_wallet_transactions(
        self,
        wallet_id: str,
        skip: int = 0,
        limit: int = 100
    ) -> List[TransactionResponse]:
        """Get transactions for a wallet"""
        transactions = self.repository.get_transactions_by_wallet(wallet_id, skip, limit)
        return [TransactionResponse(**tx.to_dict()) for tx in transactions]

    def get_wallet_transaction_history(
        self,
        wallet_id: str,
        transaction_type: Optional[TransactionType] = None,
        status: Optional[TransactionStatus] = None,
        skip: int = 0,
        limit: int = 100
    ) -> List[TransactionResponse]:
        """Get filtered transaction history"""
        transactions = self.repository.get_wallet_transaction_history(
            wallet_id,
            transaction_type,
            status,
            skip,
            limit
        )
        return [TransactionResponse(**tx.to_dict()) for tx in transactions]

    def _log_to_mongo(self, data: dict, event_type: str):
        """Log transaction event to MongoDB"""
        try:
            collection = self.mongodb.get_collection("transaction_logs")
            log_entry = {
                "event_type": event_type,
                "timestamp": datetime.utcnow(),
                "data": data
            }
            collection.insert_one(log_entry)
        except Exception as e:
            # Log error but don't fail transaction
            print(f"Error logging to MongoDB: {e}")

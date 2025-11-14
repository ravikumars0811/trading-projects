"""
Transaction Service API Routes
"""
from fastapi import APIRouter, Depends, Query
from sqlalchemy.orm import Session
from typing import List, Optional
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
from repository import TransactionRepository
from service import TransactionService
from shared.database import get_db
from shared.models import (
    TransactionCreate,
    TransactionResponse,
    TransactionType,
    TransactionStatus
)

router = APIRouter(prefix="/api/v1/transactions", tags=["transactions"])


def get_transaction_service(db: Session = Depends(get_db)) -> TransactionService:
    """Dependency to get transaction service"""
    repository = TransactionRepository(db)
    return TransactionService(repository)


@router.post("/create", response_model=TransactionResponse, status_code=201)
async def create_transaction(
    transaction_data: TransactionCreate,
    service: TransactionService = Depends(get_transaction_service)
):
    """
    Create and process a new transaction

    - **transaction_type**: Type of transaction (deposit/withdrawal/transfer/payment)
    - **amount**: Transaction amount
    - **currency**: Currency code
    - **from_wallet_id**: Source wallet ID (required for withdrawal/transfer/payment)
    - **to_wallet_id**: Destination wallet ID (required for deposit/transfer)
    - **payment_method**: Payment method (optional)
    - **description**: Transaction description (optional)
    """
    return service.create_transaction(transaction_data)


@router.get("/{transaction_id}", response_model=TransactionResponse)
async def get_transaction(
    transaction_id: str,
    service: TransactionService = Depends(get_transaction_service)
):
    """
    Get transaction by ID

    - **transaction_id**: Transaction's unique identifier
    """
    return service.get_transaction(transaction_id)


@router.get("/reference/{reference_id}", response_model=TransactionResponse)
async def get_transaction_by_reference(
    reference_id: str,
    service: TransactionService = Depends(get_transaction_service)
):
    """
    Get transaction by reference ID

    - **reference_id**: Transaction reference number
    """
    return service.get_transaction_by_reference(reference_id)


@router.get("/wallet/{wallet_id}/transactions", response_model=List[TransactionResponse])
async def get_wallet_transactions(
    wallet_id: str,
    skip: int = Query(0, ge=0),
    limit: int = Query(100, ge=1, le=1000),
    service: TransactionService = Depends(get_transaction_service)
):
    """
    Get all transactions for a wallet

    - **wallet_id**: Wallet's unique identifier
    - **skip**: Number of records to skip (pagination)
    - **limit**: Maximum number of records to return
    """
    return service.get_wallet_transactions(wallet_id, skip, limit)


@router.get("/wallet/{wallet_id}/history", response_model=List[TransactionResponse])
async def get_wallet_transaction_history(
    wallet_id: str,
    transaction_type: Optional[TransactionType] = None,
    status: Optional[TransactionStatus] = None,
    skip: int = Query(0, ge=0),
    limit: int = Query(100, ge=1, le=1000),
    service: TransactionService = Depends(get_transaction_service)
):
    """
    Get filtered transaction history for a wallet

    - **wallet_id**: Wallet's unique identifier
    - **transaction_type**: Filter by transaction type (optional)
    - **status**: Filter by transaction status (optional)
    - **skip**: Number of records to skip (pagination)
    - **limit**: Maximum number of records to return
    """
    return service.get_wallet_transaction_history(
        wallet_id,
        transaction_type,
        status,
        skip,
        limit
    )


@router.get("/health")
async def health_check():
    """Health check endpoint"""
    return {"status": "healthy", "service": "transaction-service"}

"""
Wallet Service API Routes
"""
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from typing import List
from decimal import Decimal
from pydantic import BaseModel
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
from repository import WalletRepository
from service import WalletService
from shared.database import get_db
from shared.models import Currency, WalletResponse, SuccessResponse

router = APIRouter(prefix="/api/v1/wallets", tags=["wallets"])


def get_wallet_service(db: Session = Depends(get_db)) -> WalletService:
    """Dependency to get wallet service"""
    repository = WalletRepository(db)
    return WalletService(repository)


class WalletCreate(BaseModel):
    user_id: str
    currency: Currency = Currency.USD


class FundsOperation(BaseModel):
    amount: Decimal


@router.post("/create", response_model=WalletResponse, status_code=status.HTTP_201_CREATED)
async def create_wallet(
    wallet_data: WalletCreate,
    service: WalletService = Depends(get_wallet_service)
):
    """
    Create a new wallet for a user

    - **user_id**: User's unique identifier
    - **currency**: Wallet currency (USD/EUR/GBP/INR)
    """
    return service.create_wallet(wallet_data.user_id, wallet_data.currency)


@router.get("/{wallet_id}", response_model=WalletResponse)
async def get_wallet(
    wallet_id: str,
    service: WalletService = Depends(get_wallet_service)
):
    """
    Get wallet by ID

    - **wallet_id**: Wallet's unique identifier
    """
    return service.get_wallet(wallet_id)


@router.get("/user/{user_id}", response_model=List[WalletResponse])
async def get_user_wallets(
    user_id: str,
    service: WalletService = Depends(get_wallet_service)
):
    """
    Get all wallets for a user

    - **user_id**: User's unique identifier
    """
    return service.get_user_wallets(user_id)


@router.get("/user/{user_id}/currency/{currency}", response_model=WalletResponse)
async def get_wallet_by_user_and_currency(
    user_id: str,
    currency: Currency,
    service: WalletService = Depends(get_wallet_service)
):
    """
    Get wallet by user ID and currency

    - **user_id**: User's unique identifier
    - **currency**: Wallet currency
    """
    return service.get_wallet_by_user(user_id, currency)


@router.post("/{wallet_id}/add-funds", response_model=WalletResponse)
async def add_funds(
    wallet_id: str,
    funds: FundsOperation,
    service: WalletService = Depends(get_wallet_service)
):
    """
    Add funds to wallet

    - **wallet_id**: Wallet's unique identifier
    - **amount**: Amount to add
    """
    return service.add_funds(wallet_id, funds.amount)


@router.post("/{wallet_id}/deduct-funds", response_model=WalletResponse)
async def deduct_funds(
    wallet_id: str,
    funds: FundsOperation,
    service: WalletService = Depends(get_wallet_service)
):
    """
    Deduct funds from wallet

    - **wallet_id**: Wallet's unique identifier
    - **amount**: Amount to deduct
    """
    return service.deduct_funds(wallet_id, funds.amount)


@router.get("/{wallet_id}/balance")
async def get_balance(
    wallet_id: str,
    service: WalletService = Depends(get_wallet_service)
):
    """
    Get wallet balance

    - **wallet_id**: Wallet's unique identifier
    """
    return service.get_balance(wallet_id)


@router.post("/{wallet_id}/deactivate", response_model=SuccessResponse)
async def deactivate_wallet(
    wallet_id: str,
    service: WalletService = Depends(get_wallet_service)
):
    """
    Deactivate wallet

    - **wallet_id**: Wallet's unique identifier
    """
    result = service.deactivate_wallet(wallet_id)
    return SuccessResponse(
        message="Wallet deactivated successfully",
        data={"wallet_id": wallet_id}
    )


@router.get("/health", status_code=status.HTTP_200_OK)
async def health_check():
    """
    Health check endpoint
    """
    return {"status": "healthy", "service": "wallet-service"}

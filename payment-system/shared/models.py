"""
Shared data models used across all microservices
"""
from pydantic import BaseModel, EmailStr, Field, validator
from typing import Optional, List
from datetime import datetime
from enum import Enum
from decimal import Decimal


class UserRole(str, Enum):
    USER = "user"
    MERCHANT = "merchant"
    ADMIN = "admin"


class TransactionType(str, Enum):
    DEPOSIT = "deposit"
    WITHDRAWAL = "withdrawal"
    TRANSFER = "transfer"
    PAYMENT = "payment"
    REFUND = "refund"


class TransactionStatus(str, Enum):
    PENDING = "pending"
    PROCESSING = "processing"
    COMPLETED = "completed"
    FAILED = "failed"
    CANCELLED = "cancelled"
    REFUNDED = "refunded"


class PaymentMethod(str, Enum):
    CREDIT_CARD = "credit_card"
    DEBIT_CARD = "debit_card"
    BANK_ACCOUNT = "bank_account"
    WALLET = "wallet"


class Currency(str, Enum):
    USD = "USD"
    EUR = "EUR"
    GBP = "GBP"
    INR = "INR"


# User Models
class UserBase(BaseModel):
    email: EmailStr
    first_name: str = Field(..., min_length=1, max_length=50)
    last_name: str = Field(..., min_length=1, max_length=50)
    phone: Optional[str] = Field(None, regex=r'^\+?[1-9]\d{1,14}$')
    role: UserRole = UserRole.USER


class UserCreate(UserBase):
    password: str = Field(..., min_length=8)


class UserUpdate(BaseModel):
    first_name: Optional[str] = Field(None, min_length=1, max_length=50)
    last_name: Optional[str] = Field(None, min_length=1, max_length=50)
    phone: Optional[str] = Field(None, regex=r'^\+?[1-9]\d{1,14}$')


class UserResponse(UserBase):
    id: str
    is_active: bool
    is_verified: bool
    created_at: datetime
    updated_at: datetime

    class Config:
        from_attributes = True


class UserLogin(BaseModel):
    email: EmailStr
    password: str


# Wallet Models
class WalletBase(BaseModel):
    currency: Currency = Currency.USD


class WalletCreate(WalletBase):
    user_id: str


class WalletResponse(WalletBase):
    id: str
    user_id: str
    balance: Decimal
    is_active: bool
    created_at: datetime
    updated_at: datetime

    class Config:
        from_attributes = True


# Transaction Models
class TransactionBase(BaseModel):
    amount: Decimal = Field(..., gt=0, decimal_places=2)
    currency: Currency = Currency.USD
    description: Optional[str] = Field(None, max_length=500)


class TransactionCreate(TransactionBase):
    transaction_type: TransactionType
    from_wallet_id: Optional[str] = None
    to_wallet_id: Optional[str] = None
    payment_method: Optional[PaymentMethod] = None
    metadata: Optional[dict] = None

    @validator('from_wallet_id', 'to_wallet_id')
    def validate_wallets(cls, v, values):
        if values.get('transaction_type') == TransactionType.TRANSFER:
            if not values.get('from_wallet_id') or not values.get('to_wallet_id'):
                raise ValueError('Both from_wallet_id and to_wallet_id required for transfers')
        return v


class TransactionResponse(TransactionBase):
    id: str
    transaction_type: TransactionType
    status: TransactionStatus
    from_wallet_id: Optional[str]
    to_wallet_id: Optional[str]
    payment_method: Optional[PaymentMethod]
    reference_id: str
    metadata: Optional[dict]
    created_at: datetime
    updated_at: datetime

    class Config:
        from_attributes = True


# Payment Models
class PaymentCreate(BaseModel):
    amount: Decimal = Field(..., gt=0, decimal_places=2)
    currency: Currency = Currency.USD
    payment_method: PaymentMethod
    card_number: Optional[str] = Field(None, regex=r'^\d{16}$')
    card_cvv: Optional[str] = Field(None, regex=r'^\d{3,4}$')
    card_expiry: Optional[str] = Field(None, regex=r'^(0[1-9]|1[0-2])\/\d{2}$')
    bank_account: Optional[str] = None
    description: Optional[str] = Field(None, max_length=500)


class PaymentResponse(BaseModel):
    payment_id: str
    status: TransactionStatus
    transaction_id: str
    amount: Decimal
    currency: Currency
    reference_id: str
    created_at: datetime


# Notification Models
class NotificationCreate(BaseModel):
    user_id: str
    type: str
    title: str
    message: str
    metadata: Optional[dict] = None


class NotificationResponse(BaseModel):
    id: str
    user_id: str
    type: str
    title: str
    message: str
    is_read: bool
    created_at: datetime

    class Config:
        from_attributes = True


# API Response Models
class SuccessResponse(BaseModel):
    success: bool = True
    message: str
    data: Optional[dict] = None


class ErrorResponse(BaseModel):
    success: bool = False
    error: str
    details: Optional[dict] = None


# Kafka Event Models
class KafkaEvent(BaseModel):
    event_type: str
    timestamp: datetime
    data: dict
    metadata: Optional[dict] = None

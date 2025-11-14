"""
Shared utility functions used across all microservices
"""
import os
import jwt
import bcrypt
import logging
import uuid
from datetime import datetime, timedelta
from typing import Optional, Dict, Any
from functools import wraps
from fastapi import HTTPException, status, Request
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
import redis
import json


# JWT Configuration
SECRET_KEY = os.getenv("JWT_SECRET_KEY", "your-secret-key-change-in-production")
ALGORITHM = "HS256"
ACCESS_TOKEN_EXPIRE_MINUTES = 60
REFRESH_TOKEN_EXPIRE_DAYS = 7


# Redis Configuration
REDIS_HOST = os.getenv("REDIS_HOST", "redis")
REDIS_PORT = int(os.getenv("REDIS_PORT", "6379"))
REDIS_DB = int(os.getenv("REDIS_DB", "0"))


# Logging Configuration
def setup_logging(service_name: str) -> logging.Logger:
    """Setup structured logging for microservices"""
    logging.basicConfig(
        level=logging.INFO,
        format=f'%(asctime)s - {service_name} - %(name)s - %(levelname)s - %(message)s',
        handlers=[
            logging.StreamHandler(),
            logging.FileHandler(f'/var/log/{service_name}.log')
        ]
    )
    return logging.getLogger(service_name)


# Password Hashing
def hash_password(password: str) -> str:
    """Hash a password using bcrypt"""
    salt = bcrypt.gensalt()
    hashed = bcrypt.hashpw(password.encode('utf-8'), salt)
    return hashed.decode('utf-8')


def verify_password(plain_password: str, hashed_password: str) -> bool:
    """Verify a password against its hash"""
    return bcrypt.checkpw(
        plain_password.encode('utf-8'),
        hashed_password.encode('utf-8')
    )


# JWT Token Management
def create_access_token(data: dict, expires_delta: Optional[timedelta] = None) -> str:
    """Create JWT access token"""
    to_encode = data.copy()
    if expires_delta:
        expire = datetime.utcnow() + expires_delta
    else:
        expire = datetime.utcnow() + timedelta(minutes=ACCESS_TOKEN_EXPIRE_MINUTES)

    to_encode.update({"exp": expire, "type": "access"})
    encoded_jwt = jwt.encode(to_encode, SECRET_KEY, algorithm=ALGORITHM)
    return encoded_jwt


def create_refresh_token(data: dict) -> str:
    """Create JWT refresh token"""
    to_encode = data.copy()
    expire = datetime.utcnow() + timedelta(days=REFRESH_TOKEN_EXPIRE_DAYS)
    to_encode.update({"exp": expire, "type": "refresh"})
    encoded_jwt = jwt.encode(to_encode, SECRET_KEY, algorithm=ALGORITHM)
    return encoded_jwt


def decode_token(token: str) -> Dict[str, Any]:
    """Decode and verify JWT token"""
    try:
        payload = jwt.decode(token, SECRET_KEY, algorithms=[ALGORITHM])
        return payload
    except jwt.ExpiredSignatureError:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Token has expired"
        )
    except jwt.JWTError:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Could not validate credentials"
        )


# Redis Client
class RedisClient:
    """Redis client for caching and session management"""

    def __init__(self):
        self.client = redis.Redis(
            host=REDIS_HOST,
            port=REDIS_PORT,
            db=REDIS_DB,
            decode_responses=True
        )

    def get(self, key: str) -> Optional[Any]:
        """Get value from Redis"""
        value = self.client.get(key)
        if value:
            try:
                return json.loads(value)
            except json.JSONDecodeError:
                return value
        return None

    def set(self, key: str, value: Any, expiration: int = 3600) -> bool:
        """Set value in Redis with expiration"""
        if isinstance(value, (dict, list)):
            value = json.dumps(value)
        return self.client.setex(key, expiration, value)

    def delete(self, key: str) -> bool:
        """Delete key from Redis"""
        return self.client.delete(key) > 0

    def exists(self, key: str) -> bool:
        """Check if key exists in Redis"""
        return self.client.exists(key) > 0

    def increment(self, key: str, amount: int = 1) -> int:
        """Increment counter in Redis"""
        return self.client.incrby(key, amount)

    def set_with_ttl(self, key: str, value: Any, ttl: int) -> bool:
        """Set value with custom TTL"""
        return self.set(key, value, ttl)


# Generate unique IDs
def generate_uuid() -> str:
    """Generate UUID for entities"""
    return str(uuid.uuid4())


def generate_reference_id(prefix: str = "TXN") -> str:
    """Generate unique reference ID for transactions"""
    timestamp = datetime.utcnow().strftime("%Y%m%d%H%M%S")
    random_part = str(uuid.uuid4())[:8].upper()
    return f"{prefix}-{timestamp}-{random_part}"


# Rate Limiting
class RateLimiter:
    """Rate limiter using Redis"""

    def __init__(self, redis_client: RedisClient):
        self.redis = redis_client

    def check_rate_limit(
        self,
        identifier: str,
        max_requests: int = 100,
        window_seconds: int = 60
    ) -> bool:
        """Check if request is within rate limit"""
        key = f"rate_limit:{identifier}"
        current = self.redis.increment(key)

        if current == 1:
            self.redis.set_with_ttl(key, current, window_seconds)

        return current <= max_requests


# Error Handlers
class PaymentSystemException(Exception):
    """Base exception for payment system"""
    def __init__(self, message: str, status_code: int = 500, details: Optional[dict] = None):
        self.message = message
        self.status_code = status_code
        self.details = details
        super().__init__(self.message)


class InsufficientFundsException(PaymentSystemException):
    """Exception for insufficient funds"""
    def __init__(self, message: str = "Insufficient funds"):
        super().__init__(message, status_code=400)


class WalletNotFoundException(PaymentSystemException):
    """Exception for wallet not found"""
    def __init__(self, message: str = "Wallet not found"):
        super().__init__(message, status_code=404)


class TransactionFailedException(PaymentSystemException):
    """Exception for transaction failure"""
    def __init__(self, message: str = "Transaction failed"):
        super().__init__(message, status_code=400)


# Decorator for authentication
security = HTTPBearer()


def require_auth(func):
    """Decorator to require authentication"""
    @wraps(func)
    async def wrapper(*args, **kwargs):
        request = kwargs.get('request')
        if not request:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Authentication required"
            )

        auth_header = request.headers.get('Authorization')
        if not auth_header or not auth_header.startswith('Bearer '):
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Invalid authentication credentials"
            )

        token = auth_header.split(' ')[1]
        payload = decode_token(token)
        kwargs['current_user'] = payload

        return await func(*args, **kwargs)

    return wrapper


# Monitoring and metrics helpers
def get_service_health() -> Dict[str, Any]:
    """Get service health status"""
    return {
        "status": "healthy",
        "timestamp": datetime.utcnow().isoformat(),
        "uptime": "N/A"
    }


# Data validation helpers
def validate_decimal_precision(value: float, decimal_places: int = 2) -> float:
    """Validate decimal precision for monetary values"""
    return round(value, decimal_places)

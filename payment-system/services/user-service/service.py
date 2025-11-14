"""
User Service Business Logic
"""
from typing import Optional, Dict, Any
from fastapi import HTTPException, status
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
from repository import UserRepository
from shared.models import UserCreate, UserUpdate, UserLogin, UserResponse
from shared.utils import (
    verify_password,
    create_access_token,
    create_refresh_token,
    RedisClient
)
from shared.kafka_client import KafkaProducerClient, KafkaTopics


class UserService:
    """User service business logic"""

    def __init__(self, repository: UserRepository):
        self.repository = repository
        self.redis = RedisClient()
        self.kafka_producer = KafkaProducerClient()

    def register_user(self, user_data: UserCreate) -> UserResponse:
        """Register a new user"""
        # Check if user already exists
        if self.repository.user_exists(user_data.email):
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Email already registered"
            )

        # Create user
        user = self.repository.create_user(user_data)

        # Send user registration event to Kafka
        self.kafka_producer.send_event(
            topic=KafkaTopics.USER_EVENTS,
            event_type="user.registered",
            data={
                "user_id": user.id,
                "email": user.email,
                "role": user.role.value
            }
        )

        return UserResponse(**user.to_dict())

    def authenticate_user(self, credentials: UserLogin) -> Dict[str, Any]:
        """Authenticate user and return tokens"""
        user = self.repository.get_user_by_email(credentials.email)

        if not user or not verify_password(credentials.password, user.hashed_password):
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Incorrect email or password"
            )

        if not user.is_active:
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Account is deactivated"
            )

        # Generate tokens
        token_data = {
            "sub": user.id,
            "email": user.email,
            "role": user.role.value
        }
        access_token = create_access_token(token_data)
        refresh_token = create_refresh_token(token_data)

        # Cache user session in Redis
        self.redis.set(
            f"user_session:{user.id}",
            {"user_id": user.id, "email": user.email, "role": user.role.value},
            expiration=3600
        )

        # Send login event to Kafka
        self.kafka_producer.send_event(
            topic=KafkaTopics.USER_EVENTS,
            event_type="user.logged_in",
            data={"user_id": user.id, "email": user.email}
        )

        return {
            "access_token": access_token,
            "refresh_token": refresh_token,
            "token_type": "bearer",
            "user": UserResponse(**user.to_dict())
        }

    def get_user(self, user_id: str) -> UserResponse:
        """Get user by ID"""
        # Try to get from cache first
        cached_user = self.redis.get(f"user:{user_id}")
        if cached_user:
            return UserResponse(**cached_user)

        user = self.repository.get_user_by_id(user_id)
        if not user:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="User not found"
            )

        # Cache user data
        user_dict = user.to_dict()
        self.redis.set(f"user:{user_id}", user_dict, expiration=1800)

        return UserResponse(**user_dict)

    def update_user(self, user_id: str, user_data: UserUpdate) -> UserResponse:
        """Update user information"""
        user = self.repository.update_user(user_id, user_data)
        if not user:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="User not found"
            )

        # Invalidate cache
        self.redis.delete(f"user:{user_id}")

        # Send update event to Kafka
        self.kafka_producer.send_event(
            topic=KafkaTopics.USER_EVENTS,
            event_type="user.updated",
            data={"user_id": user.id, "updated_fields": user_data.dict(exclude_unset=True)}
        )

        return UserResponse(**user.to_dict())

    def verify_user(self, user_id: str) -> bool:
        """Verify user account"""
        result = self.repository.verify_user(user_id)
        if not result:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="User not found"
            )

        # Invalidate cache
        self.redis.delete(f"user:{user_id}")

        # Send verification event to Kafka
        self.kafka_producer.send_event(
            topic=KafkaTopics.USER_EVENTS,
            event_type="user.verified",
            data={"user_id": user_id}
        )

        return True

    def logout_user(self, user_id: str) -> bool:
        """Logout user and invalidate session"""
        # Remove session from Redis
        self.redis.delete(f"user_session:{user_id}")

        # Send logout event to Kafka
        self.kafka_producer.send_event(
            topic=KafkaTopics.USER_EVENTS,
            event_type="user.logged_out",
            data={"user_id": user_id}
        )

        return True

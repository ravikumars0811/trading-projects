"""
User Service API Routes
"""
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from typing import List
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
from repository import UserRepository
from service import UserService
from shared.database import get_db
from shared.models import (
    UserCreate,
    UserUpdate,
    UserLogin,
    UserResponse,
    SuccessResponse
)
from shared.utils import decode_token

router = APIRouter(prefix="/api/v1/users", tags=["users"])


def get_user_service(db: Session = Depends(get_db)) -> UserService:
    """Dependency to get user service"""
    repository = UserRepository(db)
    return UserService(repository)


def get_current_user(
    authorization: str = Depends(lambda: None),
    db: Session = Depends(get_db)
) -> dict:
    """Get current authenticated user"""
    from fastapi import Request
    # This is a placeholder - actual implementation would extract from request
    # For now, we'll use a simple approach
    return {"user_id": "dummy"}


@router.post("/register", response_model=UserResponse, status_code=status.HTTP_201_CREATED)
async def register_user(
    user_data: UserCreate,
    service: UserService = Depends(get_user_service)
):
    """
    Register a new user

    - **email**: User's email address (unique)
    - **password**: User's password (min 8 characters)
    - **first_name**: User's first name
    - **last_name**: User's last name
    - **phone**: User's phone number (optional)
    - **role**: User role (user/merchant/admin)
    """
    return service.register_user(user_data)


@router.post("/login")
async def login_user(
    credentials: UserLogin,
    service: UserService = Depends(get_user_service)
):
    """
    Authenticate user and get access tokens

    - **email**: User's email address
    - **password**: User's password

    Returns access_token, refresh_token, and user information
    """
    return service.authenticate_user(credentials)


@router.get("/me", response_model=UserResponse)
async def get_current_user_info(
    current_user: dict = Depends(get_current_user),
    service: UserService = Depends(get_user_service)
):
    """
    Get current authenticated user's information
    """
    return service.get_user(current_user["user_id"])


@router.get("/{user_id}", response_model=UserResponse)
async def get_user(
    user_id: str,
    service: UserService = Depends(get_user_service)
):
    """
    Get user by ID

    - **user_id**: User's unique identifier
    """
    return service.get_user(user_id)


@router.put("/{user_id}", response_model=UserResponse)
async def update_user(
    user_id: str,
    user_data: UserUpdate,
    current_user: dict = Depends(get_current_user),
    service: UserService = Depends(get_user_service)
):
    """
    Update user information

    - **user_id**: User's unique identifier
    - **first_name**: Updated first name (optional)
    - **last_name**: Updated last name (optional)
    - **phone**: Updated phone number (optional)
    """
    # Check if user is updating their own profile or is admin
    if current_user["user_id"] != user_id and current_user.get("role") != "admin":
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Not authorized to update this user"
        )

    return service.update_user(user_id, user_data)


@router.post("/{user_id}/verify", response_model=SuccessResponse)
async def verify_user(
    user_id: str,
    service: UserService = Depends(get_user_service)
):
    """
    Verify user account

    - **user_id**: User's unique identifier
    """
    result = service.verify_user(user_id)
    return SuccessResponse(
        message="User verified successfully",
        data={"user_id": user_id}
    )


@router.post("/logout", response_model=SuccessResponse)
async def logout_user(
    current_user: dict = Depends(get_current_user),
    service: UserService = Depends(get_user_service)
):
    """
    Logout current user and invalidate session
    """
    result = service.logout_user(current_user["user_id"])
    return SuccessResponse(
        message="Logged out successfully",
        data={"user_id": current_user["user_id"]}
    )


@router.get("/health", status_code=status.HTTP_200_OK)
async def health_check():
    """
    Health check endpoint
    """
    return {"status": "healthy", "service": "user-service"}

from fastapi import FastAPI, HTTPException, Depends, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from contextlib import asynccontextmanager
from sqlalchemy.ext.asyncio import AsyncSession
from prometheus_client import Counter, Histogram, generate_latest, CONTENT_TYPE_LATEST
from fastapi import Response
import logging
import time

from app.database import engine, get_db, Base
from app.models import User
from app.schemas import UserCreate, UserResponse, UserUpdate
from app.auth import get_password_hash, verify_password
from app.cache import get_redis_client
from app.kafka_producer import get_kafka_producer
from app.config import settings
from sqlalchemy import select
import json

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Prometheus metrics
REQUEST_COUNT = Counter('user_service_requests_total', 'Total requests', ['method', 'endpoint', 'status'])
REQUEST_DURATION = Histogram('user_service_request_duration_seconds', 'Request duration', ['method', 'endpoint'])
USER_CREATED = Counter('user_service_users_created_total', 'Total users created')
USER_UPDATED = Counter('user_service_users_updated_total', 'Total users updated')

@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup
    logger.info(f"Starting {settings.SERVICE_NAME}...")
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
    logger.info("Database tables created/verified")
    yield
    # Shutdown
    logger.info(f"Shutting down {settings.SERVICE_NAME}...")
    await engine.dispose()

app = FastAPI(
    title="User Service",
    description="Microservice for user management",
    version="1.0.0",
    lifespan=lifespan
)

# CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Middleware for metrics
@app.middleware("http")
async def metrics_middleware(request, call_next):
    start_time = time.time()
    response = await call_next(request)
    duration = time.time() - start_time

    REQUEST_COUNT.labels(
        method=request.method,
        endpoint=request.url.path,
        status=response.status_code
    ).inc()

    REQUEST_DURATION.labels(
        method=request.method,
        endpoint=request.url.path
    ).observe(duration)

    return response

# Health check endpoints
@app.get("/health")
async def health_check():
    """Health check endpoint for container orchestration"""
    return {"status": "healthy", "service": settings.SERVICE_NAME}

@app.get("/ready")
async def readiness_check(db: AsyncSession = Depends(get_db)):
    """Readiness check - verifies database connection"""
    try:
        await db.execute(select(1))
        return {"status": "ready", "service": settings.SERVICE_NAME}
    except Exception as e:
        logger.error(f"Readiness check failed: {e}")
        raise HTTPException(status_code=503, detail="Service not ready")

@app.get("/metrics")
async def metrics():
    """Prometheus metrics endpoint"""
    return Response(content=generate_latest(), media_type=CONTENT_TYPE_LATEST)

# User endpoints
@app.post("/users", response_model=UserResponse, status_code=status.HTTP_201_CREATED)
async def create_user(
    user_data: UserCreate,
    db: AsyncSession = Depends(get_db)
):
    """Create a new user"""
    try:
        # Check if user already exists
        result = await db.execute(
            select(User).where(
                (User.email == user_data.email) | (User.username == user_data.username)
            )
        )
        existing_user = result.scalar_one_or_none()

        if existing_user:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="User with this email or username already exists"
            )

        # Create new user
        hashed_password = get_password_hash(user_data.password)
        new_user = User(
            username=user_data.username,
            email=user_data.email,
            password_hash=hashed_password,
            full_name=user_data.full_name
        )

        db.add(new_user)
        await db.commit()
        await db.refresh(new_user)

        # Publish event to Kafka
        producer = get_kafka_producer()
        event = {
            "event_type": "user_created",
            "user_id": new_user.id,
            "username": new_user.username,
            "email": new_user.email,
            "timestamp": new_user.created_at.isoformat()
        }
        producer.send('user-events', value=json.dumps(event).encode('utf-8'))
        producer.flush()

        # Cache user data
        redis_client = get_redis_client()
        cache_key = f"user:{new_user.id}"
        redis_client.setex(
            cache_key,
            3600,  # 1 hour TTL
            json.dumps({
                "id": new_user.id,
                "username": new_user.username,
                "email": new_user.email,
                "full_name": new_user.full_name
            })
        )

        USER_CREATED.inc()
        logger.info(f"User created: {new_user.username} (ID: {new_user.id})")

        return new_user

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error creating user: {e}")
        await db.rollback()
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to create user"
        )

@app.get("/users/{user_id}", response_model=UserResponse)
async def get_user(user_id: int, db: AsyncSession = Depends(get_db)):
    """Get user by ID with caching"""
    try:
        # Try cache first
        redis_client = get_redis_client()
        cache_key = f"user:{user_id}"
        cached_user = redis_client.get(cache_key)

        if cached_user:
            logger.info(f"Cache hit for user {user_id}")
            user_data = json.loads(cached_user)
            return UserResponse(**user_data)

        # Cache miss - query database
        logger.info(f"Cache miss for user {user_id}")
        result = await db.execute(select(User).where(User.id == user_id))
        user = result.scalar_one_or_none()

        if not user:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="User not found"
            )

        # Update cache
        redis_client.setex(
            cache_key,
            3600,
            json.dumps({
                "id": user.id,
                "username": user.username,
                "email": user.email,
                "full_name": user.full_name,
                "is_active": user.is_active
            })
        )

        return user

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error fetching user: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to fetch user"
        )

@app.get("/users", response_model=list[UserResponse])
async def list_users(
    skip: int = 0,
    limit: int = 100,
    db: AsyncSession = Depends(get_db)
):
    """List all users with pagination"""
    try:
        result = await db.execute(
            select(User).offset(skip).limit(limit)
        )
        users = result.scalars().all()
        return users
    except Exception as e:
        logger.error(f"Error listing users: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to list users"
        )

@app.put("/users/{user_id}", response_model=UserResponse)
async def update_user(
    user_id: int,
    user_data: UserUpdate,
    db: AsyncSession = Depends(get_db)
):
    """Update user information"""
    try:
        result = await db.execute(select(User).where(User.id == user_id))
        user = result.scalar_one_or_none()

        if not user:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="User not found"
            )

        # Update fields
        if user_data.full_name is not None:
            user.full_name = user_data.full_name
        if user_data.email is not None:
            user.email = user_data.email
        if user_data.is_active is not None:
            user.is_active = user_data.is_active

        await db.commit()
        await db.refresh(user)

        # Invalidate cache
        redis_client = get_redis_client()
        cache_key = f"user:{user_id}"
        redis_client.delete(cache_key)

        # Publish event
        producer = get_kafka_producer()
        event = {
            "event_type": "user_updated",
            "user_id": user.id,
            "timestamp": user.updated_at.isoformat()
        }
        producer.send('user-events', value=json.dumps(event).encode('utf-8'))
        producer.flush()

        USER_UPDATED.inc()
        logger.info(f"User updated: {user.username} (ID: {user.id})")

        return user

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error updating user: {e}")
        await db.rollback()
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to update user"
        )

@app.delete("/users/{user_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_user(user_id: int, db: AsyncSession = Depends(get_db)):
    """Delete a user"""
    try:
        result = await db.execute(select(User).where(User.id == user_id))
        user = result.scalar_one_or_none()

        if not user:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="User not found"
            )

        await db.delete(user)
        await db.commit()

        # Invalidate cache
        redis_client = get_redis_client()
        cache_key = f"user:{user_id}"
        redis_client.delete(cache_key)

        # Publish event
        producer = get_kafka_producer()
        event = {
            "event_type": "user_deleted",
            "user_id": user_id,
            "timestamp": time.time()
        }
        producer.send('user-events', value=json.dumps(event).encode('utf-8'))
        producer.flush()

        logger.info(f"User deleted: ID {user_id}")

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error deleting user: {e}")
        await db.rollback()
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to delete user"
        )

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

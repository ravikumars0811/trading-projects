from fastapi import FastAPI, HTTPException, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import Response
from contextlib import asynccontextmanager
from prometheus_client import Counter, Histogram, generate_latest, CONTENT_TYPE_LATEST
import logging
import time
from typing import List

from app.config import settings
from app.kafka_consumer import start_kafka_consumer, stop_kafka_consumer
from app.notification_store import get_notifications, add_notification

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Prometheus metrics
REQUEST_COUNT = Counter('notification_service_requests_total', 'Total requests', ['method', 'endpoint', 'status'])
REQUEST_DURATION = Histogram('notification_service_request_duration_seconds', 'Request duration', ['method', 'endpoint'])
NOTIFICATIONS_SENT = Counter('notification_service_notifications_sent_total', 'Total notifications sent', ['type'])

@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info(f"Starting {settings.SERVICE_NAME}...")

    # Start Kafka consumer in background
    start_kafka_consumer()
    logger.info("Kafka consumer started")

    yield

    # Shutdown
    logger.info(f"Shutting down {settings.SERVICE_NAME}...")
    stop_kafka_consumer()

app = FastAPI(
    title="Notification Service",
    description="Microservice for handling notifications",
    version="1.0.0",
    lifespan=lifespan
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

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

@app.get("/health")
async def health_check():
    return {"status": "healthy", "service": settings.SERVICE_NAME}

@app.get("/ready")
async def readiness_check():
    return {"status": "ready", "service": settings.SERVICE_NAME}

@app.get("/metrics")
async def metrics():
    return Response(content=generate_latest(), media_type=CONTENT_TYPE_LATEST)

@app.get("/notifications")
async def list_notifications(limit: int = 50):
    """List recent notifications"""
    try:
        notifications = get_notifications(limit)
        return {"notifications": notifications, "count": len(notifications)}
    except Exception as e:
        logger.error(f"Error listing notifications: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to list notifications"
        )

@app.get("/notifications/user/{user_id}")
async def get_user_notifications(user_id: int, limit: int = 20):
    """Get notifications for a specific user"""
    try:
        all_notifications = get_notifications(100)
        user_notifications = [
            n for n in all_notifications
            if n.get('user_id') == user_id
        ][:limit]

        return {
            "user_id": user_id,
            "notifications": user_notifications,
            "count": len(user_notifications)
        }
    except Exception as e:
        logger.error(f"Error getting user notifications: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to get user notifications"
        )

@app.post("/notifications/test")
async def send_test_notification(message: str, user_id: int = None):
    """Send a test notification"""
    try:
        notification = {
            "type": "test",
            "message": message,
            "user_id": user_id,
            "timestamp": time.time()
        }

        add_notification(notification)
        NOTIFICATIONS_SENT.labels(type="test").inc()

        logger.info(f"Test notification sent: {message}")

        return {"status": "sent", "notification": notification}
    except Exception as e:
        logger.error(f"Error sending test notification: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to send test notification"
        )

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

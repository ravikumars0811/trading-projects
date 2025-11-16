from fastapi import FastAPI, HTTPException, Depends, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import Response
from contextlib import asynccontextmanager
from sqlalchemy.ext.asyncio import AsyncSession
from prometheus_client import Counter, Histogram, generate_latest, CONTENT_TYPE_LATEST
import logging
import time
import json
import httpx

from app.database import engine, get_db, Base
from app.models import Order, OrderItem
from app.schemas import OrderCreate, OrderResponse, OrderItemResponse
from app.cache import get_redis_client
from app.kafka_producer import get_kafka_producer
from app.config import settings
from sqlalchemy import select
from sqlalchemy.orm import selectinload
from decimal import Decimal

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Prometheus metrics
REQUEST_COUNT = Counter('order_service_requests_total', 'Total requests', ['method', 'endpoint', 'status'])
REQUEST_DURATION = Histogram('order_service_request_duration_seconds', 'Request duration', ['method', 'endpoint'])
ORDER_CREATED = Counter('order_service_orders_created_total', 'Total orders created')
ORDER_COMPLETED = Counter('order_service_orders_completed_total', 'Total orders completed')

@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info(f"Starting {settings.SERVICE_NAME}...")
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
    logger.info("Database tables created/verified")
    yield
    logger.info(f"Shutting down {settings.SERVICE_NAME}...")
    await engine.dispose()

app = FastAPI(
    title="Order Service",
    description="Microservice for order management",
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
async def readiness_check(db: AsyncSession = Depends(get_db)):
    try:
        await db.execute(select(1))
        return {"status": "ready", "service": settings.SERVICE_NAME}
    except Exception as e:
        logger.error(f"Readiness check failed: {e}")
        raise HTTPException(status_code=503, detail="Service not ready")

@app.get("/metrics")
async def metrics():
    return Response(content=generate_latest(), media_type=CONTENT_TYPE_LATEST)

async def verify_user(user_id: int) -> bool:
    """Verify user exists by calling User Service"""
    try:
        async with httpx.AsyncClient(timeout=10.0) as client:
            response = await client.get(f"{settings.USER_SERVICE_URL}/users/{user_id}")
            return response.status_code == 200
    except Exception as e:
        logger.error(f"Error verifying user: {e}")
        return False

async def get_product_info(product_id: int) -> dict:
    """Get product information from Product Service"""
    try:
        async with httpx.AsyncClient(timeout=10.0) as client:
            response = await client.get(f"{settings.PRODUCT_SERVICE_URL}/products/{product_id}")
            if response.status_code == 200:
                return response.json()
            return None
    except Exception as e:
        logger.error(f"Error getting product info: {e}")
        return None

@app.post("/orders", response_model=OrderResponse, status_code=status.HTTP_201_CREATED)
async def create_order(
    order_data: OrderCreate,
    db: AsyncSession = Depends(get_db)
):
    """Create a new order with items"""
    try:
        # Verify user exists
        user_exists = await verify_user(order_data.user_id)
        if not user_exists:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="User not found"
            )

        # Verify all products exist and calculate total
        total_amount = Decimal(0)
        items_data = []

        for item in order_data.items:
            product_info = await get_product_info(item.product_id)
            if not product_info:
                raise HTTPException(
                    status_code=status.HTTP_404_NOT_FOUND,
                    detail=f"Product {item.product_id} not found"
                )

            unit_price = Decimal(str(product_info['price']))
            subtotal = unit_price * item.quantity
            total_amount += subtotal

            items_data.append({
                'product_id': item.product_id,
                'quantity': item.quantity,
                'unit_price': unit_price,
                'subtotal': subtotal,
                'product_name': product_info['name']
            })

        # Create order
        new_order = Order(
            user_id=order_data.user_id,
            total_amount=total_amount,
            status='pending'
        )

        db.add(new_order)
        await db.flush()  # Get order ID

        # Create order items
        for item_data in items_data:
            order_item = OrderItem(
                order_id=new_order.id,
                product_id=item_data['product_id'],
                quantity=item_data['quantity'],
                unit_price=item_data['unit_price'],
                subtotal=item_data['subtotal']
            )
            db.add(order_item)

        await db.commit()
        await db.refresh(new_order)

        # Publish order created event to Kafka
        producer = get_kafka_producer()
        event = {
            "event_type": "order_created",
            "order_id": new_order.id,
            "user_id": new_order.user_id,
            "total_amount": float(new_order.total_amount),
            "items": [
                {
                    "product_id": item['product_id'],
                    "quantity": item['quantity'],
                    "unit_price": float(item['unit_price'])
                }
                for item in items_data
            ],
            "timestamp": new_order.created_at.isoformat()
        }
        producer.send('order-events', value=json.dumps(event).encode('utf-8'))
        producer.flush()

        ORDER_CREATED.inc()
        logger.info(f"Order created: ID {new_order.id}, User {new_order.user_id}, Amount ${new_order.total_amount}")

        # Fetch order with items for response
        result = await db.execute(
            select(Order).options(selectinload(Order.items)).where(Order.id == new_order.id)
        )
        order = result.scalar_one()

        return order

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error creating order: {e}")
        await db.rollback()
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to create order: {str(e)}"
        )

@app.get("/orders/{order_id}", response_model=OrderResponse)
async def get_order(order_id: int, db: AsyncSession = Depends(get_db)):
    """Get order by ID with items"""
    try:
        result = await db.execute(
            select(Order).options(selectinload(Order.items)).where(Order.id == order_id)
        )
        order = result.scalar_one_or_none()

        if not order:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="Order not found"
            )

        return order

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error fetching order: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to fetch order"
        )

@app.get("/orders", response_model=list[OrderResponse])
async def list_orders(
    skip: int = 0,
    limit: int = 100,
    user_id: int = None,
    db: AsyncSession = Depends(get_db)
):
    """List orders with optional filtering"""
    try:
        query = select(Order).options(selectinload(Order.items))

        if user_id:
            query = query.where(Order.user_id == user_id)

        query = query.offset(skip).limit(limit)
        result = await db.execute(query)
        orders = result.scalars().all()

        return orders
    except Exception as e:
        logger.error(f"Error listing orders: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to list orders"
        )

@app.put("/orders/{order_id}/status")
async def update_order_status(
    order_id: int,
    status_value: str,
    db: AsyncSession = Depends(get_db)
):
    """Update order status"""
    try:
        valid_statuses = ['pending', 'processing', 'completed', 'cancelled']
        if status_value not in valid_statuses:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Invalid status. Must be one of: {valid_statuses}"
            )

        result = await db.execute(select(Order).where(Order.id == order_id))
        order = result.scalar_one_or_none()

        if not order:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="Order not found"
            )

        old_status = order.status
        order.status = status_value
        await db.commit()

        # Publish status update event
        producer = get_kafka_producer()
        event = {
            "event_type": "order_status_updated",
            "order_id": order.id,
            "old_status": old_status,
            "new_status": status_value,
            "timestamp": time.time()
        }
        producer.send('order-events', value=json.dumps(event).encode('utf-8'))
        producer.flush()

        if status_value == 'completed':
            ORDER_COMPLETED.inc()

        logger.info(f"Order {order_id} status updated: {old_status} -> {status_value}")

        return {"order_id": order_id, "status": status_value}

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error updating order status: {e}")
        await db.rollback()
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to update order status"
        )

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

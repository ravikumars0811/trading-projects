from fastapi import FastAPI, HTTPException, Depends, status, BackgroundTasks
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import Response
from contextlib import asynccontextmanager
from prometheus_client import Counter, Histogram, Gauge, generate_latest, CONTENT_TYPE_LATEST
import logging
import time
import json
from typing import List

from app.database import get_database
from app.models import InventoryItem
from app.schemas import InventoryItemCreate, InventoryItemResponse, InventoryItemUpdate
from app.cache import get_redis_client
from app.kafka_consumer import start_kafka_consumer, stop_kafka_consumer
from app.config import settings

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Prometheus metrics
REQUEST_COUNT = Counter('inventory_service_requests_total', 'Total requests', ['method', 'endpoint', 'status'])
REQUEST_DURATION = Histogram('inventory_service_request_duration_seconds', 'Request duration', ['method', 'endpoint'])
INVENTORY_UPDATES = Counter('inventory_service_updates_total', 'Total inventory updates')
LOW_STOCK_ITEMS = Gauge('inventory_service_low_stock_items', 'Number of low stock items')

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
    title="Inventory Service",
    description="Microservice for inventory management with MongoDB",
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
    try:
        db = get_database()
        # Ping MongoDB
        await db.command("ping")
        return {"status": "ready", "service": settings.SERVICE_NAME}
    except Exception as e:
        logger.error(f"Readiness check failed: {e}")
        raise HTTPException(status_code=503, detail="Service not ready")

@app.get("/metrics")
async def metrics():
    return Response(content=generate_latest(), media_type=CONTENT_TYPE_LATEST)

@app.post("/inventory", response_model=InventoryItemResponse, status_code=status.HTTP_201_CREATED)
async def create_inventory_item(item_data: InventoryItemCreate):
    """Create a new inventory item"""
    try:
        db = get_database()

        # Check if item already exists
        existing = await db.inventory.find_one({"product_id": item_data.product_id})
        if existing:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Inventory item for this product already exists"
            )

        # Create new inventory item
        item_dict = item_data.model_dump()
        result = await db.inventory.insert_one(item_dict)

        created_item = await db.inventory.find_one({"_id": result.inserted_id})

        # Cache the item
        redis_client = get_redis_client()
        cache_key = f"inventory:{item_data.product_id}"
        redis_client.setex(
            cache_key,
            3600,
            json.dumps({
                "product_id": created_item['product_id'],
                "quantity": created_item['quantity'],
                "reserved": created_item['reserved'],
                "warehouse_location": created_item.get('warehouse_location')
            })
        )

        logger.info(f"Inventory item created for product {item_data.product_id}")

        return InventoryItemResponse(
            id=str(created_item['_id']),
            product_id=created_item['product_id'],
            quantity=created_item['quantity'],
            reserved=created_item['reserved'],
            warehouse_location=created_item.get('warehouse_location'),
            last_updated=created_item.get('last_updated')
        )

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error creating inventory item: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to create inventory item"
        )

@app.get("/inventory/{product_id}", response_model=InventoryItemResponse)
async def get_inventory_item(product_id: int):
    """Get inventory item by product ID with caching"""
    try:
        # Try cache first
        redis_client = get_redis_client()
        cache_key = f"inventory:{product_id}"
        cached_item = redis_client.get(cache_key)

        if cached_item:
            logger.info(f"Cache hit for inventory product {product_id}")
            item_data = json.loads(cached_item)
            return InventoryItemResponse(
                id="cached",
                **item_data
            )

        # Cache miss - query MongoDB
        logger.info(f"Cache miss for inventory product {product_id}")
        db = get_database()
        item = await db.inventory.find_one({"product_id": product_id})

        if not item:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="Inventory item not found"
            )

        # Update cache
        redis_client.setex(
            cache_key,
            3600,
            json.dumps({
                "product_id": item['product_id'],
                "quantity": item['quantity'],
                "reserved": item['reserved'],
                "warehouse_location": item.get('warehouse_location')
            })
        )

        return InventoryItemResponse(
            id=str(item['_id']),
            product_id=item['product_id'],
            quantity=item['quantity'],
            reserved=item['reserved'],
            warehouse_location=item.get('warehouse_location'),
            last_updated=item.get('last_updated')
        )

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error fetching inventory item: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to fetch inventory item"
        )

@app.get("/inventory", response_model=List[InventoryItemResponse])
async def list_inventory(skip: int = 0, limit: int = 100):
    """List all inventory items"""
    try:
        db = get_database()
        cursor = db.inventory.find().skip(skip).limit(limit)
        items = await cursor.to_list(length=limit)

        return [
            InventoryItemResponse(
                id=str(item['_id']),
                product_id=item['product_id'],
                quantity=item['quantity'],
                reserved=item['reserved'],
                warehouse_location=item.get('warehouse_location'),
                last_updated=item.get('last_updated')
            )
            for item in items
        ]

    except Exception as e:
        logger.error(f"Error listing inventory: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to list inventory"
        )

@app.put("/inventory/{product_id}", response_model=InventoryItemResponse)
async def update_inventory_item(product_id: int, item_data: InventoryItemUpdate):
    """Update inventory item"""
    try:
        db = get_database()

        update_dict = {}
        if item_data.quantity is not None:
            update_dict['quantity'] = item_data.quantity
        if item_data.reserved is not None:
            update_dict['reserved'] = item_data.reserved
        if item_data.warehouse_location is not None:
            update_dict['warehouse_location'] = item_data.warehouse_location

        update_dict['last_updated'] = time.time()

        result = await db.inventory.update_one(
            {"product_id": product_id},
            {"$set": update_dict}
        )

        if result.matched_count == 0:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="Inventory item not found"
            )

        # Invalidate cache
        redis_client = get_redis_client()
        cache_key = f"inventory:{product_id}"
        redis_client.delete(cache_key)

        # Fetch updated item
        updated_item = await db.inventory.find_one({"product_id": product_id})

        INVENTORY_UPDATES.inc()
        logger.info(f"Inventory updated for product {product_id}")

        return InventoryItemResponse(
            id=str(updated_item['_id']),
            product_id=updated_item['product_id'],
            quantity=updated_item['quantity'],
            reserved=updated_item['reserved'],
            warehouse_location=updated_item.get('warehouse_location'),
            last_updated=updated_item.get('last_updated')
        )

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error updating inventory: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to update inventory"
        )

@app.get("/inventory/low-stock/count")
async def get_low_stock_count(threshold: int = 10):
    """Get count of low stock items"""
    try:
        db = get_database()
        count = await db.inventory.count_documents({"quantity": {"$lt": threshold}})

        LOW_STOCK_ITEMS.set(count)

        return {"low_stock_count": count, "threshold": threshold}

    except Exception as e:
        logger.error(f"Error getting low stock count: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to get low stock count"
        )

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

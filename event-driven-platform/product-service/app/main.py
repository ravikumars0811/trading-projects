from fastapi import FastAPI, HTTPException, Depends, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse, Response
from contextlib import asynccontextmanager
from sqlalchemy.ext.asyncio import AsyncSession
from prometheus_client import Counter, Histogram, generate_latest, CONTENT_TYPE_LATEST
import logging
import time
import json

from app.database import engine, get_db, Base
from app.models import Product
from app.schemas import ProductCreate, ProductResponse, ProductUpdate
from app.cache import get_redis_client
from app.kafka_producer import get_kafka_producer
from app.config import settings
from sqlalchemy import select
from decimal import Decimal

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Prometheus metrics
REQUEST_COUNT = Counter('product_service_requests_total', 'Total requests', ['method', 'endpoint', 'status'])
REQUEST_DURATION = Histogram('product_service_request_duration_seconds', 'Request duration', ['method', 'endpoint'])
PRODUCT_CREATED = Counter('product_service_products_created_total', 'Total products created')
PRODUCT_UPDATED = Counter('product_service_products_updated_total', 'Total products updated')

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
    title="Product Service",
    description="Microservice for product catalog management",
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

# Metrics middleware
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

# Product endpoints
@app.post("/products", response_model=ProductResponse, status_code=status.HTTP_201_CREATED)
async def create_product(
    product_data: ProductCreate,
    db: AsyncSession = Depends(get_db)
):
    """Create a new product"""
    try:
        # Check if SKU already exists
        result = await db.execute(select(Product).where(Product.sku == product_data.sku))
        existing_product = result.scalar_one_or_none()

        if existing_product:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Product with this SKU already exists"
            )

        # Create new product
        new_product = Product(
            name=product_data.name,
            description=product_data.description,
            price=product_data.price,
            category=product_data.category,
            sku=product_data.sku
        )

        db.add(new_product)
        await db.commit()
        await db.refresh(new_product)

        # Publish event to Kafka
        producer = get_kafka_producer()
        event = {
            "event_type": "product_created",
            "product_id": new_product.id,
            "sku": new_product.sku,
            "name": new_product.name,
            "price": float(new_product.price),
            "category": new_product.category,
            "timestamp": new_product.created_at.isoformat()
        }
        producer.send('product-events', value=json.dumps(event).encode('utf-8'))
        producer.flush()

        # Cache product data
        redis_client = get_redis_client()
        cache_key = f"product:{new_product.id}"
        redis_client.setex(
            cache_key,
            3600,
            json.dumps({
                "id": new_product.id,
                "name": new_product.name,
                "description": new_product.description,
                "price": float(new_product.price),
                "category": new_product.category,
                "sku": new_product.sku,
                "is_active": new_product.is_active
            })
        )

        PRODUCT_CREATED.inc()
        logger.info(f"Product created: {new_product.name} (ID: {new_product.id})")

        return new_product

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error creating product: {e}")
        await db.rollback()
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to create product"
        )

@app.get("/products/{product_id}", response_model=ProductResponse)
async def get_product(product_id: int, db: AsyncSession = Depends(get_db)):
    """Get product by ID with caching"""
    try:
        # Try cache first
        redis_client = get_redis_client()
        cache_key = f"product:{product_id}"
        cached_product = redis_client.get(cache_key)

        if cached_product:
            logger.info(f"Cache hit for product {product_id}")
            product_data = json.loads(cached_product)
            return ProductResponse(**product_data)

        # Cache miss
        logger.info(f"Cache miss for product {product_id}")
        result = await db.execute(select(Product).where(Product.id == product_id))
        product = result.scalar_one_or_none()

        if not product:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="Product not found"
            )

        # Update cache
        redis_client.setex(
            cache_key,
            3600,
            json.dumps({
                "id": product.id,
                "name": product.name,
                "description": product.description,
                "price": float(product.price),
                "category": product.category,
                "sku": product.sku,
                "is_active": product.is_active
            })
        )

        return product

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error fetching product: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to fetch product"
        )

@app.get("/products", response_model=list[ProductResponse])
async def list_products(
    skip: int = 0,
    limit: int = 100,
    category: str = None,
    db: AsyncSession = Depends(get_db)
):
    """List all products with pagination and filtering"""
    try:
        query = select(Product)

        if category:
            query = query.where(Product.category == category)

        query = query.offset(skip).limit(limit)
        result = await db.execute(query)
        products = result.scalars().all()

        return products
    except Exception as e:
        logger.error(f"Error listing products: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to list products"
        )

@app.put("/products/{product_id}", response_model=ProductResponse)
async def update_product(
    product_id: int,
    product_data: ProductUpdate,
    db: AsyncSession = Depends(get_db)
):
    """Update product information"""
    try:
        result = await db.execute(select(Product).where(Product.id == product_id))
        product = result.scalar_one_or_none()

        if not product:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="Product not found"
            )

        # Update fields
        if product_data.name is not None:
            product.name = product_data.name
        if product_data.description is not None:
            product.description = product_data.description
        if product_data.price is not None:
            product.price = product_data.price
        if product_data.category is not None:
            product.category = product_data.category
        if product_data.is_active is not None:
            product.is_active = product_data.is_active

        await db.commit()
        await db.refresh(product)

        # Invalidate cache
        redis_client = get_redis_client()
        cache_key = f"product:{product_id}"
        redis_client.delete(cache_key)

        # Publish event
        producer = get_kafka_producer()
        event = {
            "event_type": "product_updated",
            "product_id": product.id,
            "sku": product.sku,
            "timestamp": product.updated_at.isoformat()
        }
        producer.send('product-events', value=json.dumps(event).encode('utf-8'))
        producer.flush()

        PRODUCT_UPDATED.inc()
        logger.info(f"Product updated: {product.name} (ID: {product.id})")

        return product

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error updating product: {e}")
        await db.rollback()
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to update product"
        )

@app.delete("/products/{product_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_product(product_id: int, db: AsyncSession = Depends(get_db)):
    """Delete a product"""
    try:
        result = await db.execute(select(Product).where(Product.id == product_id))
        product = result.scalar_one_or_none()

        if not product:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="Product not found"
            )

        await db.delete(product)
        await db.commit()

        # Invalidate cache
        redis_client = get_redis_client()
        cache_key = f"product:{product_id}"
        redis_client.delete(cache_key)

        # Publish event
        producer = get_kafka_producer()
        event = {
            "event_type": "product_deleted",
            "product_id": product_id,
            "timestamp": time.time()
        }
        producer.send('product-events', value=json.dumps(event).encode('utf-8'))
        producer.flush()

        logger.info(f"Product deleted: ID {product_id}")

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error deleting product: {e}")
        await db.rollback()
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Failed to delete product"
        )

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

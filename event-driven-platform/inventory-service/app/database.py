from motor.motor_asyncio import AsyncIOMotorClient
from functools import lru_cache
from app.config import settings
import logging

logger = logging.getLogger(__name__)

@lru_cache()
def get_mongo_client():
    """Get MongoDB client singleton"""
    try:
        client = AsyncIOMotorClient(settings.MONGODB_URL)
        logger.info("MongoDB client initialized")
        return client
    except Exception as e:
        logger.error(f"Failed to initialize MongoDB client: {e}")
        raise

def get_database():
    """Get database instance"""
    client = get_mongo_client()
    return client.inventory_db

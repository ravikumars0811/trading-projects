import redis
from functools import lru_cache
from app.config import settings
import logging

logger = logging.getLogger(__name__)

@lru_cache()
def get_redis_client():
    """Get Redis client singleton"""
    try:
        client = redis.from_url(
            settings.REDIS_URL,
            decode_responses=True,
            socket_connect_timeout=5,
            socket_timeout=5,
            retry_on_timeout=True
        )
        # Test connection
        client.ping()
        logger.info("Redis connection established")
        return client
    except Exception as e:
        logger.error(f"Failed to connect to Redis: {e}")
        raise

from kafka import KafkaProducer
from functools import lru_cache
from app.config import settings
import logging

logger = logging.getLogger(__name__)

@lru_cache()
def get_kafka_producer():
    """Get Kafka producer singleton"""
    try:
        producer = KafkaProducer(
            bootstrap_servers=settings.KAFKA_BOOTSTRAP_SERVERS.split(','),
            acks='all',
            retries=3,
            max_in_flight_requests_per_connection=1,
            compression_type='gzip',
            request_timeout_ms=30000,
            api_version=(0, 10, 1)
        )
        logger.info("Kafka producer initialized")
        return producer
    except Exception as e:
        logger.error(f"Failed to initialize Kafka producer: {e}")
        raise

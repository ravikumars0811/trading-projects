"""
Kafka client for event streaming
"""
import os
import json
import logging
from typing import Optional, Callable, Any
from datetime import datetime
from kafka import KafkaProducer, KafkaConsumer
from kafka.errors import KafkaError

logger = logging.getLogger(__name__)

# Kafka Configuration
KAFKA_BOOTSTRAP_SERVERS = os.getenv(
    "KAFKA_BOOTSTRAP_SERVERS",
    "kafka:9092"
).split(",")


class KafkaProducerClient:
    """Kafka producer wrapper for sending events"""

    def __init__(self):
        self.producer = None
        self._connect()

    def _connect(self):
        """Connect to Kafka"""
        try:
            self.producer = KafkaProducer(
                bootstrap_servers=KAFKA_BOOTSTRAP_SERVERS,
                value_serializer=lambda v: json.dumps(v).encode('utf-8'),
                key_serializer=lambda k: k.encode('utf-8') if k else None,
                acks='all',
                retries=3,
                max_in_flight_requests_per_connection=1
            )
            logger.info("Connected to Kafka producer successfully")
        except Exception as e:
            logger.error(f"Error connecting to Kafka producer: {e}")
            raise

    def send_event(
        self,
        topic: str,
        event_type: str,
        data: dict,
        key: Optional[str] = None,
        metadata: Optional[dict] = None
    ) -> bool:
        """Send event to Kafka topic"""
        try:
            event = {
                "event_type": event_type,
                "timestamp": datetime.utcnow().isoformat(),
                "data": data,
                "metadata": metadata or {}
            }

            future = self.producer.send(topic, key=key, value=event)
            future.get(timeout=10)  # Wait for confirmation
            logger.info(f"Event sent to topic {topic}: {event_type}")
            return True

        except KafkaError as e:
            logger.error(f"Error sending event to Kafka: {e}")
            return False

    def close(self):
        """Close Kafka producer"""
        if self.producer:
            self.producer.flush()
            self.producer.close()
            logger.info("Kafka producer closed")


class KafkaConsumerClient:
    """Kafka consumer wrapper for receiving events"""

    def __init__(
        self,
        topics: list,
        group_id: str,
        auto_offset_reset: str = 'earliest'
    ):
        self.topics = topics
        self.group_id = group_id
        self.consumer = None
        self._connect(auto_offset_reset)

    def _connect(self, auto_offset_reset: str):
        """Connect to Kafka"""
        try:
            self.consumer = KafkaConsumer(
                *self.topics,
                bootstrap_servers=KAFKA_BOOTSTRAP_SERVERS,
                group_id=self.group_id,
                auto_offset_reset=auto_offset_reset,
                enable_auto_commit=True,
                value_deserializer=lambda m: json.loads(m.decode('utf-8')),
                key_deserializer=lambda k: k.decode('utf-8') if k else None
            )
            logger.info(f"Connected to Kafka consumer for topics: {self.topics}")
        except Exception as e:
            logger.error(f"Error connecting to Kafka consumer: {e}")
            raise

    def consume_events(self, callback: Callable[[dict], None]):
        """Consume events from Kafka topics"""
        try:
            logger.info(f"Starting to consume events from {self.topics}")
            for message in self.consumer:
                try:
                    event = message.value
                    logger.info(
                        f"Received event from {message.topic}: "
                        f"{event.get('event_type')}"
                    )
                    callback(event)
                except Exception as e:
                    logger.error(f"Error processing event: {e}")
                    continue

        except KeyboardInterrupt:
            logger.info("Consumer interrupted by user")
        except Exception as e:
            logger.error(f"Error consuming events: {e}")
        finally:
            self.close()

    def close(self):
        """Close Kafka consumer"""
        if self.consumer:
            self.consumer.close()
            logger.info("Kafka consumer closed")


# Kafka Topics
class KafkaTopics:
    """Kafka topic names"""
    USER_EVENTS = "user-events"
    TRANSACTION_EVENTS = "transaction-events"
    PAYMENT_EVENTS = "payment-events"
    NOTIFICATION_EVENTS = "notification-events"
    AUDIT_EVENTS = "audit-events"

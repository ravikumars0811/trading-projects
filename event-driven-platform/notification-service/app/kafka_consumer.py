from kafka import KafkaConsumer
import json
import logging
import threading
from app.config import settings
from app.notification_store import add_notification
from prometheus_client import Counter

logger = logging.getLogger(__name__)

NOTIFICATIONS_SENT = Counter('notification_service_notifications_sent_total', 'Total notifications sent', ['type'])

consumer_thread = None
should_stop = False

def send_notification(event_type: str, message: str, user_id: int = None):
    """Send a notification (in production, integrate with email/SMS/push services)"""
    notification = {
        "type": event_type,
        "message": message,
        "user_id": user_id,
        "timestamp": None
    }

    add_notification(notification)
    NOTIFICATIONS_SENT.labels(type=event_type).inc()

    # In production, send email/SMS/push notification here
    logger.info(f"Notification sent - Type: {event_type}, Message: {message}")

def process_event(event):
    """Process events and send notifications"""
    try:
        event_type = event.get('event_type')
        logger.info(f"Processing event: {event_type}")

        if event_type == 'user_created':
            user_id = event.get('user_id')
            username = event.get('username')
            send_notification(
                'user_created',
                f"Welcome {username}! Your account has been created successfully.",
                user_id
            )

        elif event_type == 'order_created':
            user_id = event.get('user_id')
            order_id = event.get('order_id')
            total_amount = event.get('total_amount')
            send_notification(
                'order_created',
                f"Order #{order_id} created successfully! Total: ${total_amount:.2f}",
                user_id
            )

        elif event_type == 'order_status_updated':
            order_id = event.get('order_id')
            new_status = event.get('new_status')
            send_notification(
                'order_status_updated',
                f"Order #{order_id} status updated to: {new_status}"
            )

        elif event_type == 'product_created':
            product_id = event.get('product_id')
            name = event.get('name')
            send_notification(
                'product_created',
                f"New product added: {name} (ID: {product_id})"
            )

    except Exception as e:
        logger.error(f"Error processing event: {e}")

def consume_kafka_messages():
    """Kafka consumer loop"""
    global should_stop

    try:
        consumer = KafkaConsumer(
            'user-events',
            'order-events',
            'product-events',
            bootstrap_servers=settings.KAFKA_BOOTSTRAP_SERVERS.split(','),
            auto_offset_reset='latest',
            enable_auto_commit=True,
            group_id='notification-service-group',
            value_deserializer=lambda x: json.loads(x.decode('utf-8'))
        )

        logger.info("Kafka consumer connected, listening for events...")

        for message in consumer:
            if should_stop:
                break

            event = message.value
            logger.info(f"Received event from topic {message.topic}: {event.get('event_type')}")
            process_event(event)

    except Exception as e:
        logger.error(f"Kafka consumer error: {e}")
    finally:
        if 'consumer' in locals():
            consumer.close()
        logger.info("Kafka consumer stopped")

def start_kafka_consumer():
    """Start Kafka consumer in background thread"""
    global consumer_thread, should_stop

    should_stop = False
    consumer_thread = threading.Thread(target=consume_kafka_messages, daemon=True)
    consumer_thread.start()
    logger.info("Kafka consumer thread started")

def stop_kafka_consumer():
    """Stop Kafka consumer"""
    global should_stop
    should_stop = True
    logger.info("Kafka consumer stop signal sent")

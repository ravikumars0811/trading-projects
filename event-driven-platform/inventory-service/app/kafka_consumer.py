from kafka import KafkaConsumer
import json
import logging
import threading
from app.config import settings
from app.database import get_database
import asyncio

logger = logging.getLogger(__name__)

consumer_thread = None
should_stop = False

def process_order_event(event):
    """Process order events and update inventory"""
    try:
        event_type = event.get('event_type')
        logger.info(f"Processing event: {event_type}")

        if event_type == 'order_created':
            # Reserve inventory for order items
            loop = asyncio.new_event_loop()
            asyncio.set_event_loop(loop)

            async def reserve_inventory():
                db = get_database()
                for item in event.get('items', []):
                    product_id = item['product_id']
                    quantity = item['quantity']

                    # Update inventory - reserve quantity
                    result = await db.inventory.update_one(
                        {"product_id": product_id},
                        {
                            "$inc": {"reserved": quantity},
                            "$set": {"last_updated": event.get('timestamp')}
                        }
                    )

                    if result.matched_count > 0:
                        logger.info(f"Reserved {quantity} units for product {product_id}")
                    else:
                        logger.warning(f"No inventory found for product {product_id}")

            loop.run_until_complete(reserve_inventory())
            loop.close()

        elif event_type == 'order_status_updated':
            if event.get('new_status') == 'completed':
                # Deduct reserved quantity from total
                order_id = event.get('order_id')
                logger.info(f"Processing completed order: {order_id}")

    except Exception as e:
        logger.error(f"Error processing order event: {e}")

def consume_kafka_messages():
    """Kafka consumer loop"""
    global should_stop

    try:
        consumer = KafkaConsumer(
            'order-events',
            bootstrap_servers=settings.KAFKA_BOOTSTRAP_SERVERS.split(','),
            auto_offset_reset='latest',
            enable_auto_commit=True,
            group_id='inventory-service-group',
            value_deserializer=lambda x: json.loads(x.decode('utf-8'))
        )

        logger.info("Kafka consumer connected, listening for events...")

        for message in consumer:
            if should_stop:
                break

            event = message.value
            logger.info(f"Received event: {event.get('event_type')}")
            process_order_event(event)

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

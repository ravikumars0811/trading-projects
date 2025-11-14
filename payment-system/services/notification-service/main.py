"""
Notification Service
Kafka consumer that sends notifications based on events
"""
import logging
import sys
import os
from datetime import datetime

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
from shared.kafka_client import KafkaConsumerClient, KafkaTopics
from shared.database import get_mongodb
from shared.utils import setup_logging

logger = setup_logging("notification-service")


class NotificationService:
    """Notification service to handle event-based notifications"""

    def __init__(self):
        self.mongodb = get_mongodb()

    def send_email(self, to: str, subject: str, body: str):
        """Simulate sending email"""
        logger.info(f"Sending email to {to}: {subject}")
        # In production, integrate with SendGrid, AWS SES, etc.
        self._log_notification("email", to, subject, body)

    def send_sms(self, to: str, message: str):
        """Simulate sending SMS"""
        logger.info(f"Sending SMS to {to}: {message}")
        # In production, integrate with Twilio, AWS SNS, etc.
        self._log_notification("sms", to, "SMS", message)

    def send_push_notification(self, user_id: str, title: str, message: str):
        """Simulate sending push notification"""
        logger.info(f"Sending push to {user_id}: {title}")
        # In production, integrate with Firebase Cloud Messaging, etc.
        self._log_notification("push", user_id, title, message)

    def _log_notification(self, notification_type: str, recipient: str, subject: str, body: str):
        """Log notification to MongoDB"""
        try:
            collection = self.mongodb.get_collection("notifications")
            notification = {
                "type": notification_type,
                "recipient": recipient,
                "subject": subject,
                "body": body,
                "sent_at": datetime.utcnow(),
                "status": "sent"
            }
            collection.insert_one(notification)
        except Exception as e:
            logger.error(f"Error logging notification: {e}")

    def handle_event(self, event: dict):
        """Handle incoming Kafka events"""
        event_type = event.get("event_type")
        data = event.get("data", {})

        logger.info(f"Processing event: {event_type}")

        if event_type == "user.registered":
            self.send_email(
                to=data.get("email"),
                subject="Welcome to Payment System",
                body=f"Thank you for registering! Your user ID is {data.get('user_id')}"
            )

        elif event_type == "transaction.completed":
            self.send_email(
                to="user@example.com",  # In production, fetch user email
                subject="Transaction Completed",
                body=f"Your transaction {data.get('reference_id')} of {data.get('amount')} has been completed."
            )

        elif event_type == "transaction.failed":
            self.send_email(
                to="user@example.com",
                subject="Transaction Failed",
                body=f"Your transaction {data.get('reference_id')} has failed. Please try again."
            )

        elif event_type == "payment.processed":
            self.send_email(
                to="user@example.com",
                subject="Payment Processed",
                body=f"Your payment of {data.get('amount')} {data.get('currency')} has been processed successfully."
            )

        elif event_type == "wallet.created":
            logger.info(f"Wallet created notification for user {data.get('user_id')}")

        elif event_type == "wallet.funds_added":
            logger.info(f"Funds added notification: {data.get('amount')}")

        else:
            logger.warning(f"Unhandled event type: {event_type}")


def main():
    """Main entry point for notification service"""
    logger.info("Starting Notification Service...")

    notification_service = NotificationService()

    # Subscribe to all relevant topics
    topics = [
        KafkaTopics.USER_EVENTS,
        KafkaTopics.TRANSACTION_EVENTS,
        KafkaTopics.PAYMENT_EVENTS,
        KafkaTopics.NOTIFICATION_EVENTS
    ]

    consumer = KafkaConsumerClient(
        topics=topics,
        group_id="notification-service-group"
    )

    # Start consuming events
    consumer.consume_events(notification_service.handle_event)


if __name__ == "__main__":
    main()

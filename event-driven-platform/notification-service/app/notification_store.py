from collections import deque
import logging

logger = logging.getLogger(__name__)

# In-memory notification store (in production, use Redis or database)
notifications_queue = deque(maxlen=1000)

def add_notification(notification: dict):
    """Add a notification to the store"""
    notifications_queue.append(notification)
    logger.info(f"Notification added: {notification.get('type')}")

def get_notifications(limit: int = 50):
    """Get recent notifications"""
    return list(notifications_queue)[-limit:]

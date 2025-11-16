"""
WebSocket endpoint for real-time metrics updates
"""
import json
import asyncio
from typing import Dict, Set
from fastapi import APIRouter, WebSocket, WebSocketDisconnect, Depends
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, and_, desc
from datetime import datetime, timedelta

from ..core.database import get_db
from ..models.models import ServerMetric, Server
import logging

logger = logging.getLogger(__name__)

router = APIRouter(tags=["websocket"])


class ConnectionManager:
    """Manages WebSocket connections"""

    def __init__(self):
        self.active_connections: Dict[int, Set[WebSocket]] = {}

    async def connect(self, websocket: WebSocket, user_id: int):
        """Connect a new WebSocket client"""
        await websocket.accept()
        if user_id not in self.active_connections:
            self.active_connections[user_id] = set()
        self.active_connections[user_id].add(websocket)
        logger.info(f"WebSocket connected for user {user_id}")

    def disconnect(self, websocket: WebSocket, user_id: int):
        """Disconnect a WebSocket client"""
        if user_id in self.active_connections:
            self.active_connections[user_id].discard(websocket)
            if not self.active_connections[user_id]:
                del self.active_connections[user_id]
        logger.info(f"WebSocket disconnected for user {user_id}")

    async def send_to_user(self, user_id: int, message: dict):
        """Send message to all connections for a user"""
        if user_id in self.active_connections:
            disconnected = set()
            for connection in self.active_connections[user_id]:
                try:
                    await connection.send_json(message)
                except Exception as e:
                    logger.error(f"Error sending to websocket: {e}")
                    disconnected.add(connection)

            # Remove disconnected connections
            for connection in disconnected:
                self.active_connections[user_id].discard(connection)

    async def broadcast(self, message: dict):
        """Broadcast message to all connected clients"""
        for user_id in list(self.active_connections.keys()):
            await self.send_to_user(user_id, message)


manager = ConnectionManager()


@router.websocket("/ws/metrics/{user_id}")
async def websocket_endpoint(websocket: WebSocket, user_id: int):
    """
    WebSocket endpoint for real-time metrics updates

    Client should send authentication token in first message:
    {"type": "auth", "token": "Bearer xxx"}

    Server sends updates:
    {"type": "metrics_update", "server_id": 1, "data": {...}}
    {"type": "alert", "alert_id": 1, "data": {...}}
    """
    await manager.connect(websocket, user_id)

    try:
        # Wait for authentication
        auth_message = await websocket.receive_json()

        if auth_message.get('type') != 'auth':
            await websocket.send_json({"type": "error", "message": "Authentication required"})
            await websocket.close()
            return

        # In production, verify the token here
        # For now, we'll accept any token

        await websocket.send_json({"type": "connected", "message": "WebSocket connected"})

        # Keep connection alive and handle incoming messages
        while True:
            try:
                # Receive message with timeout
                data = await asyncio.wait_for(
                    websocket.receive_json(),
                    timeout=30.0
                )

                # Handle different message types
                if data.get('type') == 'ping':
                    await websocket.send_json({"type": "pong"})

                elif data.get('type') == 'subscribe':
                    server_id = data.get('server_id')
                    await websocket.send_json({
                        "type": "subscribed",
                        "server_id": server_id
                    })

            except asyncio.TimeoutError:
                # Send ping to keep connection alive
                await websocket.send_json({"type": "ping"})

    except WebSocketDisconnect:
        manager.disconnect(websocket, user_id)
        logger.info(f"WebSocket disconnected for user {user_id}")

    except Exception as e:
        logger.error(f"WebSocket error: {e}")
        manager.disconnect(websocket, user_id)


async def broadcast_metric_update(user_id: int, server_id: int, metric_data: dict):
    """
    Broadcast metric update to connected clients

    Args:
        user_id: User ID
        server_id: Server ID
        metric_data: Metric data
    """
    message = {
        "type": "metrics_update",
        "server_id": server_id,
        "timestamp": datetime.utcnow().isoformat(),
        "data": metric_data
    }
    await manager.send_to_user(user_id, message)


async def broadcast_alert(user_id: int, alert_data: dict):
    """
    Broadcast alert to connected clients

    Args:
        user_id: User ID
        alert_data: Alert data
    """
    message = {
        "type": "alert",
        "timestamp": datetime.utcnow().isoformat(),
        "data": alert_data
    }
    await manager.send_to_user(user_id, message)

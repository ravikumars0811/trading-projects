"""
Alert Service

Sends notifications via multiple channels:
- Email (SendGrid)
- Telegram
- Slack
"""

import logging
from typing import Optional, List
from datetime import datetime
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
import asyncio

from app.models.alert_config import AlertConfig, AlertChannel, AlertType
from app.models.incident import Incident
from app.models.endpoint import Endpoint
from app.core.config import settings

logger = logging.getLogger(__name__)


class AlertService:
    """Service for sending alerts through various channels"""

    def __init__(self):
        self.sendgrid_enabled = bool(settings.SENDGRID_API_KEY)
        self.telegram_enabled = bool(settings.TELEGRAM_BOT_TOKEN)
        self.slack_enabled = bool(settings.SLACK_WEBHOOK_URL)

    async def send_incident_alert(
        self,
        incident: Incident,
        endpoint: Endpoint,
        db: AsyncSession
    ):
        """
        Send alert for an incident

        Args:
            incident: Incident to alert on
            endpoint: Endpoint associated with the incident
            db: Database session
        """
        # Get user's alert configurations
        result = await db.execute(
            select(AlertConfig)
            .where(
                AlertConfig.user_id == endpoint.user_id,
                AlertConfig.is_active == True
            )
        )
        alert_configs = result.scalars().all()

        if not alert_configs:
            logger.info(f"No active alert configs for user {endpoint.user_id}")
            return

        # Prepare alert message
        message = self._format_alert_message(incident, endpoint)

        # Send alerts through configured channels
        for config in alert_configs:
            # Check if this alert type is enabled
            if incident.incident_type.value not in config.alert_types:
                continue

            try:
                if config.channel == AlertChannel.EMAIL and config.email:
                    await self._send_email_alert(config.email, message, incident, endpoint)

                elif config.channel == AlertChannel.TELEGRAM and config.telegram_chat_id:
                    await self._send_telegram_alert(config.telegram_chat_id, message)

                elif config.channel == AlertChannel.SLACK and config.slack_webhook_url:
                    await self._send_slack_alert(config.slack_webhook_url, message, incident, endpoint)

            except Exception as e:
                logger.error(f"Failed to send alert via {config.channel}: {e}")

        # Mark incident as alerted
        incident.alert_sent = True
        incident.alert_sent_at = datetime.utcnow()
        await db.commit()

    def _format_alert_message(self, incident: Incident, endpoint: Endpoint) -> str:
        """Format alert message"""
        severity_emoji = {
            AlertType.DOWNTIME: "🔴",
            AlertType.SLOW_RESPONSE: "🟡",
            AlertType.SSL_EXPIRY: "⚠️",
            AlertType.ANOMALY: "🔔",
        }

        emoji = severity_emoji.get(incident.incident_type, "⚠️")

        message = f"""
{emoji} **{incident.title}**

**Endpoint:** {endpoint.name}
**URL:** {endpoint.url}
**Type:** {incident.incident_type.value.replace('_', ' ').title()}
**Status:** {incident.status.value.title()}
**Started:** {incident.started_at.strftime('%Y-%m-%d %H:%M:%S UTC')}

**Details:**
{incident.description}

---
_API Uptime Monitor Alert_
        """.strip()

        return message

    async def _send_email_alert(
        self,
        to_email: str,
        message: str,
        incident: Incident,
        endpoint: Endpoint
    ):
        """Send email alert via SendGrid"""
        if not self.sendgrid_enabled:
            logger.warning("SendGrid not configured")
            return

        try:
            from sendgrid import SendGridAPIClient
            from sendgrid.helpers.mail import Mail, Email, To, Content

            subject = f"Alert: {incident.title}"

            # Convert markdown to HTML (simple conversion)
            html_message = message.replace('\n', '<br>').replace('**', '<strong>').replace('**', '</strong>')

            mail = Mail(
                from_email=Email(settings.FROM_EMAIL),
                to_emails=To(to_email),
                subject=subject,
                html_content=Content("text/html", html_message)
            )

            sg = SendGridAPIClient(settings.SENDGRID_API_KEY)
            response = sg.send(mail)

            logger.info(f"Email alert sent to {to_email}: {response.status_code}")

        except Exception as e:
            logger.error(f"Failed to send email alert: {e}")
            raise

    async def _send_telegram_alert(self, chat_id: str, message: str):
        """Send Telegram alert"""
        if not self.telegram_enabled:
            logger.warning("Telegram not configured")
            return

        try:
            from telegram import Bot

            bot = Bot(token=settings.TELEGRAM_BOT_TOKEN)
            await bot.send_message(
                chat_id=chat_id,
                text=message,
                parse_mode='Markdown'
            )

            logger.info(f"Telegram alert sent to {chat_id}")

        except Exception as e:
            logger.error(f"Failed to send Telegram alert: {e}")
            raise

    async def _send_slack_alert(
        self,
        webhook_url: str,
        message: str,
        incident: Incident,
        endpoint: Endpoint
    ):
        """Send Slack alert via webhook"""
        try:
            import httpx

            # Format for Slack blocks
            color_map = {
                AlertType.DOWNTIME: "danger",
                AlertType.SLOW_RESPONSE: "warning",
                AlertType.SSL_EXPIRY: "warning",
                AlertType.ANOMALY: "#0066cc",
            }

            payload = {
                "attachments": [
                    {
                        "color": color_map.get(incident.incident_type, "warning"),
                        "title": incident.title,
                        "fields": [
                            {
                                "title": "Endpoint",
                                "value": endpoint.name,
                                "short": True
                            },
                            {
                                "title": "URL",
                                "value": endpoint.url,
                                "short": True
                            },
                            {
                                "title": "Type",
                                "value": incident.incident_type.value.replace('_', ' ').title(),
                                "short": True
                            },
                            {
                                "title": "Status",
                                "value": incident.status.value.title(),
                                "short": True
                            },
                            {
                                "title": "Details",
                                "value": incident.description,
                                "short": False
                            }
                        ],
                        "footer": "API Uptime Monitor",
                        "ts": int(incident.started_at.timestamp())
                    }
                ]
            }

            async with httpx.AsyncClient() as client:
                response = await client.post(webhook_url, json=payload)
                response.raise_for_status()

            logger.info(f"Slack alert sent: {response.status_code}")

        except Exception as e:
            logger.error(f"Failed to send Slack alert: {e}")
            raise

    async def test_alert_channel(
        self,
        channel: AlertChannel,
        config: AlertConfig
    ) -> bool:
        """
        Test an alert channel configuration

        Args:
            channel: Alert channel to test
            config: Alert configuration

        Returns:
            True if test successful
        """
        test_message = "🔔 **Test Alert**\n\nThis is a test alert from API Uptime Monitor.\nYour alert channel is configured correctly!"

        try:
            if channel == AlertChannel.EMAIL and config.email:
                # Create a dummy incident for test
                from app.models.incident import Incident
                from app.models.endpoint import Endpoint

                test_incident = Incident(
                    title="Test Alert",
                    description="This is a test",
                    incident_type=AlertType.DOWNTIME
                )
                test_endpoint = Endpoint(
                    name="Test Endpoint",
                    url="https://example.com"
                )

                await self._send_email_alert(config.email, test_message, test_incident, test_endpoint)

            elif channel == AlertChannel.TELEGRAM and config.telegram_chat_id:
                await self._send_telegram_alert(config.telegram_chat_id, test_message)

            elif channel == AlertChannel.SLACK and config.slack_webhook_url:
                from app.models.incident import Incident
                from app.models.endpoint import Endpoint

                test_incident = Incident(
                    title="Test Alert",
                    description="This is a test",
                    incident_type=AlertType.DOWNTIME
                )
                test_endpoint = Endpoint(
                    name="Test Endpoint",
                    url="https://example.com"
                )

                await self._send_slack_alert(
                    config.slack_webhook_url,
                    test_message,
                    test_incident,
                    test_endpoint
                )

            return True

        except Exception as e:
            logger.error(f"Alert channel test failed: {e}")
            return False


class AlertScheduler:
    """
    Scheduler for checking incidents and sending alerts
    """

    def __init__(self, db_session_factory):
        self.db_session_factory = db_session_factory
        self.alert_service = AlertService()

    async def check_and_send_alerts(self):
        """
        Check for incidents that need alerts and send them
        """
        async with self.db_session_factory() as db:
            # Find incidents that haven't been alerted yet
            result = await db.execute(
                select(Incident)
                .where(Incident.alert_sent == False)
                .order_by(Incident.started_at.desc())
            )
            incidents = result.scalars().all()

            for incident in incidents:
                # Get endpoint
                endpoint_result = await db.execute(
                    select(Endpoint).where(Endpoint.id == incident.endpoint_id)
                )
                endpoint = endpoint_result.scalar_one_or_none()

                if endpoint:
                    await self.alert_service.send_incident_alert(incident, endpoint, db)

                await asyncio.sleep(0.1)  # Rate limiting

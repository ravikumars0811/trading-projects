"""
Health Check Monitoring Service

Performs HTTP health checks on endpoints and records results
"""

import asyncio
import httpx
import ssl
from datetime import datetime, timedelta
from typing import Dict, Optional, Tuple
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
import logging

from app.models.endpoint import Endpoint
from app.models.health_check import HealthCheck
from app.models.incident import Incident, IncidentStatus
from app.models.alert_config import AlertType
from app.ml.anomaly_detector import AnomalyDetector

logger = logging.getLogger(__name__)


class HealthCheckService:
    """Service for performing health checks on endpoints"""

    def __init__(self):
        self.anomaly_detectors: Dict[int, AnomalyDetector] = {}

    async def perform_health_check(
        self,
        endpoint: Endpoint,
        db: AsyncSession
    ) -> HealthCheck:
        """
        Perform a health check on an endpoint

        Args:
            endpoint: Endpoint to check
            db: Database session

        Returns:
            HealthCheck result
        """
        health_check = HealthCheck(endpoint_id=endpoint.id)

        try:
            # Prepare request
            headers = endpoint.headers or {}
            timeout = httpx.Timeout(endpoint.timeout, connect=10.0)

            # Perform HTTP request
            start_time = datetime.utcnow()

            async with httpx.AsyncClient(
                verify=endpoint.verify_ssl,
                follow_redirects=endpoint.follow_redirects,
                timeout=timeout
            ) as client:
                response = await client.request(
                    method=endpoint.method.value,
                    url=endpoint.url,
                    headers=headers,
                    json=endpoint.body if endpoint.body else None,
                )

            end_time = datetime.utcnow()

            # Calculate response time in milliseconds
            response_time = (end_time - start_time).total_seconds() * 1000

            # Record results
            health_check.status_code = response.status_code
            health_check.response_time = response_time
            health_check.is_success = response.status_code in endpoint.expected_status_codes

            # Check SSL certificate if enabled
            if endpoint.check_ssl_expiry and endpoint.url.startswith('https'):
                ssl_info = await self._check_ssl_certificate(endpoint.url)
                if ssl_info:
                    health_check.ssl_expiry_date = ssl_info['expiry_date']
                    health_check.ssl_days_remaining = ssl_info['days_remaining']

            # Perform anomaly detection
            if endpoint.id not in self.anomaly_detectors:
                await self._init_anomaly_detector(endpoint.id, db)

            detector = self.anomaly_detectors.get(endpoint.id)
            if detector and detector.is_trained:
                check_data = {
                    'response_time': response_time,
                    'status_code': response.status_code,
                    'checked_at': datetime.utcnow(),
                    'is_success': health_check.is_success,
                }
                is_anomaly, anomaly_score = detector.predict(check_data)
                health_check.is_anomaly = is_anomaly
                health_check.anomaly_score = anomaly_score

        except httpx.TimeoutException:
            health_check.is_success = False
            health_check.error_message = "Request timeout"
            logger.warning(f"Timeout checking endpoint {endpoint.url}")

        except httpx.ConnectError as e:
            health_check.is_success = False
            health_check.error_message = f"Connection error: {str(e)}"
            logger.error(f"Connection error for {endpoint.url}: {e}")

        except Exception as e:
            health_check.is_success = False
            health_check.error_message = f"Error: {str(e)}"
            logger.error(f"Error checking endpoint {endpoint.url}: {e}")

        # Save health check
        db.add(health_check)
        await db.commit()
        await db.refresh(health_check)

        # Update endpoint last checked time
        endpoint.last_checked_at = datetime.utcnow()
        await db.commit()

        # Check if we need to create/update incidents
        await self._handle_incidents(endpoint, health_check, db)

        return health_check

    async def _check_ssl_certificate(self, url: str) -> Optional[Dict]:
        """
        Check SSL certificate expiry

        Args:
            url: HTTPS URL to check

        Returns:
            Dict with expiry_date and days_remaining, or None if check fails
        """
        try:
            import socket
            from urllib.parse import urlparse

            parsed = urlparse(url)
            hostname = parsed.hostname
            port = parsed.port or 443

            context = ssl.create_default_context()
            with socket.create_connection((hostname, port), timeout=10) as sock:
                with context.wrap_socket(sock, server_hostname=hostname) as ssock:
                    cert = ssock.getpeercert()

            # Parse expiry date
            expiry_str = cert['notAfter']
            expiry_date = datetime.strptime(expiry_str, '%b %d %H:%M:%S %Y %Z')

            # Calculate days remaining
            days_remaining = (expiry_date - datetime.utcnow()).days

            return {
                'expiry_date': expiry_date,
                'days_remaining': days_remaining,
            }

        except Exception as e:
            logger.error(f"SSL check failed for {url}: {e}")
            return None

    async def _init_anomaly_detector(self, endpoint_id: int, db: AsyncSession):
        """Initialize and train anomaly detector for an endpoint"""
        detector = AnomalyDetector(contamination=0.1)

        # Try to load existing model
        if detector.load_model(endpoint_id):
            self.anomaly_detectors[endpoint_id] = detector
            return

        # Train on historical data
        result = await db.execute(
            select(HealthCheck)
            .where(HealthCheck.endpoint_id == endpoint_id)
            .order_by(HealthCheck.checked_at.desc())
            .limit(500)
        )
        health_checks = result.scalars().all()

        if len(health_checks) >= 50:
            check_data = [
                {
                    'response_time': hc.response_time,
                    'status_code': hc.status_code,
                    'checked_at': hc.checked_at,
                    'is_success': hc.is_success,
                }
                for hc in health_checks
            ]

            if detector.train(check_data):
                detector.save_model(endpoint_id)
                self.anomaly_detectors[endpoint_id] = detector

    async def _handle_incidents(
        self,
        endpoint: Endpoint,
        health_check: HealthCheck,
        db: AsyncSession
    ):
        """
        Create or resolve incidents based on health check results

        Args:
            endpoint: Endpoint that was checked
            health_check: Health check result
            db: Database session
        """
        # Check for open incidents
        result = await db.execute(
            select(Incident)
            .where(
                Incident.endpoint_id == endpoint.id,
                Incident.status == IncidentStatus.OPEN
            )
            .order_by(Incident.started_at.desc())
        )
        open_incident = result.scalar_one_or_none()

        # Handle downtime
        if not health_check.is_success:
            if not open_incident:
                # Create new downtime incident
                incident = Incident(
                    endpoint_id=endpoint.id,
                    incident_type=AlertType.DOWNTIME,
                    status=IncidentStatus.OPEN,
                    title=f"Endpoint {endpoint.name} is down",
                    description=f"Error: {health_check.error_message or 'Unknown error'}",
                )
                db.add(incident)
                await db.commit()

        else:
            # Endpoint is up - resolve downtime incidents
            if open_incident and open_incident.incident_type == AlertType.DOWNTIME:
                open_incident.status = IncidentStatus.RESOLVED
                open_incident.resolved_at = datetime.utcnow()
                await db.commit()

        # Handle slow response
        if (health_check.is_success and
            health_check.response_time and
            health_check.response_time > endpoint.response_time_threshold):

            # Check if there's already a slow response incident
            result = await db.execute(
                select(Incident)
                .where(
                    Incident.endpoint_id == endpoint.id,
                    Incident.incident_type == AlertType.SLOW_RESPONSE,
                    Incident.status == IncidentStatus.OPEN
                )
            )
            slow_incident = result.scalar_one_or_none()

            if not slow_incident:
                incident = Incident(
                    endpoint_id=endpoint.id,
                    incident_type=AlertType.SLOW_RESPONSE,
                    status=IncidentStatus.OPEN,
                    title=f"Endpoint {endpoint.name} is responding slowly",
                    description=f"Response time: {health_check.response_time:.2f}ms (threshold: {endpoint.response_time_threshold}ms)",
                )
                db.add(incident)
                await db.commit()

        # Handle SSL expiry warning
        if (health_check.ssl_days_remaining is not None and
            health_check.ssl_days_remaining <= endpoint.ssl_expiry_alert_days):

            result = await db.execute(
                select(Incident)
                .where(
                    Incident.endpoint_id == endpoint.id,
                    Incident.incident_type == AlertType.SSL_EXPIRY,
                    Incident.status == IncidentStatus.OPEN
                )
            )
            ssl_incident = result.scalar_one_or_none()

            if not ssl_incident:
                incident = Incident(
                    endpoint_id=endpoint.id,
                    incident_type=AlertType.SSL_EXPIRY,
                    status=IncidentStatus.OPEN,
                    title=f"SSL certificate expiring soon for {endpoint.name}",
                    description=f"Certificate expires in {health_check.ssl_days_remaining} days",
                )
                db.add(incident)
                await db.commit()

        # Handle anomalies
        if health_check.is_anomaly:
            result = await db.execute(
                select(Incident)
                .where(
                    Incident.endpoint_id == endpoint.id,
                    Incident.incident_type == AlertType.ANOMALY,
                    Incident.status == IncidentStatus.OPEN
                )
            )
            anomaly_incident = result.scalar_one_or_none()

            if not anomaly_incident:
                incident = Incident(
                    endpoint_id=endpoint.id,
                    incident_type=AlertType.ANOMALY,
                    status=IncidentStatus.OPEN,
                    title=f"Anomaly detected for {endpoint.name}",
                    description=f"Anomaly score: {health_check.anomaly_score:.4f}",
                )
                db.add(incident)
                await db.commit()

    async def get_endpoint_stats(
        self,
        endpoint_id: int,
        period_hours: int,
        db: AsyncSession
    ) -> Dict:
        """
        Get statistics for an endpoint over a time period

        Args:
            endpoint_id: Endpoint ID
            period_hours: Time period in hours
            db: Database session

        Returns:
            Dictionary with statistics
        """
        cutoff_time = datetime.utcnow() - timedelta(hours=period_hours)

        result = await db.execute(
            select(HealthCheck)
            .where(
                HealthCheck.endpoint_id == endpoint_id,
                HealthCheck.checked_at >= cutoff_time
            )
            .order_by(HealthCheck.checked_at.asc())
        )
        health_checks = result.scalars().all()

        if not health_checks:
            return {
                'uptime_percentage': 0,
                'avg_response_time': 0,
                'total_checks': 0,
                'failed_checks': 0,
                'current_status': 'unknown',
            }

        total_checks = len(health_checks)
        successful_checks = sum(1 for hc in health_checks if hc.is_success)
        failed_checks = total_checks - successful_checks
        uptime_percentage = (successful_checks / total_checks) * 100

        response_times = [
            hc.response_time for hc in health_checks
            if hc.response_time is not None
        ]
        avg_response_time = sum(response_times) / len(response_times) if response_times else 0

        # Determine current status
        recent_checks = health_checks[-5:]  # Last 5 checks
        recent_failures = sum(1 for hc in recent_checks if not hc.is_success)

        if recent_failures >= 3:
            current_status = 'down'
        elif recent_failures > 0:
            current_status = 'degraded'
        else:
            current_status = 'up'

        return {
            'uptime_percentage': uptime_percentage,
            'avg_response_time': avg_response_time,
            'total_checks': total_checks,
            'failed_checks': failed_checks,
            'current_status': current_status,
            'last_check': health_checks[-1].checked_at if health_checks else None,
        }

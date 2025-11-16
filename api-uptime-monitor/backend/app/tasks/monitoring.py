"""
Celery tasks for monitoring endpoints
"""

from celery import Task
from sqlalchemy import select, delete
from datetime import datetime, timedelta
import asyncio
import logging

from app.core.celery_app import celery_app
from app.core.database import AsyncSessionLocal
from app.models.endpoint import Endpoint
from app.models.health_check import HealthCheck
from app.services.monitoring import HealthCheckService
from app.services.alerts import AlertScheduler
from app.ml.anomaly_detector import AnomalyDetector

logger = logging.getLogger(__name__)


class AsyncTask(Task):
    """Base task that handles async operations"""

    def __call__(self, *args, **kwargs):
        loop = asyncio.get_event_loop()
        return loop.run_until_complete(self.run(*args, **kwargs))


@celery_app.task(name='app.tasks.monitoring.check_all_endpoints', base=AsyncTask)
async def check_all_endpoints():
    """
    Check all active endpoints that are due for a check
    """
    async with AsyncSessionLocal() as db:
        try:
            # Get all active endpoints
            result = await db.execute(
                select(Endpoint).where(Endpoint.is_active == True)
            )
            endpoints = result.scalars().all()

            logger.info(f"Checking {len(endpoints)} active endpoints")

            health_service = HealthCheckService()
            alert_scheduler = AlertScheduler(AsyncSessionLocal)

            for endpoint in endpoints:
                try:
                    # Check if it's time to check this endpoint
                    if endpoint.last_checked_at:
                        time_since_last_check = (
                            datetime.utcnow() - endpoint.last_checked_at
                        ).total_seconds()

                        if time_since_last_check < endpoint.check_interval:
                            continue  # Not time yet

                    # Perform health check
                    logger.info(f"Checking endpoint: {endpoint.name} ({endpoint.url})")
                    await health_service.perform_health_check(endpoint, db)

                except Exception as e:
                    logger.error(f"Error checking endpoint {endpoint.id}: {e}")

            # Check and send alerts
            await alert_scheduler.check_and_send_alerts()

        except Exception as e:
            logger.error(f"Error in check_all_endpoints task: {e}")
            raise


@celery_app.task(name='app.tasks.monitoring.cleanup_old_health_checks')
def cleanup_old_health_checks():
    """
    Clean up health check records older than 90 days
    """
    async def _cleanup():
        async with AsyncSessionLocal() as db:
            try:
                cutoff_date = datetime.utcnow() - timedelta(days=90)

                result = await db.execute(
                    delete(HealthCheck).where(
                        HealthCheck.checked_at < cutoff_date
                    )
                )

                deleted_count = result.rowcount
                await db.commit()

                logger.info(f"Cleaned up {deleted_count} old health check records")

            except Exception as e:
                logger.error(f"Error in cleanup task: {e}")
                raise

    loop = asyncio.get_event_loop()
    loop.run_until_complete(_cleanup())


@celery_app.task(name='app.tasks.monitoring.retrain_ml_models')
def retrain_ml_models():
    """
    Retrain ML models for all endpoints with sufficient data
    """
    async def _retrain():
        async with AsyncSessionLocal() as db:
            try:
                # Get all endpoints
                result = await db.execute(select(Endpoint))
                endpoints = result.scalars().all()

                for endpoint in endpoints:
                    try:
                        # Get historical data
                        health_check_result = await db.execute(
                            select(HealthCheck)
                            .where(HealthCheck.endpoint_id == endpoint.id)
                            .order_by(HealthCheck.checked_at.desc())
                            .limit(1000)
                        )
                        health_checks = health_check_result.scalars().all()

                        if len(health_checks) >= 50:
                            # Train model
                            detector = AnomalyDetector(contamination=0.1)

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
                                detector.save_model(endpoint.id)
                                logger.info(f"Retrained model for endpoint {endpoint.id}")

                    except Exception as e:
                        logger.error(f"Error retraining model for endpoint {endpoint.id}: {e}")

            except Exception as e:
                logger.error(f"Error in retrain_ml_models task: {e}")
                raise

    loop = asyncio.get_event_loop()
    loop.run_until_complete(_retrain())


@celery_app.task(name='app.tasks.monitoring.check_single_endpoint', base=AsyncTask)
async def check_single_endpoint(endpoint_id: int):
    """
    Check a single endpoint immediately

    Args:
        endpoint_id: ID of the endpoint to check
    """
    async with AsyncSessionLocal() as db:
        try:
            result = await db.execute(
                select(Endpoint).where(Endpoint.id == endpoint_id)
            )
            endpoint = result.scalar_one_or_none()

            if not endpoint:
                logger.error(f"Endpoint {endpoint_id} not found")
                return

            health_service = HealthCheckService()
            await health_service.perform_health_check(endpoint, db)

            logger.info(f"Checked endpoint {endpoint_id}")

        except Exception as e:
            logger.error(f"Error checking endpoint {endpoint_id}: {e}")
            raise

"""
Celery configuration for background tasks
"""

from celery import Celery
from celery.schedules import crontab
from app.core.config import settings

celery_app = Celery(
    "uptime_monitor",
    broker=settings.REDIS_URL,
    backend=settings.REDIS_URL,
    include=['app.tasks.monitoring']
)

celery_app.conf.update(
    task_serializer='json',
    accept_content=['json'],
    result_serializer='json',
    timezone='UTC',
    enable_utc=True,
    task_track_started=True,
    task_time_limit=30 * 60,  # 30 minutes
    task_soft_time_limit=25 * 60,  # 25 minutes
    worker_prefetch_multiplier=4,
    worker_max_tasks_per_child=1000,
)

# Beat schedule for periodic tasks
celery_app.conf.beat_schedule = {
    'check-endpoints-every-minute': {
        'task': 'app.tasks.monitoring.check_all_endpoints',
        'schedule': 60.0,  # Every minute
    },
    'cleanup-old-health-checks': {
        'task': 'app.tasks.monitoring.cleanup_old_health_checks',
        'schedule': crontab(hour=2, minute=0),  # 2 AM daily
    },
    'retrain-ml-models': {
        'task': 'app.tasks.monitoring.retrain_ml_models',
        'schedule': crontab(hour=3, minute=0),  # 3 AM daily
    },
}

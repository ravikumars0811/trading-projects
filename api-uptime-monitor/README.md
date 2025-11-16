# API Uptime Monitor - Production-Ready SaaS Platform

A comprehensive API uptime monitoring solution with **AI/ML anomaly detection**, real-time alerts, and beautiful dashboards.

## Features

### Core Features
- ✅ **API Health Monitoring** - Monitor any HTTP endpoint with customizable intervals
- ✅ **Real-time Dashboard** - Beautiful Next.js dashboard with live updates
- ✅ **Multi-channel Alerts** - Email, Telegram, and Slack notifications
- ✅ **SSL Certificate Monitoring** - Track SSL expiry and get alerts
- ✅ **Response Time Tracking** - Monitor API performance with detailed metrics

### AI/ML Features
- 🤖 **Anomaly Detection** - Machine learning-based anomaly detection using Isolation Forest
- 📊 **Pattern Recognition** - Identify recurring outage patterns
- 🔮 **Predictive Maintenance** - Predict potential downtime before it happens
- 🎯 **Smart Thresholds** - Auto-adjust thresholds based on historical data
- 🧠 **Intelligent Alerting** - Reduce alert fatigue with smart alert grouping

### Technical Features
- 🚀 **Scalable Architecture** - Built with FastAPI, Celery, and Redis
- 📈 **Time-series Optimization** - PostgreSQL with TimescaleDB for efficient queries
- 🔒 **Secure Authentication** - JWT-based auth with bcrypt password hashing
- 🐳 **Docker Ready** - Complete Docker Compose setup
- 📊 **Metrics & Monitoring** - Built-in Flower dashboard for Celery monitoring

## Tech Stack

### Backend
- **Framework**: FastAPI (Python 3.11)
- **Database**: PostgreSQL 15 with TimescaleDB extension
- **Task Queue**: Celery with Redis
- **AI/ML**: scikit-learn (Isolation Forest)
- **Notifications**: SendGrid, python-telegram-bot, Slack SDK

### Frontend
- **Framework**: Next.js 14 (React 18)
- **Language**: TypeScript
- **Styling**: Tailwind CSS
- **Charts**: Recharts
- **State Management**: Zustand
- **Data Fetching**: SWR + Axios

### DevOps
- **Containerization**: Docker & Docker Compose
- **Database**: PostgreSQL with TimescaleDB
- **Cache/Queue**: Redis
- **Monitoring**: Flower (Celery monitoring)

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         Frontend (Next.js)                   │
│                     Port 3000 - React Dashboard              │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                     Backend API (FastAPI)                    │
│                         Port 8000                            │
└───────────┬─────────────────────────────────┬───────────────┘
            │                                 │
            ▼                                 ▼
┌───────────────────────┐         ┌────────────────────────┐
│  PostgreSQL + TimescaleDB │         │   Redis (Cache/Queue)   │
│      Port 5432        │         │      Port 6379         │
└───────────────────────┘         └────────────────────────┘
                                              │
                                              ▼
                            ┌─────────────────────────────────┐
                            │    Celery Workers & Beat        │
                            │  (Health Checks + ML Training)  │
                            └─────────────────────────────────┘
                                              │
                                              ▼
                            ┌─────────────────────────────────┐
                            │    Alert Services (Multi-channel)│
                            │   Email, Telegram, Slack        │
                            └─────────────────────────────────┘
```

## Quick Start

### Prerequisites
- Docker & Docker Compose
- Node.js 20+ (for local frontend development)
- Python 3.11+ (for local backend development)

### 1. Clone and Setup

```bash
cd api-uptime-monitor
cp backend/.env.example backend/.env
cp frontend/.env.example frontend/.env
```

### 2. Configure Environment Variables

Edit `backend/.env`:
```env
# Database
DATABASE_URL=postgresql+asyncpg://uptime_user:uptime_pass@postgres:5432/uptime_monitor
POSTGRES_USER=uptime_user
POSTGRES_PASSWORD=uptime_pass
POSTGRES_DB=uptime_monitor

# Redis
REDIS_URL=redis://redis:6379/0

# JWT Security
SECRET_KEY=your-super-secret-key-change-this-in-production
ALGORITHM=HS256
ACCESS_TOKEN_EXPIRE_MINUTES=30

# Email (SendGrid) - Optional
SENDGRID_API_KEY=your-sendgrid-api-key
FROM_EMAIL=alerts@yourdomain.com

# Telegram - Optional
TELEGRAM_BOT_TOKEN=your-telegram-bot-token

# Slack - Optional
SLACK_WEBHOOK_URL=your-slack-webhook-url
```

### 3. Start Services with Docker Compose

```bash
docker-compose up -d
```

This will start:
- **Backend API** - http://localhost:8000
- **Frontend Dashboard** - http://localhost:3000
- **API Docs** - http://localhost:8000/api/docs
- **Flower (Celery Monitoring)** - http://localhost:5555
- **PostgreSQL** - localhost:5432
- **Redis** - localhost:6379

### 4. Access the Application

1. Open http://localhost:3000
2. Register a new account
3. Add your first endpoint to monitor
4. Configure alerts (optional)

## Development Setup

### Backend Development

```bash
cd backend

# Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Run database migrations (if using Alembic)
# alembic upgrade head

# Start FastAPI server
uvicorn app.main:app --reload --port 8000

# Start Celery worker (in another terminal)
celery -A app.core.celery_app worker --loglevel=info

# Start Celery beat scheduler (in another terminal)
celery -A app.core.celery_app beat --loglevel=info
```

### Frontend Development

```bash
cd frontend

# Install dependencies
npm install

# Start development server
npm run dev
```

Frontend will be available at http://localhost:3000

## Usage Guide

### 1. Adding an Endpoint

```json
{
  "name": "My API",
  "url": "https://api.example.com/health",
  "method": "GET",
  "check_interval": 300,
  "timeout": 30,
  "expected_status_codes": [200],
  "response_time_threshold": 5000,
  "verify_ssl": true,
  "check_ssl_expiry": true,
  "ssl_expiry_alert_days": 7
}
```

### 2. Setting Up Alerts

**Email Alerts (via SendGrid):**
```json
{
  "channel": "email",
  "email": "your@email.com",
  "alert_types": ["downtime", "slow_response", "ssl_expiry"],
  "is_active": true,
  "cooldown_minutes": 5
}
```

**Telegram Alerts:**
1. Create a bot with @BotFather on Telegram
2. Get your bot token
3. Start a chat with your bot
4. Get your chat ID: https://api.telegram.org/bot<YOUR_TOKEN>/getUpdates
5. Configure:
```json
{
  "channel": "telegram",
  "telegram_chat_id": "your-chat-id",
  "alert_types": ["downtime", "slow_response"]
}
```

**Slack Alerts:**
1. Create an incoming webhook in your Slack workspace
2. Configure:
```json
{
  "channel": "slack",
  "slack_webhook_url": "https://hooks.slack.com/services/YOUR/WEBHOOK/URL",
  "alert_types": ["downtime", "anomaly"]
}
```

### 3. Viewing Analytics

The dashboard provides:
- **24h/7d/30d statistics** - Uptime percentage, response times, failure counts
- **Real-time graphs** - Response time trends, uptime charts
- **Anomaly detection** - ML-identified unusual patterns
- **Incident history** - Track all incidents with timestamps

## API Documentation

### Authentication

All API endpoints (except `/auth/register` and `/auth/login`) require JWT authentication.

**Login:**
```bash
curl -X POST http://localhost:8000/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username": "your_username", "password": "your_password"}'
```

**Use Token:**
```bash
curl -X GET http://localhost:8000/api/v1/endpoints \
  -H "Authorization: Bearer YOUR_TOKEN"
```

### Key Endpoints

- `POST /api/v1/auth/register` - Register new user
- `POST /api/v1/auth/login` - Login and get token
- `GET /api/v1/endpoints` - List all endpoints
- `POST /api/v1/endpoints` - Create endpoint
- `GET /api/v1/endpoints/{id}/stats` - Get endpoint statistics
- `POST /api/v1/endpoints/{id}/check` - Trigger immediate check
- `GET /api/v1/health-checks/endpoint/{id}` - Get health check history
- `POST /api/v1/alerts` - Create alert configuration

Full API documentation: http://localhost:8000/api/docs

## AI/ML Features Explained

### Anomaly Detection

The system uses **Isolation Forest** algorithm to detect anomalies in:
- Response times
- Failure patterns
- SSL certificate issues
- Unusual traffic patterns

**How it works:**
1. Collects minimum 50 health checks per endpoint
2. Trains an Isolation Forest model on historical data
3. Continuously analyzes new checks for anomalies
4. Automatically retrains models daily at 3 AM

### Predictive Maintenance

Analyzes trends to predict potential downtime:
- Response time degradation patterns
- Increasing failure rates
- SSL certificate expiry timeline

### Smart Thresholds

Automatically adjusts response time thresholds based on:
- 95th percentile of historical response times
- Standard deviation analysis
- Time-of-day patterns

## Monitoring & Observability

### Celery Monitoring (Flower)

Access Flower at http://localhost:5555 to monitor:
- Active workers
- Task execution rates
- Failed tasks
- Queue lengths

### Database Queries

Monitor database performance:
```sql
-- Check endpoint health check counts
SELECT endpoint_id, COUNT(*) as check_count
FROM health_checks
GROUP BY endpoint_id;

-- Recent anomalies
SELECT * FROM health_checks
WHERE is_anomaly = true
ORDER BY checked_at DESC
LIMIT 10;
```

## Production Deployment

### Environment Variables for Production

```env
# Backend
ENVIRONMENT=production
DEBUG=false
SECRET_KEY=<generate-strong-random-key>

# Database (use managed PostgreSQL)
DATABASE_URL=postgresql+asyncpg://user:pass@db-host:5432/dbname

# Redis (use managed Redis)
REDIS_URL=redis://redis-host:6379/0

# Configure all alert channels
SENDGRID_API_KEY=<your-key>
FROM_EMAIL=alerts@yourdomain.com
TELEGRAM_BOT_TOKEN=<your-token>
SLACK_WEBHOOK_URL=<your-webhook>
```

### Security Checklist

- [ ] Change all default passwords
- [ ] Use strong SECRET_KEY (32+ characters)
- [ ] Enable SSL/TLS for all connections
- [ ] Configure CORS properly in backend
- [ ] Use environment-specific .env files
- [ ] Enable database backups
- [ ] Set up log aggregation
- [ ] Configure rate limiting
- [ ] Use managed database services

### Scaling

**Horizontal Scaling:**
- Add more Celery workers: `docker-compose up --scale celery-worker=4`
- Use load balancer for backend API
- Use CDN for frontend

**Database Optimization:**
- TimescaleDB hypertables for health_checks table
- Automated data retention policies
- Regular VACUUM and ANALYZE

## Troubleshooting

### Common Issues

**1. Celery tasks not running:**
```bash
# Check Celery worker logs
docker-compose logs celery-worker

# Check Redis connection
docker-compose exec redis redis-cli ping
```

**2. Database connection errors:**
```bash
# Check PostgreSQL logs
docker-compose logs postgres

# Test connection
docker-compose exec postgres psql -U uptime_user -d uptime_monitor
```

**3. Frontend can't connect to backend:**
- Check `NEXT_PUBLIC_API_URL` in frontend/.env
- Verify backend is running: `curl http://localhost:8000/health`

**4. Alerts not sending:**
- Verify SendGrid/Telegram/Slack credentials
- Check alert configuration in database
- Review alert service logs: `docker-compose logs backend`

## Contributing

We welcome contributions! Please see our contributing guidelines.

## License

MIT License - see LICENSE file for details

## Support

For issues and questions:
- GitHub Issues: [Create an issue]
- Documentation: [Full docs]
- Email: support@yourdomain.com

## Roadmap

- [ ] WebSocket support for real-time updates
- [ ] Mobile app (React Native)
- [ ] Advanced ML models (LSTM for time-series prediction)
- [ ] Multi-region monitoring
- [ ] Status page generator
- [ ] API response validation
- [ ] Custom metrics and dimensions
- [ ] Grafana integration
- [ ] Kubernetes deployment manifests

---

**Built with ❤️ using FastAPI, Next.js, and Machine Learning**

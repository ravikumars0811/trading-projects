# Watchdog360 - AI-Powered Server Performance Monitoring SaaS

<div align="center">

![Watchdog360](https://img.shields.io/badge/Watchdog360-Server%20Monitoring-blue?style=for-the-badge)
![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)
![Python](https://img.shields.io/badge/Python-3.11-blue?style=for-the-badge&logo=python)
![FastAPI](https://img.shields.io/badge/FastAPI-0.104-green?style=for-the-badge&logo=fastapi)
![React](https://img.shields.io/badge/React-18.2-blue?style=for-the-badge&logo=react)
![TypeScript](https://img.shields.io/badge/TypeScript-5.2-blue?style=for-the-badge&logo=typescript)

**A modern, AI-powered server performance monitoring SaaS platform with real-time metrics, anomaly detection, and predictive analytics.**

[Features](#features) • [Quick Start](#quick-start) • [Installation](#installation) • [Documentation](#documentation) • [Architecture](#architecture)

</div>

---

## 🌟 Features

### Core Monitoring Features (FR-SRV-2)
- **CPU Monitoring**: Real-time CPU usage, core count, frequency, and load averages
- **Memory Monitoring**: RAM and Swap usage tracking
- **Disk Monitoring**: Multi-disk usage, I/O operations, read/write statistics
- **Network Monitoring**: Bandwidth usage, packet statistics, errors tracking
- **Process Monitoring**: Top 10 processes by CPU and memory usage

### Agent Installation (FR-SRV-1)
- **One-Line Install**: Simple curl command installation
  ```bash
  curl -s https://watchdog360.com/install.sh | bash
  ```
- **Cross-Platform**: Support for Ubuntu, Debian, CentOS, RHEL, Amazon Linux
- **Auto-Start**: Systemd service with automatic restart
- **Secure**: Token-based authentication

### Metrics Push (FR-SRV-3)
- **Real-Time**: Metrics pushed every 10 seconds
- **Secure**: Bearer token authentication
- **Reliable**: Exponential backoff retry mechanism (3 retries: 2s, 4s, 8s)
- **Efficient**: Compressed JSON payloads

### Dashboard (FR-SRV-4)
- **Real-Time Graphs**: Interactive CPU, Memory, and Disk usage charts
- **Process Table**: Top processes with CPU/Memory breakdown
- **Alert System**: Visual alerts for high CPU/RAM usage
- **Server Overview**: Multi-server monitoring from single dashboard

### AI/ML Features
- **Anomaly Detection**:
  - Isolation Forest algorithm for pattern detection
  - Statistical Z-score and IQR methods
  - Automatic anomaly flagging with confidence scores

- **Predictive Analytics**:
  - Prophet-based time series forecasting
  - CPU and Memory usage predictions
  - Trend analysis (increasing/decreasing/stable)
  - Proactive alerts for predicted issues

- **Smart Alerts**:
  - AI-predicted high resource usage
  - Anomaly-based alerts
  - Customizable thresholds
  - Alert deduplication

---

## 🚀 Quick Start

### Prerequisites
- Docker & Docker Compose
- Python 3.11+
- Node.js 18+
- PostgreSQL 15+ with TimescaleDB

### 1. Clone and Setup

```bash
git clone https://github.com/yourusername/watchdog360.git
cd watchdog360
```

### 2. Start with Docker Compose

```bash
cd deployment
cp .env.example .env
# Edit .env with your configuration
docker-compose up -d
```

### 3. Access Dashboard

- Dashboard: http://localhost:3000
- API: http://localhost:8000
- API Docs: http://localhost:8000/docs

### 4. Install Agent on Servers

```bash
# On your server to monitor
curl -s https://your-watchdog360-domain.com/install.sh | bash
```

---

## 📦 Installation

### Development Setup

#### Backend

```bash
cd backend

# Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Setup database
cp .env.example .env
# Edit .env with your database credentials

# Run migrations
alembic upgrade head

# Start server
uvicorn app.main:app --reload
```

#### Frontend

```bash
cd frontend

# Install dependencies
npm install

# Start development server
npm run dev
```

#### Agent

```bash
cd agent

# Install dependencies
pip install -r requirements.txt

# Create config
sudo mkdir -p /etc/watchdog360
sudo vim /etc/watchdog360/config.json

# Example config:
{
    "api_url": "http://localhost:8000",
    "token": "your-api-token",
    "interval": 10
}

# Run agent
sudo python watchdog_agent.py
```

---

## 🏗️ Architecture

### Tech Stack

#### Backend
- **Framework**: FastAPI (async Python web framework)
- **Database**: PostgreSQL + TimescaleDB (time-series optimization)
- **Cache**: Redis (real-time data & pub/sub)
- **AI/ML**:
  - scikit-learn (Isolation Forest)
  - Prophet (time series forecasting)
  - TensorFlow (deep learning models)
- **API**: RESTful + WebSocket for real-time updates

#### Frontend
- **Framework**: React 18 + TypeScript
- **Build Tool**: Vite
- **Styling**: Tailwind CSS
- **Charts**: Recharts
- **State**: Zustand
- **Real-time**: WebSocket client

#### Agent
- **Language**: Python 3.11
- **Monitoring**: psutil
- **HTTP**: requests with retry logic
- **Service**: systemd

### System Architecture

```
┌─────────────────┐
│   Monitored     │
│    Servers      │
│  (Agent runs)   │
└────────┬────────┘
         │ Metrics (10s interval)
         │ HTTPS + Token Auth
         ↓
┌─────────────────────────────────────┐
│         Load Balancer (Nginx)       │
└────────┬────────────────────────────┘
         │
         ↓
┌─────────────────────────────────────┐
│      FastAPI Backend (8000)         │
│  - REST API                         │
│  - WebSocket                        │
│  - AI/ML Processing                 │
└────┬───────────────┬────────────────┘
     │               │
     ↓               ↓
┌──────────┐   ┌─────────┐
│PostgreSQL│   │  Redis  │
│TimescaleDB│   │ Cache   │
└──────────┘   └─────────┘

┌─────────────────────────────────────┐
│     React Dashboard (3000)          │
│  - Real-time graphs                 │
│  - Alert management                 │
│  - Server overview                  │
└─────────────────────────────────────┘
```

### Database Schema

```sql
-- Users
users (id, email, hashed_password, ...)

-- API Tokens for agent authentication
api_tokens (id, token, user_id, ...)

-- Servers being monitored
servers (id, hostname, user_id, last_seen_at, ...)

-- Metrics (TimescaleDB hypertable)
server_metrics (
    id, server_id, timestamp,
    cpu_percent, memory_percent,
    disk_metrics (JSON),
    io_metrics, network_metrics,
    top_processes (JSON),
    anomaly_score, is_anomaly,
    predicted_cpu, predicted_memory
)

-- Alerts
alerts (
    id, server_id, alert_type, severity,
    title, message, is_resolved,
    is_prediction, ...
)
```

---

## 📊 AI/ML Features Explained

### 1. Anomaly Detection

Watchdog360 uses multiple algorithms to detect unusual behavior:

**Isolation Forest**
- Trains on historical data (last 24 hours)
- Identifies outliers based on feature isolation
- Provides anomaly score (0-1)
- Works well with high-dimensional data

**Statistical Methods**
- **Z-Score**: Detects values beyond 3 standard deviations
- **IQR**: Identifies outliers using interquartile range
- Complementary to ML-based detection

**Usage**:
```python
# Automatic in backend
is_anomaly, score, affected_metrics = detector.detect(current_metrics)
```

### 2. Predictive Analytics

Uses Prophet for time series forecasting:

**Features**:
- Predicts CPU/Memory usage 2 minutes ahead
- Seasonal pattern recognition
- Confidence intervals
- Trend analysis

**Alerts**:
- Proactive warnings before issues occur
- "CPU predicted to reach 90% in 2 minutes"
- Helps prevent downtime

### 3. Smart Alert System

**Threshold-Based**:
- CPU > 80%: Warning
- CPU > 90%: Critical
- Memory > 85%: Warning
- Disk > 90%: Warning

**AI-Based**:
- Anomaly detection alerts
- Predicted resource exhaustion
- Pattern-based notifications

**Deduplication**:
- Prevents alert spam
- 5-minute cooldown for similar alerts

---

## 🔧 Configuration

### Environment Variables

#### Backend (.env)
```env
# Database
DATABASE_URL=postgresql+asyncpg://user:pass@localhost/watchdog360
TIMESCALEDB_ENABLED=true

# Redis
REDIS_URL=redis://localhost:6379/0

# Security
SECRET_KEY=your-secret-key
ALGORITHM=HS256
ACCESS_TOKEN_EXPIRE_MINUTES=30

# AI/ML
ANOMALY_DETECTION_ENABLED=true
ANOMALY_DETECTION_THRESHOLD=0.85
PREDICTION_ENABLED=true

# Alerts
ALERT_HIGH_CPU_THRESHOLD=80
ALERT_HIGH_MEMORY_THRESHOLD=85
ALERT_HIGH_DISK_THRESHOLD=90
```

#### Agent (config.json)
```json
{
    "api_url": "https://api.watchdog360.com",
    "token": "your-api-token",
    "interval": 10,
    "hostname": "server-name"
}
```

---

## 📖 API Documentation

### Authentication

**Register**
```bash
POST /api/v1/auth/register
{
    "email": "user@example.com",
    "password": "secure-password",
    "full_name": "John Doe"
}
```

**Login**
```bash
POST /api/v1/auth/login
{
    "email": "user@example.com",
    "password": "secure-password"
}

Response:
{
    "access_token": "eyJ...",
    "token_type": "bearer"
}
```

### Metrics

**Push Metrics** (Agent endpoint)
```bash
POST /api/v1/metrics
Authorization: Bearer <token>
{
    "timestamp": "2024-01-01T12:00:00Z",
    "hostname": "server-1",
    "cpu": {...},
    "memory": {...},
    "disk": [...],
    ...
}
```

**Get Latest Metrics**
```bash
GET /api/v1/metrics/server/{server_id}/latest
Authorization: Bearer <token>
```

**Get Time Series**
```bash
GET /api/v1/metrics/server/{server_id}/timeseries?metric_name=cpu_percent&hours=24
Authorization: Bearer <token>
```

### Dashboard

**Get Stats**
```bash
GET /api/v1/dashboard/stats
Authorization: Bearer <token>

Response:
{
    "total_servers": 5,
    "active_servers": 4,
    "critical_alerts": 2,
    "warning_alerts": 3,
    "total_metrics_today": 12000
}
```

**Get Servers**
```bash
GET /api/v1/dashboard/servers
Authorization: Bearer <token>
```

**Get Alerts**
```bash
GET /api/v1/dashboard/alerts?server_id=1
Authorization: Bearer <token>
```

Full API documentation: http://localhost:8000/docs

---

## 🚢 Deployment

### Production Deployment with Docker

```bash
# 1. Clone repository
git clone https://github.com/yourusername/watchdog360.git
cd watchdog360/deployment

# 2. Configure environment
cp .env.example .env
vim .env  # Set production values

# 3. Start services
docker-compose up -d

# 4. Check status
docker-compose ps
docker-compose logs -f backend
```

### SSL Configuration

Update `deployment/nginx.conf` for HTTPS:

```nginx
server {
    listen 443 ssl http2;
    server_name watchdog360.com;

    ssl_certificate /etc/nginx/ssl/cert.pem;
    ssl_certificate_key /etc/nginx/ssl/key.pem;

    # ... rest of configuration
}
```

### Scaling

- **Horizontal**: Run multiple backend instances behind load balancer
- **Database**: Use PostgreSQL replication
- **Cache**: Redis Cluster for high availability
- **CDN**: Serve frontend through CDN

---

## 🧪 Testing

```bash
# Backend tests
cd backend
pytest

# Frontend tests
cd frontend
npm test

# Agent tests
cd agent
pytest test_agent.py
```

---

## 📝 License

MIT License - see [LICENSE](LICENSE) file

---

## 🤝 Contributing

Contributions welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md)

---

## 📞 Support

- Documentation: https://docs.watchdog360.com
- Issues: https://github.com/yourusername/watchdog360/issues
- Email: support@watchdog360.com

---

## 🎯 Roadmap

- [ ] Mobile app (iOS/Android)
- [ ] Kubernetes monitoring
- [ ] Docker container metrics
- [ ] Custom dashboards
- [ ] Slack/Teams integrations
- [ ] Advanced ML models (LSTM, Autoencoders)
- [ ] Multi-region support
- [ ] Cost optimization recommendations

---

<div align="center">

**Built with ❤️ using FastAPI, React, and AI/ML**

</div>

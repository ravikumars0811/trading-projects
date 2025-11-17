# Watchdog360 Installation Guide

## Table of Contents
1. [System Requirements](#system-requirements)
2. [Quick Install (Docker)](#quick-install-docker)
3. [Manual Installation](#manual-installation)
4. [Agent Installation](#agent-installation)
5. [Configuration](#configuration)
6. [Troubleshooting](#troubleshooting)

---

## System Requirements

### Server Requirements (Backend)
- **OS**: Linux (Ubuntu 20.04+, Debian 11+, CentOS 8+)
- **CPU**: 2+ cores
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 20GB+ (depends on retention period)
- **Software**:
  - Docker 20.10+ & Docker Compose
  - OR Python 3.11+, PostgreSQL 15+, Redis 7+

### Monitored Server Requirements (Agent)
- **OS**: Linux (Ubuntu, Debian, CentOS, RHEL, Amazon Linux)
- **Python**: 3.6+ (auto-installed by script)
- **RAM**: 50MB for agent
- **Network**: Outbound HTTPS to Watchdog360 server

### Client Requirements (Dashboard)
- Modern web browser (Chrome, Firefox, Safari, Edge)
- JavaScript enabled

---

## Quick Install (Docker)

### 1. Install Docker

**Ubuntu/Debian:**
```bash
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
sudo usermod -aG docker $USER
```

**CentOS/RHEL:**
```bash
sudo yum install -y docker docker-compose
sudo systemctl start docker
sudo systemctl enable docker
```

### 2. Clone Repository

```bash
git clone https://github.com/yourusername/watchdog360.git
cd watchdog360
```

### 3. Configure Environment

```bash
cd deployment
cp .env.example .env
```

Edit `.env`:
```env
DB_PASSWORD=YourSecurePassword123
SECRET_KEY=YourRandomSecretKey456
DEBUG=false
```

### 4. Start Services

```bash
docker-compose up -d
```

### 5. Verify Installation

```bash
# Check services
docker-compose ps

# Check logs
docker-compose logs backend
docker-compose logs frontend

# Test API
curl http://localhost:8000/health
```

### 6. Create First User

Access http://localhost:3000 and register.

---

## Manual Installation

### 1. Install PostgreSQL with TimescaleDB

**Ubuntu/Debian:**
```bash
# Add PostgreSQL repository
sudo sh -c 'echo "deb https://apt.postgresql.org/pub/repos/apt $(lsb_release -cs)-pgdg main" > /etc/apt/sources.list.d/pgdg.list'
wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | sudo apt-key add -
sudo apt update

# Install PostgreSQL
sudo apt install -y postgresql-15

# Install TimescaleDB
sudo add-apt-repository ppa:timescale/timescaledb-ppa
sudo apt update
sudo apt install -y timescaledb-2-postgresql-15

# Setup TimescaleDB
sudo timescaledb-tune --quiet --yes

# Restart PostgreSQL
sudo systemctl restart postgresql
```

**Create Database:**
```bash
sudo -u postgres psql
```

```sql
CREATE DATABASE watchdog360;
CREATE USER watchdog360 WITH PASSWORD 'yourpassword';
GRANT ALL PRIVILEGES ON DATABASE watchdog360 TO watchdog360;
\c watchdog360
CREATE EXTENSION IF NOT EXISTS timescaledb;
\q
```

### 2. Install Redis

**Ubuntu/Debian:**
```bash
sudo apt install -y redis-server
sudo systemctl enable redis-server
sudo systemctl start redis-server
```

### 3. Install Backend

```bash
cd backend

# Create virtual environment
python3.11 -m venv venv
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt

# Configure
cp .env.example .env
vim .env

# Run migrations
alembic upgrade head

# Start server
uvicorn app.main:app --host 0.0.0.0 --port 8000
```

**Create systemd service:**
```bash
sudo vim /etc/systemd/system/watchdog360-backend.service
```

```ini
[Unit]
Description=Watchdog360 Backend
After=network.target postgresql.service redis.service

[Service]
Type=simple
User=watchdog360
WorkingDirectory=/opt/watchdog360/backend
Environment="PATH=/opt/watchdog360/backend/venv/bin"
ExecStart=/opt/watchdog360/backend/venv/bin/uvicorn app.main:app --host 0.0.0.0 --port 8000
Restart=always

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl daemon-reload
sudo systemctl enable watchdog360-backend
sudo systemctl start watchdog360-backend
```

### 4. Install Frontend

```bash
cd frontend

# Install Node.js 18
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt install -y nodejs

# Install dependencies
npm install

# Build
npm run build

# Serve with nginx
sudo apt install -y nginx
sudo cp dist/* /var/www/watchdog360/
```

**Nginx configuration:**
```bash
sudo vim /etc/nginx/sites-available/watchdog360
```

```nginx
server {
    listen 80;
    server_name your-domain.com;
    root /var/www/watchdog360;
    index index.html;

    location / {
        try_files $uri $uri/ /index.html;
    }

    location /api/ {
        proxy_pass http://localhost:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

```bash
sudo ln -s /etc/nginx/sites-available/watchdog360 /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

---

## Agent Installation

### Automatic Installation (Recommended)

```bash
curl -s https://your-watchdog360-domain.com/install.sh | sudo bash
```

During installation, you'll be prompted for:
- API URL (e.g., https://api.watchdog360.com)
- Authentication token

### Manual Installation

```bash
# Install Python and pip
sudo apt install -y python3 python3-pip

# Download agent
sudo mkdir -p /opt/watchdog360
cd /opt/watchdog360
sudo wget https://your-domain.com/watchdog_agent.py

# Install dependencies
sudo pip3 install psutil requests

# Create config
sudo mkdir -p /etc/watchdog360
sudo vim /etc/watchdog360/config.json
```

```json
{
    "api_url": "https://api.watchdog360.com",
    "token": "your-api-token-here",
    "interval": 10
}
```

**Create systemd service:**
```bash
sudo vim /etc/systemd/system/watchdog360.service
```

```ini
[Unit]
Description=Watchdog360 Monitoring Agent
After=network.target

[Service]
Type=simple
User=root
ExecStart=/usr/bin/python3 /opt/watchdog360/watchdog_agent.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl daemon-reload
sudo systemctl enable watchdog360
sudo systemctl start watchdog360
```

### Verify Agent

```bash
# Check status
sudo systemctl status watchdog360

# View logs
sudo journalctl -u watchdog360 -f

# Check metrics in dashboard
# Visit http://your-domain.com/dashboard
```

---

## Configuration

### Backend Configuration

**Environment Variables** (`.env`):

| Variable | Description | Default |
|----------|-------------|---------|
| DATABASE_URL | PostgreSQL connection string | - |
| REDIS_URL | Redis connection string | - |
| SECRET_KEY | JWT secret key | - |
| ALLOWED_ORIGINS | CORS origins | - |
| ALERT_HIGH_CPU_THRESHOLD | CPU alert threshold | 80 |
| ALERT_HIGH_MEMORY_THRESHOLD | Memory alert threshold | 85 |
| ANOMALY_DETECTION_ENABLED | Enable AI anomaly detection | true |

### Agent Configuration

**Config file** (`/etc/watchdog360/config.json`):

```json
{
    "api_url": "https://api.watchdog360.com",
    "token": "your-token",
    "interval": 10,
    "hostname": "custom-hostname"
}
```

| Field | Description | Default |
|-------|-------------|---------|
| api_url | Backend API URL | - |
| token | Authentication token | - |
| interval | Metrics push interval (seconds) | 10 |
| hostname | Custom hostname | System hostname |

---

## Troubleshooting

### Backend Issues

**Database connection failed**
```bash
# Check PostgreSQL status
sudo systemctl status postgresql

# Test connection
psql -U watchdog360 -d watchdog360 -h localhost
```

**Redis connection failed**
```bash
# Check Redis status
sudo systemctl status redis

# Test connection
redis-cli ping
```

### Agent Issues

**Agent not starting**
```bash
# Check logs
sudo journalctl -u watchdog360 -n 50

# Common issues:
# 1. Python not installed
sudo apt install python3 python3-pip

# 2. Missing dependencies
sudo pip3 install psutil requests

# 3. Config file missing
sudo ls -la /etc/watchdog360/config.json
```

**Metrics not appearing**
```bash
# Check agent logs
sudo tail -f /var/log/watchdog360/agent.log

# Test API connectivity
curl -H "Authorization: Bearer YOUR_TOKEN" \
     https://api.watchdog360.com/api/v1/dashboard/stats

# Check firewall
sudo ufw status
```

### Frontend Issues

**Blank page**
```bash
# Check browser console for errors
# Check nginx logs
sudo tail -f /var/log/nginx/error.log

# Verify build
ls -la /var/www/watchdog360/
```

**API connection failed**
- Check CORS configuration in backend
- Verify API URL in frontend `.env`
- Check network connectivity

---

## Security Considerations

1. **Use HTTPS in production**
   - Configure SSL certificates
   - Use Let's Encrypt for free certs

2. **Secure database**
   - Use strong passwords
   - Enable SSL for PostgreSQL

3. **Firewall rules**
   ```bash
   sudo ufw allow 22    # SSH
   sudo ufw allow 80    # HTTP
   sudo ufw allow 443   # HTTPS
   sudo ufw enable
   ```

4. **Regular updates**
   ```bash
   # Update system
   sudo apt update && sudo apt upgrade

   # Update Docker images
   docker-compose pull
   docker-compose up -d
   ```

---

## Next Steps

- [Configuration Guide](CONFIGURATION.md)
- [API Documentation](API.md)
- [Deployment Guide](DEPLOYMENT.md)
- [FAQ](FAQ.md)

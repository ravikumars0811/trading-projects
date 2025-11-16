# Watchdog360 Quick Start Guide

Get up and running with Watchdog360 in under 5 minutes!

## Prerequisites

- Docker & Docker Compose installed
- One or more Linux servers to monitor

## Step 1: Launch Watchdog360 (2 minutes)

```bash
# Clone repository
git clone https://github.com/yourusername/watchdog360.git
cd watchdog360/deployment

# Configure
cp .env.example .env
# Edit .env and set DB_PASSWORD and SECRET_KEY

# Start all services
docker-compose up -d

# Verify services are running
docker-compose ps
```

Expected output:
```
NAME                    STATUS
watchdog360-backend     Up
watchdog360-frontend    Up
watchdog360-db          Up (healthy)
watchdog360-redis       Up (healthy)
```

## Step 2: Create Account (1 minute)

1. Open http://localhost:3000 in browser
2. Click "Register"
3. Enter email and password
4. Click "Register"

You'll be automatically logged in.

## Step 3: Get API Token (1 minute)

For now, use the access token from login. In production:

```bash
# Login via API
curl -X POST http://localhost:8000/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "email": "your@email.com",
    "password": "yourpassword"
  }'

# Copy the access_token from response
```

## Step 4: Install Agent on Server (1 minute)

On the server you want to monitor:

```bash
# Download install script
curl -o install.sh https://raw.githubusercontent.com/yourusername/watchdog360/main/agent/install.sh

# Run installation
sudo bash install.sh

# When prompted:
# - API URL: http://your-watchdog360-ip:8000
# - Token: [paste token from Step 3]
```

The agent will:
- Install Python and dependencies
- Create systemd service
- Start monitoring automatically

## Step 5: View Metrics (30 seconds)

1. Refresh dashboard at http://localhost:3000
2. You should see your server appear
3. Click on the server to see detailed metrics
4. Watch real-time graphs update every 10 seconds!

## What's Next?

### Customize Alert Thresholds

Edit `deployment/.env`:
```env
ALERT_HIGH_CPU_THRESHOLD=80
ALERT_HIGH_MEMORY_THRESHOLD=85
ALERT_HIGH_DISK_THRESHOLD=90
```

Restart backend:
```bash
docker-compose restart backend
```

### Enable AI Features

AI/ML features are enabled by default:
- Anomaly detection using Isolation Forest
- Predictive analytics using Prophet
- Smart alerts

Customize in `.env`:
```env
ANOMALY_DETECTION_ENABLED=true
ANOMALY_DETECTION_THRESHOLD=0.85
PREDICTION_ENABLED=true
```

### Monitor Multiple Servers

Simply run the install script on each server:
```bash
curl -s https://your-domain.com/install.sh | sudo bash
```

All servers will appear in your dashboard.

### Setup Production Deployment

See [Deployment Guide](DEPLOYMENT.md) for:
- HTTPS/SSL configuration
- Domain setup
- Performance tuning
- High availability

## Troubleshooting

### Server not appearing in dashboard?

**Check agent status:**
```bash
sudo systemctl status watchdog360
sudo journalctl -u watchdog360 -f
```

**Verify connectivity:**
```bash
curl -H "Authorization: Bearer YOUR_TOKEN" \
     http://your-watchdog360-ip:8000/health
```

### Metrics not updating?

**Check backend logs:**
```bash
docker-compose logs backend
```

**Verify database:**
```bash
docker-compose exec timescaledb psql -U watchdog360 -c "SELECT COUNT(*) FROM server_metrics;"
```

### Dashboard not loading?

**Check frontend:**
```bash
docker-compose logs frontend
```

**Verify nginx:**
```bash
docker-compose exec frontend nginx -t
```

## Common Tasks

### View All Logs
```bash
docker-compose logs -f
```

### Restart Services
```bash
docker-compose restart backend
docker-compose restart frontend
```

### Stop Everything
```bash
docker-compose down
```

### Backup Database
```bash
docker-compose exec timescaledb pg_dump -U watchdog360 watchdog360 > backup.sql
```

### Update to Latest Version
```bash
git pull
docker-compose pull
docker-compose up -d
```

## Testing the System

### Generate Test Load

On monitored server:
```bash
# CPU stress
stress --cpu 4 --timeout 60

# Memory stress
stress --vm 2 --vm-bytes 1G --timeout 60
```

Watch the dashboard - you should see:
- Real-time graph spikes
- Alerts trigger (if thresholds exceeded)
- Anomaly detection flag unusual patterns

## Next Steps

- [Full Installation Guide](INSTALLATION.md)
- [Configuration Options](CONFIGURATION.md)
- [API Documentation](../README.md#api-documentation)
- [AI/ML Features](../README.md#aiml-features-explained)

## Need Help?

- Check [FAQ](FAQ.md)
- Open [GitHub Issue](https://github.com/yourusername/watchdog360/issues)
- Email: support@watchdog360.com

---

**Congratulations!** 🎉 You now have a fully functional AI-powered server monitoring system!

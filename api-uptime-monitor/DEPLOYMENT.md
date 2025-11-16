# Deployment Guide

## Table of Contents
1. [Docker Compose Deployment](#docker-compose-deployment)
2. [Production Deployment](#production-deployment)
3. [Cloud Provider Specific](#cloud-provider-specific)
4. [Monitoring & Maintenance](#monitoring--maintenance)

## Docker Compose Deployment

### Prerequisites
- Docker 24.0+
- Docker Compose 2.20+
- 4GB RAM minimum
- 20GB disk space

### Quick Start

```bash
# Clone repository
git clone <repository-url>
cd api-uptime-monitor

# Copy environment files
cp backend/.env.example backend/.env
cp frontend/.env.example frontend/.env

# Generate secure secret key
python -c "import secrets; print(secrets.token_urlsafe(32))"
# Add to backend/.env as SECRET_KEY

# Start all services
docker-compose up -d

# Check service status
docker-compose ps

# View logs
docker-compose logs -f
```

### Service URLs
- Frontend: http://localhost:3000
- Backend API: http://localhost:8000
- API Docs: http://localhost:8000/api/docs
- Flower (Celery): http://localhost:5555

### Stopping Services

```bash
# Stop all services
docker-compose down

# Stop and remove volumes (WARNING: deletes all data)
docker-compose down -v
```

## Production Deployment

### 1. Environment Configuration

Create production `.env` file:

```env
# Application
ENVIRONMENT=production
DEBUG=false

# Database (use managed service)
DATABASE_URL=postgresql+asyncpg://user:pass@prod-db.example.com:5432/uptime_monitor
POSTGRES_USER=production_user
POSTGRES_PASSWORD=<strong-password>
POSTGRES_DB=uptime_monitor

# Redis (use managed service)
REDIS_URL=redis://:password@prod-redis.example.com:6379/0

# Security
SECRET_KEY=<generate-with-openssl-rand-hex-32>
ALGORITHM=HS256
ACCESS_TOKEN_EXPIRE_MINUTES=60

# Email
SENDGRID_API_KEY=SG.xxxxxxxxxxxxx
FROM_EMAIL=alerts@yourdomain.com

# Telegram
TELEGRAM_BOT_TOKEN=123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11

# Slack
SLACK_WEBHOOK_URL=https://hooks.slack.com/services/T00000000/B00000000/XXXXXXXXXXXX

# Monitoring
CHECK_INTERVAL_SECONDS=60
MAX_WORKERS=8
```

### 2. Database Setup

#### Option A: Managed PostgreSQL (Recommended)

**AWS RDS:**
```bash
# Create RDS instance with TimescaleDB
aws rds create-db-instance \
  --db-instance-identifier uptime-monitor-db \
  --db-instance-class db.t3.medium \
  --engine postgres \
  --engine-version 15.4 \
  --master-username admin \
  --master-user-password <password> \
  --allocated-storage 100 \
  --backup-retention-period 7 \
  --multi-az
```

**Google Cloud SQL:**
```bash
gcloud sql instances create uptime-monitor-db \
  --database-version=POSTGRES_15 \
  --tier=db-n1-standard-2 \
  --region=us-central1 \
  --backup-start-time=03:00
```

#### Option B: Self-hosted PostgreSQL

```bash
# Install PostgreSQL 15
sudo apt-get update
sudo apt-get install postgresql-15 postgresql-contrib

# Install TimescaleDB
sudo add-apt-repository ppa:timescale/timescaledb-ppa
sudo apt-get update
sudo apt-get install timescaledb-2-postgresql-15

# Configure PostgreSQL
sudo -u postgres psql
CREATE DATABASE uptime_monitor;
CREATE USER uptime_user WITH PASSWORD 'secure_password';
GRANT ALL PRIVILEGES ON DATABASE uptime_monitor TO uptime_user;

# Enable TimescaleDB
\c uptime_monitor
CREATE EXTENSION IF NOT EXISTS timescaledb;
```

### 3. Application Deployment

#### Option A: Docker Swarm

```bash
# Initialize swarm
docker swarm init

# Create secrets
echo "your-secret-key" | docker secret create db_password -
echo "your-redis-password" | docker secret create redis_password -

# Deploy stack
docker stack deploy -c docker-compose.yml uptime-monitor

# Check services
docker stack services uptime-monitor
```

#### Option B: Kubernetes

```yaml
# kubernetes/deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: uptime-monitor-backend
spec:
  replicas: 3
  selector:
    matchLabels:
      app: uptime-monitor-backend
  template:
    metadata:
      labels:
        app: uptime-monitor-backend
    spec:
      containers:
      - name: backend
        image: your-registry/uptime-monitor-backend:latest
        ports:
        - containerPort: 8000
        env:
        - name: DATABASE_URL
          valueFrom:
            secretKeyRef:
              name: uptime-monitor-secrets
              key: database-url
        - name: REDIS_URL
          valueFrom:
            secretKeyRef:
              name: uptime-monitor-secrets
              key: redis-url
---
apiVersion: v1
kind: Service
metadata:
  name: uptime-monitor-backend
spec:
  selector:
    app: uptime-monitor-backend
  ports:
  - port: 8000
    targetPort: 8000
  type: LoadBalancer
```

Deploy:
```bash
kubectl apply -f kubernetes/
```

### 4. SSL/TLS Configuration

#### Using Nginx Reverse Proxy

```nginx
# /etc/nginx/sites-available/uptime-monitor
server {
    listen 80;
    server_name api.yourdomain.com;
    return 301 https://$server_name$request_uri;
}

server {
    listen 443 ssl http2;
    server_name api.yourdomain.com;

    ssl_certificate /etc/letsencrypt/live/api.yourdomain.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/api.yourdomain.com/privkey.pem;

    location / {
        proxy_pass http://localhost:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}

server {
    listen 443 ssl http2;
    server_name monitor.yourdomain.com;

    ssl_certificate /etc/letsencrypt/live/monitor.yourdomain.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/monitor.yourdomain.com/privkey.pem;

    location / {
        proxy_pass http://localhost:3000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

Enable and restart:
```bash
sudo ln -s /etc/nginx/sites-available/uptime-monitor /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl restart nginx
```

### 5. Performance Tuning

#### PostgreSQL Configuration

```bash
# /etc/postgresql/15/main/postgresql.conf

# Memory
shared_buffers = 2GB
effective_cache_size = 6GB
maintenance_work_mem = 512MB
work_mem = 16MB

# Connections
max_connections = 200

# TimescaleDB
timescaledb.max_background_workers = 8
```

#### Celery Worker Configuration

```bash
# Scale workers based on CPU cores
docker-compose up --scale celery-worker=4

# Or in production
celery -A app.core.celery_app worker \
  --loglevel=info \
  --concurrency=8 \
  --max-tasks-per-child=1000
```

## Cloud Provider Specific

### AWS Deployment

#### Architecture
- **Compute**: ECS Fargate or EKS
- **Database**: RDS PostgreSQL with TimescaleDB
- **Cache**: ElastiCache Redis
- **Storage**: S3 for ML models
- **Load Balancer**: Application Load Balancer
- **CDN**: CloudFront

#### Terraform Example

```hcl
# main.tf
resource "aws_db_instance" "postgres" {
  identifier           = "uptime-monitor-db"
  engine              = "postgres"
  engine_version      = "15.4"
  instance_class      = "db.t3.medium"
  allocated_storage   = 100
  storage_encrypted   = true
  username            = var.db_username
  password            = var.db_password
  multi_az            = true
  backup_retention_period = 7
}

resource "aws_elasticache_cluster" "redis" {
  cluster_id           = "uptime-monitor-redis"
  engine               = "redis"
  node_type            = "cache.t3.medium"
  num_cache_nodes      = 1
  parameter_group_name = "default.redis7"
  port                 = 6379
}

resource "aws_ecs_cluster" "main" {
  name = "uptime-monitor-cluster"
}
```

### Google Cloud Platform

```bash
# Create GKE cluster
gcloud container clusters create uptime-monitor \
  --num-nodes=3 \
  --machine-type=n1-standard-2 \
  --region=us-central1

# Create Cloud SQL instance
gcloud sql instances create uptime-monitor-db \
  --database-version=POSTGRES_15 \
  --tier=db-n1-standard-2 \
  --region=us-central1

# Create Redis instance
gcloud redis instances create uptime-monitor-redis \
  --size=2 \
  --region=us-central1
```

### Azure

```bash
# Create resource group
az group create --name uptime-monitor-rg --location eastus

# Create PostgreSQL
az postgres server create \
  --resource-group uptime-monitor-rg \
  --name uptime-monitor-db \
  --sku-name GP_Gen5_2 \
  --version 15

# Create Redis
az redis create \
  --resource-group uptime-monitor-rg \
  --name uptime-monitor-redis \
  --sku Standard \
  --vm-size c1

# Create AKS cluster
az aks create \
  --resource-group uptime-monitor-rg \
  --name uptime-monitor-aks \
  --node-count 3 \
  --enable-managed-identity
```

## Monitoring & Maintenance

### Health Checks

```bash
# Backend health
curl http://localhost:8000/health

# Database connection
docker-compose exec postgres pg_isready

# Redis connection
docker-compose exec redis redis-cli ping
```

### Backup Strategy

#### Database Backups

```bash
# Automated daily backups
0 2 * * * docker-compose exec -T postgres pg_dump -U uptime_user uptime_monitor | gzip > /backups/uptime_monitor_$(date +\%Y\%m\%d).sql.gz

# Restore from backup
gunzip < backup.sql.gz | docker-compose exec -T postgres psql -U uptime_user uptime_monitor
```

#### ML Models Backup

```bash
# Backup ML models to S3
aws s3 sync /app/ml_models s3://your-bucket/ml_models/

# Schedule with cron
0 4 * * * aws s3 sync /app/ml_models s3://your-bucket/ml_models/
```

### Log Management

```bash
# Configure log rotation
# /etc/logrotate.d/uptime-monitor
/var/log/uptime-monitor/*.log {
    daily
    rotate 14
    compress
    delaycompress
    notifempty
    create 0640 www-data www-data
    sharedscripts
}
```

### Updates

```bash
# Pull latest images
docker-compose pull

# Restart with zero downtime
docker-compose up -d --no-deps --build backend
docker-compose up -d --no-deps --build frontend
docker-compose up -d --no-deps --build celery-worker
```

### Monitoring Alerts

Set up alerts for:
- Database connection failures
- High CPU/memory usage
- Failed Celery tasks
- Disk space low
- SSL certificate expiry

## Troubleshooting

### Common Issues

**Services won't start:**
```bash
# Check logs
docker-compose logs backend
docker-compose logs postgres

# Check network
docker network ls
docker network inspect uptime-monitor_default
```

**Database migration issues:**
```bash
# Run migrations manually
docker-compose exec backend alembic upgrade head
```

**Celery tasks not running:**
```bash
# Check Celery worker
docker-compose logs celery-worker

# Purge queue
docker-compose exec redis redis-cli FLUSHDB
```

## Security Hardening

- [ ] Enable firewall (ufw, iptables)
- [ ] Configure fail2ban
- [ ] Use strong passwords
- [ ] Enable SSL/TLS
- [ ] Regular security updates
- [ ] Implement rate limiting
- [ ] Enable audit logging
- [ ] Use secrets management
- [ ] Configure CORS properly
- [ ] Enable database SSL

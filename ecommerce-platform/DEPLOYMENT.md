# Deployment Guide

This guide covers deploying the e-commerce platform to various environments.

## Table of Contents
- [Docker Compose Deployment](#docker-compose-deployment)
- [AWS Deployment](#aws-deployment)
- [Google Cloud Deployment](#google-cloud-deployment)
- [Digital Ocean Deployment](#digital-ocean-deployment)
- [Environment Configuration](#environment-configuration)
- [SSL/HTTPS Setup](#sslhttps-setup)
- [Monitoring & Logging](#monitoring--logging)

## Docker Compose Deployment

### Local/Development

1. Configure environment variables:
```bash
cp .env.example .env
# Edit .env with your configuration
```

2. Build and start services:
```bash
docker-compose up -d --build
```

3. Check logs:
```bash
docker-compose logs -f
```

### Production

1. Use production environment variables
2. Configure proper secrets
3. Set up SSL certificates
4. Run with restart policies:

```bash
docker-compose -f docker-compose.yml -f docker-compose.prod.yml up -d
```

## AWS Deployment

### Using AWS ECS (Elastic Container Service)

1. **Push images to ECR**:
```bash
# Login to ECR
aws ecr get-login-password --region us-east-1 | docker login --username AWS --password-stdin YOUR_ECR_URL

# Build and tag images
docker build -t ecommerce-backend ./backend
docker tag ecommerce-backend:latest YOUR_ECR_URL/ecommerce-backend:latest
docker push YOUR_ECR_URL/ecommerce-backend:latest

docker build -t ecommerce-frontend ./frontend
docker tag ecommerce-frontend:latest YOUR_ECR_URL/ecommerce-frontend:latest
docker push YOUR_ECR_URL/ecommerce-frontend:latest
```

2. **Set up RDS (PostgreSQL)**:
- Create RDS PostgreSQL instance
- Configure security groups
- Update connection strings

3. **Set up DocumentDB (MongoDB)**:
- Create DocumentDB cluster
- Configure VPC and security groups
- Update MongoDB connection string

4. **Set up ElastiCache (Redis)**:
- Create Redis cluster
- Configure security groups
- Update Redis connection

5. **Create ECS Task Definitions** for backend and frontend

6. **Set up Application Load Balancer**:
- Create target groups
- Configure health checks
- Set up SSL certificate (AWS ACM)

7. **Create ECS Service** and deploy

### Using AWS Elastic Beanstalk

1. Install EB CLI
2. Initialize:
```bash
eb init
```
3. Create environment:
```bash
eb create production
```
4. Deploy:
```bash
eb deploy
```

## Google Cloud Deployment

### Using Google Cloud Run

1. **Build and push to Container Registry**:
```bash
# Backend
gcloud builds submit --tag gcr.io/PROJECT_ID/ecommerce-backend ./backend

# Frontend
gcloud builds submit --tag gcr.io/PROJECT_ID/ecommerce-frontend ./frontend
```

2. **Deploy to Cloud Run**:
```bash
# Backend
gcloud run deploy ecommerce-backend \
  --image gcr.io/PROJECT_ID/ecommerce-backend \
  --platform managed \
  --region us-central1 \
  --allow-unauthenticated

# Frontend
gcloud run deploy ecommerce-frontend \
  --image gcr.io/PROJECT_ID/ecommerce-frontend \
  --platform managed \
  --region us-central1 \
  --allow-unauthenticated
```

3. **Set up Cloud SQL (PostgreSQL)**
4. **Set up MongoDB Atlas** (recommended over self-hosted)
5. **Set up Memorystore (Redis)**

### Using GKE (Kubernetes)

1. Create GKE cluster
2. Apply Kubernetes manifests
3. Configure ingress
4. Set up managed certificates

## Digital Ocean Deployment

### Using App Platform

1. Connect GitHub repository
2. Configure build settings
3. Set environment variables
4. Deploy

### Using Droplets

1. Create Ubuntu droplet
2. Install Docker and Docker Compose
3. Clone repository
4. Run docker-compose

## Environment Configuration

### Production Environment Variables

**Critical Settings**:
```env
NODE_ENV=production
JWT_SECRET=STRONG_RANDOM_SECRET_CHANGE_THIS
SESSION_SECRET=STRONG_RANDOM_SECRET_CHANGE_THIS

# Use strong database passwords
POSTGRES_PASSWORD=STRONG_PASSWORD
MONGO_PASSWORD=STRONG_PASSWORD

# Production URLs
FRONTEND_URL=https://yourdomain.com
CORS_ORIGIN=https://yourdomain.com

# Production payment keys
STRIPE_SECRET_KEY=sk_live_your_key
PAYPAL_MODE=live
```

### Environment-Specific Configs

Create multiple docker-compose files:
- `docker-compose.yml` - Base configuration
- `docker-compose.dev.yml` - Development overrides
- `docker-compose.prod.yml` - Production overrides

## SSL/HTTPS Setup

### Using Let's Encrypt with Certbot

1. Install Certbot
2. Generate certificates:
```bash
certbot certonly --standalone -d yourdomain.com -d www.yourdomain.com
```

3. Update nginx configuration
4. Set up auto-renewal:
```bash
certbot renew --dry-run
```

### Using Cloudflare

1. Add domain to Cloudflare
2. Update nameservers
3. Enable SSL/TLS
4. Configure page rules

## Monitoring & Logging

### Application Monitoring

**Using PM2**:
```bash
pm2 start dist/server.js --name ecommerce-api
pm2 startup
pm2 save
pm2 monit
```

### Log Management

**Using ELK Stack**:
1. Set up Elasticsearch
2. Configure Logstash
3. Set up Kibana
4. Ship logs from Winston

**Using CloudWatch (AWS)**:
- Configure CloudWatch Logs agent
- Set up log groups
- Create dashboards

### Health Checks

All services include health check endpoints:
- Backend: `GET /health`
- Frontend: nginx health check
- Databases: Docker health checks

### Uptime Monitoring

Use services like:
- UptimeRobot
- Pingdom
- StatusCake
- AWS CloudWatch Alarms

## Database Backups

### PostgreSQL

```bash
# Backup
pg_dump -U user -h host dbname > backup.sql

# Restore
psql -U user -h host dbname < backup.sql
```

### MongoDB

```bash
# Backup
mongodump --uri="mongodb://user:pass@host/db" --out=/backup

# Restore
mongorestore --uri="mongodb://user:pass@host/db" /backup
```

### Automated Backups

Set up cron jobs or use managed database backup features.

## Scaling

### Horizontal Scaling

**Backend**:
```bash
docker-compose up -d --scale backend=3
```

**Database**:
- Use read replicas
- Implement connection pooling
- Use caching extensively

### Load Balancing

Configure nginx or use cloud load balancers:
```nginx
upstream backend {
    server backend1:5000;
    server backend2:5000;
    server backend3:5000;
}
```

## Security Checklist

- [ ] Use HTTPS everywhere
- [ ] Strong passwords and secrets
- [ ] Regular security updates
- [ ] Database access restricted
- [ ] Rate limiting enabled
- [ ] CORS properly configured
- [ ] Input validation active
- [ ] SQL injection protection
- [ ] XSS protection enabled
- [ ] Regular backups
- [ ] Monitoring and alerting
- [ ] Security headers configured

## Performance Optimization

1. **Enable Compression**: Already configured
2. **Use CDN**: For static assets
3. **Database Indexing**: Already implemented
4. **Redis Caching**: Already implemented
5. **Image Optimization**: Use Sharp
6. **Code Splitting**: Frontend already configured
7. **Lazy Loading**: Implement for images

## Troubleshooting

### Common Issues

**Database Connection Errors**:
- Check network connectivity
- Verify credentials
- Check firewall rules

**Out of Memory**:
- Increase container memory limits
- Check for memory leaks
- Optimize queries

**Slow Performance**:
- Check database indexes
- Enable caching
- Optimize queries
- Use connection pooling

## Rollback Strategy

1. Tag all releases
2. Keep previous Docker images
3. Use blue-green deployment
4. Test in staging first
5. Have database migration rollback scripts

## Support

For deployment issues:
1. Check logs: `docker-compose logs`
2. Verify environment variables
3. Check network connectivity
4. Review security groups/firewall rules
5. Consult cloud provider documentation

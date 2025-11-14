# Deployment Guide

Complete guide for deploying the Payment System to various environments.

## Table of Contents

1. [Local Development Setup](#local-development-setup)
2. [AWS ECS Deployment](#aws-ecs-deployment)
3. [Kubernetes Deployment](#kubernetes-deployment)
4. [AWS Auto-Scaling Configuration](#aws-auto-scaling-configuration)
5. [Monitoring Setup](#monitoring-setup)
6. [Troubleshooting](#troubleshooting)

## Local Development Setup

### Prerequisites
- Docker Desktop 20.10+
- Docker Compose 2.0+
- Make (optional, for Makefile commands)

### Step 1: Clone and Setup

```bash
# Clone repository
git clone https://github.com/username/payment-system.git
cd payment-system

# Copy environment file
cp .env.example .env

# Edit .env with your configurations
vim .env
```

### Step 2: Start Services

```bash
# Using Make
make up

# Or using Docker Compose directly
docker-compose up -d
```

### Step 3: Verify Installation

```bash
# Check all services are running
make ps

# View logs
make logs

# Check health
make health
```

### Step 4: Access Services

- API Gateway: http://localhost:8000
- API Docs: http://localhost:8000/docs
- Grafana: http://localhost:3000 (admin/admin)
- Prometheus: http://localhost:9090

## AWS ECS Deployment

### Prerequisites

- AWS Account with appropriate permissions
- AWS CLI installed and configured
- Terraform 1.0+
- Docker

### Step 1: Configure AWS Credentials

```bash
aws configure
# Enter your AWS Access Key ID
# Enter your AWS Secret Access Key
# Enter your default region (e.g., us-east-1)
```

### Step 2: Create ECR Repositories

```bash
cd deployments/terraform

# Initialize Terraform
terraform init

# Preview changes
terraform plan

# Apply infrastructure (creates VPC, ECR, RDS, etc.)
terraform apply
```

**Note the outputs:**
- ECR repository URLs
- RDS endpoint
- Redis endpoint
- Load Balancer DNS

### Step 3: Build and Push Docker Images

```bash
# Login to ECR
aws ecr get-login-password --region us-east-1 | \
  docker login --username AWS --password-stdin \
  YOUR_ACCOUNT.dkr.ecr.us-east-1.amazonaws.com

# Build and push all services
services=("user-service" "wallet-service" "transaction-service" "payment-gateway" "notification-service" "api-gateway")

for service in "${services[@]}"; do
  echo "Building and pushing $service..."

  if [ "$service" = "api-gateway" ]; then
    DOCKERFILE_PATH="api-gateway/Dockerfile"
  else
    DOCKERFILE_PATH="services/$service/Dockerfile"
  fi

  # Build
  docker build -t $service -f $DOCKERFILE_PATH .

  # Tag
  docker tag $service:latest \
    YOUR_ACCOUNT.dkr.ecr.us-east-1.amazonaws.com/$service:latest

  # Push
  docker push YOUR_ACCOUNT.dkr.ecr.us-east-1.amazonaws.com/$service:latest
done
```

### Step 4: Create ECS Task Definitions

Create `task-definitions/api-gateway.json`:

```json
{
  "family": "api-gateway",
  "networkMode": "awsvpc",
  "requiresCompatibilities": ["FARGATE"],
  "cpu": "512",
  "memory": "1024",
  "executionRoleArn": "arn:aws:iam::YOUR_ACCOUNT:role/ecsTaskExecutionRole",
  "containerDefinitions": [
    {
      "name": "api-gateway",
      "image": "YOUR_ACCOUNT.dkr.ecr.us-east-1.amazonaws.com/api-gateway:latest",
      "portMappings": [
        {
          "containerPort": 8000,
          "protocol": "tcp"
        }
      ],
      "environment": [
        {
          "name": "USER_SERVICE_URL",
          "value": "http://user-service:8001"
        },
        {
          "name": "WALLET_SERVICE_URL",
          "value": "http://wallet-service:8002"
        },
        {
          "name": "TRANSACTION_SERVICE_URL",
          "value": "http://transaction-service:8003"
        },
        {
          "name": "PAYMENT_GATEWAY_URL",
          "value": "http://payment-gateway:8004"
        },
        {
          "name": "REDIS_HOST",
          "value": "YOUR_REDIS_ENDPOINT"
        }
      ],
      "logConfiguration": {
        "logDriver": "awslogs",
        "options": {
          "awslogs-group": "/ecs/payment-system",
          "awslogs-region": "us-east-1",
          "awslogs-stream-prefix": "api-gateway"
        }
      }
    }
  ]
}
```

Repeat for all services.

### Step 5: Create ECS Services

```bash
# Create API Gateway service
aws ecs create-service \
  --cluster payment-system-cluster \
  --service-name api-gateway \
  --task-definition api-gateway \
  --desired-count 2 \
  --launch-type FARGATE \
  --network-configuration "awsvpcConfiguration={subnets=[subnet-xxx,subnet-yyy],securityGroups=[sg-xxx],assignPublicIp=ENABLED}" \
  --load-balancers "targetGroupArn=arn:aws:elasticloadbalancing:region:account:targetgroup/api-gateway-tg/xxx,containerName=api-gateway,containerPort=8000"

# Repeat for other services
```

### Step 6: Configure Application Load Balancer

The ALB is created by Terraform. Configure routing rules:

```bash
# Add listener rules in AWS Console or via CLI
aws elbv2 create-listener \
  --load-balancer-arn arn:aws:elasticloadbalancing:region:account:loadbalancer/app/payment-alb/xxx \
  --protocol HTTP \
  --port 80 \
  --default-actions Type=forward,TargetGroupArn=arn:aws:elasticloadbalancing:region:account:targetgroup/api-gateway-tg/xxx
```

### Step 7: Configure Auto-Scaling

Auto-scaling is configured in Terraform, but you can also set it manually:

```bash
# Register scalable target
aws application-autoscaling register-scalable-target \
  --service-namespace ecs \
  --resource-id service/payment-system-cluster/api-gateway \
  --scalable-dimension ecs:service:DesiredCount \
  --min-capacity 2 \
  --max-capacity 10

# Create scaling policy for CPU
aws application-autoscaling put-scaling-policy \
  --service-namespace ecs \
  --resource-id service/payment-system-cluster/api-gateway \
  --scalable-dimension ecs:service:DesiredCount \
  --policy-name api-gateway-cpu-scaling \
  --policy-type TargetTrackingScaling \
  --target-tracking-scaling-policy-configuration file://scaling-policy.json
```

scaling-policy.json:
```json
{
  "TargetValue": 70.0,
  "PredefinedMetricSpecification": {
    "PredefinedMetricType": "ECSServiceAverageCPUUtilization"
  },
  "ScaleOutCooldown": 60,
  "ScaleInCooldown": 60
}
```

### Step 8: Verify Deployment

```bash
# Check service status
aws ecs describe-services \
  --cluster payment-system-cluster \
  --services api-gateway user-service wallet-service

# Get load balancer DNS
aws elbv2 describe-load-balancers \
  --names payment-alb \
  --query 'LoadBalancers[0].DNSName'

# Test endpoint
curl http://YOUR_ALB_DNS/health
```

## Kubernetes Deployment

### Prerequisites

- Kubernetes cluster (EKS, GKE, or self-hosted)
- kubectl configured
- Helm 3.0+ (optional)

### Step 1: Create Namespace

```bash
kubectl create namespace payment-system
kubectl config set-context --current --namespace=payment-system
```

### Step 2: Create Secrets

```bash
kubectl create secret generic payment-secrets \
  --from-literal=jwt-secret=your-super-secret-key \
  --from-literal=db-password=your-db-password \
  --from-literal=redis-password=your-redis-password
```

### Step 3: Deploy Infrastructure

```bash
# Deploy PostgreSQL
kubectl apply -f deployments/kubernetes/postgres-deployment.yml

# Deploy Redis
kubectl apply -f deployments/kubernetes/redis-deployment.yml

# Deploy MongoDB
kubectl apply -f deployments/kubernetes/mongodb-deployment.yml

# Deploy Kafka
kubectl apply -f deployments/kubernetes/kafka-deployment.yml
```

### Step 4: Deploy Services

```bash
# Update image URLs in deployment files
# Then apply deployments
kubectl apply -f deployments/kubernetes/api-gateway-deployment.yml
kubectl apply -f deployments/kubernetes/user-service-deployment.yml
kubectl apply -f deployments/kubernetes/wallet-service-deployment.yml
kubectl apply -f deployments/kubernetes/transaction-service-deployment.yml
kubectl apply -f deployments/kubernetes/payment-gateway-deployment.yml
```

### Step 5: Verify Deployment

```bash
# Check pods
kubectl get pods

# Check services
kubectl get services

# Get external IP
kubectl get service api-gateway

# View logs
kubectl logs -f deployment/api-gateway
```

## AWS Auto-Scaling Configuration

### CPU-Based Auto-Scaling

```bash
# For each service
aws application-autoscaling register-scalable-target \
  --service-namespace ecs \
  --resource-id service/payment-system-cluster/SERVICE_NAME \
  --scalable-dimension ecs:service:DesiredCount \
  --min-capacity 2 \
  --max-capacity 10

aws application-autoscaling put-scaling-policy \
  --service-namespace ecs \
  --resource-id service/payment-system-cluster/SERVICE_NAME \
  --scalable-dimension ecs:service:DesiredCount \
  --policy-name cpu-scaling-policy \
  --policy-type TargetTrackingScaling \
  --target-tracking-scaling-policy-configuration '{
    "TargetValue": 70.0,
    "PredefinedMetricSpecification": {
      "PredefinedMetricType": "ECSServiceAverageCPUUtilization"
    }
  }'
```

### Memory-Based Auto-Scaling

```bash
aws application-autoscaling put-scaling-policy \
  --service-namespace ecs \
  --resource-id service/payment-system-cluster/SERVICE_NAME \
  --scalable-dimension ecs:service:DesiredCount \
  --policy-name memory-scaling-policy \
  --policy-type TargetTrackingScaling \
  --target-tracking-scaling-policy-configuration '{
    "TargetValue": 80.0,
    "PredefinedMetricSpecification": {
      "PredefinedMetricType": "ECSServiceAverageMemoryUtilization"
    }
  }'
```

### Request-Based Auto-Scaling

```bash
# Create custom CloudWatch metric
# Then create scaling policy based on that metric

aws application-autoscaling put-scaling-policy \
  --service-namespace ecs \
  --resource-id service/payment-system-cluster/api-gateway \
  --scalable-dimension ecs:service:DesiredCount \
  --policy-name request-count-scaling \
  --policy-type TargetTrackingScaling \
  --target-tracking-scaling-policy-configuration '{
    "TargetValue": 1000.0,
    "CustomizedMetricSpecification": {
      "MetricName": "RequestCountPerTarget",
      "Namespace": "AWS/ApplicationELB",
      "Statistic": "Sum"
    }
  }'
```

## Monitoring Setup

### Prometheus Configuration

Already configured in `infrastructure/prometheus/prometheus.yml`.

To add custom metrics:

```yaml
scrape_configs:
  - job_name: 'custom-service'
    static_configs:
      - targets: ['custom-service:port']
    metrics_path: '/metrics'
```

### Grafana Dashboard Setup

1. Access Grafana at http://YOUR_GRAFANA_URL:3000
2. Login (admin/admin)
3. Add Prometheus data source:
   - Go to Configuration → Data Sources
   - Add Prometheus
   - URL: http://prometheus:9090
   - Save & Test

4. Import dashboards:
   - Go to Dashboards → Import
   - Upload JSON from `infrastructure/grafana/dashboards/`

### CloudWatch Integration (AWS)

```bash
# Enable Container Insights
aws ecs update-cluster-settings \
  --cluster payment-system-cluster \
  --settings name=containerInsights,value=enabled

# View metrics in CloudWatch Console
# Metrics → ECS → ClusterName
```

## Troubleshooting

### Services Not Starting

```bash
# Check logs
docker-compose logs SERVICE_NAME

# For ECS
aws ecs describe-tasks \
  --cluster payment-system-cluster \
  --tasks TASK_ARN

# Check task stopped reason
aws logs get-log-events \
  --log-group-name /ecs/payment-system \
  --log-stream-name SERVICE_NAME/container/TASK_ID
```

### Database Connection Issues

```bash
# Test PostgreSQL connection
docker exec -it payment-postgres psql -U paymentuser -d payment_system

# Test from service container
docker exec -it user-service python -c "
from shared.database import engine
print(engine.connect())
"
```

### Kafka Issues

```bash
# Check Kafka topics
docker exec -it payment-kafka kafka-topics --list --bootstrap-server localhost:9092

# Check consumer groups
docker exec -it payment-kafka kafka-consumer-groups --list --bootstrap-server localhost:9092

# View messages
docker exec -it payment-kafka kafka-console-consumer \
  --bootstrap-server localhost:9092 \
  --topic transaction-events \
  --from-beginning
```

### Performance Issues

```bash
# Check resource usage
docker stats

# For ECS
aws cloudwatch get-metric-statistics \
  --namespace AWS/ECS \
  --metric-name CPUUtilization \
  --dimensions Name=ServiceName,Value=api-gateway \
  --start-time 2024-01-01T00:00:00Z \
  --end-time 2024-01-01T23:59:59Z \
  --period 300 \
  --statistics Average
```

### Rolling Back Deployment

```bash
# Docker Compose
git checkout PREVIOUS_COMMIT
docker-compose up -d --build

# ECS
aws ecs update-service \
  --cluster payment-system-cluster \
  --service api-gateway \
  --task-definition api-gateway:PREVIOUS_REVISION
```

## Production Checklist

- [ ] Change all default passwords
- [ ] Configure HTTPS/TLS
- [ ] Set up AWS Secrets Manager
- [ ] Configure backups (RDS, MongoDB)
- [ ] Set up monitoring alerts
- [ ] Configure log retention
- [ ] Enable WAF (Web Application Firewall)
- [ ] Set up VPN for internal access
- [ ] Configure DDoS protection
- [ ] Test disaster recovery procedures
- [ ] Document runbooks
- [ ] Set up on-call rotation
- [ ] Configure rate limiting
- [ ] Enable audit logging
- [ ] Test auto-scaling

## Support

For deployment issues:
- Check logs first
- Review CloudWatch metrics
- Consult [Architecture Documentation](architecture.md)
- Open GitHub issue with deployment logs

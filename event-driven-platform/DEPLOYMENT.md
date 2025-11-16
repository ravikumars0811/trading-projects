# Deployment Guide

This guide covers deploying the Event-Driven Microservices Platform to AWS.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Local Development](#local-development)
3. [AWS Infrastructure Setup](#aws-infrastructure-setup)
4. [CI/CD Pipeline Setup](#cicd-pipeline-setup)
5. [Monitoring Setup](#monitoring-setup)
6. [Troubleshooting](#troubleshooting)

## Prerequisites

### Required Tools

- Docker & Docker Compose (v20+)
- AWS CLI (v2+)
- Terraform (v1.6+)
- kubectl (for Kubernetes deployment)
- Node.js 18+
- Python 3.11+

### AWS Account Setup

1. **Create an AWS Account** (if you don't have one)

2. **Create an IAM User** with the following permissions:
   - AmazonEC2FullAccess
   - AmazonECSFullAccess
   - AmazonRDSFullAccess
   - AmazonElastiCacheFullAccess
   - AmazonMSKFullAccess
   - AmazonVPCFullAccess
   - AmazonRoute53FullAccess
   - CloudWatchFullAccess
   - IAMFullAccess (for creating roles)

3. **Configure AWS CLI**
   ```bash
   aws configure
   # Enter your AWS Access Key ID
   # Enter your AWS Secret Access Key
   # Default region: us-east-1
   # Default output format: json
   ```

4. **Create S3 Bucket for Terraform State** (recommended)
   ```bash
   aws s3 mb s3://your-terraform-state-bucket --region us-east-1
   aws s3api put-bucket-versioning \
     --bucket your-terraform-state-bucket \
     --versioning-configuration Status=Enabled
   ```

## Local Development

### Quick Start with Docker Compose

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd event-driven-platform
   ```

2. **Start all services**
   ```bash
   make up
   # or
   docker-compose up -d
   ```

3. **Check service health**
   ```bash
   make health
   ```

4. **Access the applications**
   - Frontend: http://localhost:3000
   - API Documentation: http://localhost:8001/docs (User Service)
   - Prometheus: http://localhost:9090
   - Grafana: http://localhost:3001 (admin/admin123)
   - Kibana: http://localhost:5601

5. **View logs**
   ```bash
   make logs
   # or for specific service
   make logs-user
   ```

6. **Stop all services**
   ```bash
   make down
   ```

### Manual Development Setup

If you want to run services individually:

1. **Start infrastructure services**
   ```bash
   docker-compose up -d postgres mongodb redis kafka zookeeper
   ```

2. **Start backend services**
   ```bash
   # Terminal 1 - User Service
   cd user-service
   pip install -r requirements.txt
   uvicorn app.main:app --reload --port 8001

   # Terminal 2 - Product Service
   cd product-service
   pip install -r requirements.txt
   uvicorn app.main:app --reload --port 8002

   # Terminal 3 - Order Service
   cd order-service
   pip install -r requirements.txt
   uvicorn app.main:app --reload --port 8003

   # Terminal 4 - Inventory Service
   cd inventory-service
   pip install -r requirements.txt
   uvicorn app.main:app --reload --port 8004

   # Terminal 5 - Notification Service
   cd notification-service
   pip install -r requirements.txt
   uvicorn app.main:app --reload --port 8005
   ```

3. **Start frontend**
   ```bash
   cd frontend
   npm install
   npm start
   ```

## AWS Infrastructure Setup

### Step 1: Configure Terraform Variables

1. **Copy the example variables file**
   ```bash
   cd infrastructure/terraform
   cp terraform.tfvars.example terraform.tfvars
   ```

2. **Edit terraform.tfvars**
   ```hcl
   aws_region       = "us-east-1"
   environment      = "production"
   project_name     = "event-driven-platform"

   # Database credentials (use strong passwords!)
   db_username      = "admin"
   db_password      = "YourStrongPassword123!"

   mongodb_username = "admin"
   mongodb_password = "YourStrongPassword123!"

   # Instance sizes (adjust for production)
   db_instance_class   = "db.t3.medium"
   redis_node_type     = "cache.t3.medium"
   kafka_instance_type = "kafka.m5.large"
   kafka_broker_nodes  = 3
   ```

3. **Update backend configuration** (if using S3 for state)
   ```hcl
   # In main.tf, uncomment and update:
   backend "s3" {
     bucket = "your-terraform-state-bucket"
     key    = "event-driven-platform/terraform.tfstate"
     region = "us-east-1"
   }
   ```

### Step 2: Deploy Infrastructure

1. **Initialize Terraform**
   ```bash
   cd infrastructure/terraform
   terraform init
   ```

2. **Review the plan**
   ```bash
   terraform plan
   ```

3. **Apply the configuration**
   ```bash
   terraform apply
   ```

   This will create:
   - VPC with public and private subnets
   - ECS Cluster (Fargate)
   - Application Load Balancer
   - RDS PostgreSQL instance
   - ElastiCache Redis cluster
   - MSK Kafka cluster
   - ECR repositories
   - IAM roles and policies
   - Security groups
   - CloudWatch log groups

4. **Note the outputs**
   ```bash
   terraform output
   ```

   Save these outputs:
   - `alb_dns_name`: Your application URL
   - `rds_endpoint`: PostgreSQL connection string
   - `redis_endpoint`: Redis connection string
   - `kafka_bootstrap_brokers`: Kafka brokers

### Step 3: Build and Push Docker Images

1. **Login to ECR**
   ```bash
   aws ecr get-login-password --region us-east-1 | \
     docker login --username AWS --password-stdin \
     <AWS_ACCOUNT_ID>.dkr.ecr.us-east-1.amazonaws.com
   ```

2. **Build and push images**
   ```bash
   # User Service
   cd user-service
   docker build -t <AWS_ACCOUNT_ID>.dkr.ecr.us-east-1.amazonaws.com/event-driven-platform/user-service:latest .
   docker push <AWS_ACCOUNT_ID>.dkr.ecr.us-east-1.amazonaws.com/event-driven-platform/user-service:latest

   # Repeat for other services...
   ```

   Or use the automated script:
   ```bash
   ./scripts/build-and-push.sh
   ```

### Step 4: Create ECS Task Definitions

See `infrastructure/terraform/ecs_tasks.tf` for task definition examples.

### Step 5: Deploy Services to ECS

The GitHub Actions pipeline will automatically deploy on push to main branch.

## CI/CD Pipeline Setup

### Step 1: Configure GitHub Secrets

Add these secrets to your GitHub repository:

1. Go to Settings > Secrets and variables > Actions
2. Add the following secrets:
   - `AWS_ACCESS_KEY_ID`: Your AWS access key
   - `AWS_SECRET_ACCESS_KEY`: Your AWS secret key
   - `AWS_ACCOUNT_ID`: Your 12-digit AWS account ID
   - `DB_PASSWORD`: PostgreSQL password
   - `MONGODB_PASSWORD`: MongoDB password

### Step 2: Enable GitHub Actions

1. The workflow file is already in `.github/workflows/ci-cd.yml`
2. Push to `main` branch to trigger deployment
3. Pull requests will run tests and Terraform plan

### Pipeline Stages

1. **Test** - Runs on every push and PR
   - Backend service tests
   - Frontend tests
   - Code quality checks

2. **Build & Push** - Runs on push to main/develop
   - Builds Docker images
   - Pushes to ECR
   - Tags with commit SHA

3. **Deploy** - Runs on push to main
   - Updates ECS services
   - Triggers rolling deployment
   - Zero-downtime deployment

4. **Security Scan** - Runs after build
   - Trivy vulnerability scanning
   - SARIF report upload to GitHub

## Monitoring Setup

### Prometheus Configuration

Prometheus is automatically configured to scrape metrics from all services.

Access: `http://<ALB_DNS_NAME>:9090`

### Grafana Dashboards

1. **Access Grafana**
   - URL: `http://<ALB_DNS_NAME>:3001`
   - Username: `admin`
   - Password: `admin123` (change in production)

2. **Import Dashboards**
   - Go to Dashboards > Import
   - Import the dashboards from `monitoring/grafana/dashboards/`

### CloudWatch Logs

All services send logs to CloudWatch:

```bash
# View logs for a service
aws logs tail /ecs/event-driven-platform/user-service --follow

# Filter logs
aws logs filter-log-events \
  --log-group-name /ecs/event-driven-platform/user-service \
  --filter-pattern "ERROR"
```

### ELK Stack

1. **Access Kibana**
   - URL: `http://<ALB_DNS_NAME>:5601`

2. **Create Index Pattern**
   - Go to Management > Index Patterns
   - Create pattern: `microservices-logs-*`
   - Select timestamp field: `@timestamp`

3. **Create Visualizations**
   - Go to Visualize > Create visualization
   - Select your index pattern
   - Create charts for error rates, request volumes, etc.

## Scaling

### Auto-Scaling Configuration

ECS services are configured with auto-scaling:

```bash
# Update auto-scaling settings
aws application-autoscaling register-scalable-target \
  --service-namespace ecs \
  --scalable-dimension ecs:service:DesiredCount \
  --resource-id service/event-driven-platform-cluster/user-service \
  --min-capacity 2 \
  --max-capacity 10

aws application-autoscaling put-scaling-policy \
  --service-namespace ecs \
  --scalable-dimension ecs:service:DesiredCount \
  --resource-id service/event-driven-platform-cluster/user-service \
  --policy-name cpu-scaling \
  --policy-type TargetTrackingScaling \
  --target-tracking-scaling-policy-configuration \
  '{
    "TargetValue": 70.0,
    "PredefinedMetricSpecification": {
      "PredefinedMetricType": "ECSServiceAverageCPUUtilization"
    }
  }'
```

## Troubleshooting

### Common Issues

#### 1. Service Not Starting

**Check logs:**
```bash
aws logs tail /ecs/event-driven-platform/user-service --follow
```

**Check task health:**
```bash
aws ecs describe-services \
  --cluster event-driven-platform-cluster \
  --services user-service
```

#### 2. Database Connection Issues

**Verify security groups:**
```bash
aws ec2 describe-security-groups \
  --group-ids sg-xxxxxxxxx
```

**Test connection:**
```bash
psql -h <RDS_ENDPOINT> -U admin -d microservices_db
```

#### 3. Kafka Connection Issues

**Get Kafka brokers:**
```bash
aws kafka get-bootstrap-brokers \
  --cluster-arn <MSK_CLUSTER_ARN>
```

**Verify MSK cluster:**
```bash
aws kafka describe-cluster \
  --cluster-arn <MSK_CLUSTER_ARN>
```

#### 4. High Costs

**Review resource usage:**
```bash
aws ce get-cost-and-usage \
  --time-period Start=2024-01-01,End=2024-01-31 \
  --granularity MONTHLY \
  --metrics BlendedCost \
  --group-by Type=SERVICE
```

**Cost optimization tips:**
- Use Fargate Spot for non-critical workloads
- Enable RDS instance auto-scaling
- Use single NAT gateway for dev/staging
- Enable ElastiCache automatic backups only for prod
- Use t3/t4g instances for lower environments

## Rollback

### Rollback ECS Service

```bash
aws ecs update-service \
  --cluster event-driven-platform-cluster \
  --service user-service \
  --task-definition user-service:PREVIOUS_VERSION \
  --force-new-deployment
```

### Rollback Terraform Changes

```bash
cd infrastructure/terraform
terraform apply -target=<resource> -var="version=previous"
```

## Disaster Recovery

### Backup Strategy

1. **RDS Automated Backups**: Enabled (7-day retention)
2. **Manual Snapshots**: Create before major changes
3. **Configuration Backups**: Store in S3

### Recovery Steps

1. **Database Recovery**
   ```bash
   aws rds restore-db-instance-from-db-snapshot \
     --db-instance-identifier event-driven-platform-postgres-restored \
     --db-snapshot-identifier <snapshot-id>
   ```

2. **Service Recovery**
   - Redeploy from last known good commit
   - Use GitHub Actions to trigger deployment

## Security Best Practices

1. **Rotate Credentials Regularly**
2. **Enable AWS GuardDuty**
3. **Enable AWS Security Hub**
4. **Use AWS Secrets Manager** for sensitive data
5. **Enable VPC Flow Logs**
6. **Regular security audits** with AWS Inspector
7. **Enable MFA** for AWS console access
8. **Use least privilege** IAM policies

## Cost Optimization

### Estimated Monthly Costs (us-east-1)

- **ECS Fargate** (5 services, 2 tasks each): ~$150
- **RDS PostgreSQL** (db.t3.medium): ~$75
- **ElastiCache Redis** (cache.t3.medium): ~$50
- **MSK Kafka** (3x kafka.m5.large): ~$450
- **NAT Gateway**: ~$45
- **Application Load Balancer**: ~$25
- **Data Transfer**: ~$50
- **CloudWatch Logs**: ~$10

**Total**: ~$855/month

### Cost Reduction Tips

1. Use Reserved Instances for predictable workloads
2. Enable Savings Plans
3. Use Fargate Spot (up to 70% savings)
4. Implement auto-scaling aggressively
5. Use S3 Intelligent-Tiering for logs
6. Review and delete unused resources

## Maintenance

### Regular Tasks

- **Weekly**: Review CloudWatch alarms and metrics
- **Monthly**: Review costs and optimize resources
- **Quarterly**: Update dependencies and security patches
- **Annually**: Review and update architecture

---

For questions or issues, please open a GitHub issue or contact the team.

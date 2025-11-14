# Payment System - Production-Ready PayPal Clone

A complete, production-ready payment system built with microservices architecture, similar to PayPal. Built with FastAPI, PostgreSQL, MongoDB, Redis, Kafka, and deployed on AWS with full CI/CD pipeline.

[![CI/CD](https://github.com/username/payment-system/workflows/CI-CD%20Pipeline/badge.svg)](https://github.com/username/payment-system/actions)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

## 🚀 Features

### Core Features
- ✅ **User Management**: Registration, authentication, profile management
- ✅ **Wallet System**: Multi-currency wallet support with real-time balance tracking
- ✅ **Transactions**: Deposits, withdrawals, transfers, and payments
- ✅ **Payment Gateway**: External payment processing (simulated Stripe/PayPal integration)
- ✅ **Notifications**: Event-driven email/SMS notifications
- ✅ **API Gateway**: Centralized routing with rate limiting and authentication

### Technical Features
- ✅ **Microservices Architecture**: 6 independent services
- ✅ **Event-Driven**: Kafka for asynchronous event streaming
- ✅ **Caching**: Redis for high-performance data access
- ✅ **Audit Logging**: MongoDB for comprehensive transaction logs
- ✅ **Monitoring**: Prometheus + Grafana for real-time metrics
- ✅ **Security**: JWT authentication, rate limiting, password hashing
- ✅ **Containerization**: Docker + Docker Compose
- ✅ **CI/CD**: GitHub Actions for automated testing and deployment
- ✅ **Cloud-Ready**: AWS ECS deployment with auto-scaling
- ✅ **Infrastructure as Code**: Terraform for AWS provisioning

## 📋 Table of Contents

- [Architecture](#-architecture)
- [Tech Stack](#-tech-stack)
- [Prerequisites](#-prerequisites)
- [Quick Start](#-quick-start)
- [Local Development](#-local-development)
- [API Documentation](#-api-documentation)
- [Testing](#-testing)
- [Deployment](#-deployment)
- [Monitoring](#-monitoring)
- [Security](#-security)
- [Contributing](#-contributing)

## 🏗 Architecture

```
├── api-gateway/              # API Gateway service
├── services/
│   ├── user-service/        # User management & authentication
│   ├── wallet-service/      # Wallet & balance management
│   ├── transaction-service/ # Transaction processing
│   ├── payment-gateway/     # External payment integration
│   └── notification-service/# Event-driven notifications
├── shared/                  # Shared utilities and models
├── infrastructure/          # Monitoring configurations
│   ├── prometheus/
│   └── grafana/
├── deployments/             # Deployment configurations
│   ├── terraform/          # AWS infrastructure
│   └── kubernetes/         # K8s manifests
├── docs/                   # Documentation
│   ├── architecture.md
│   ├── class-diagram.md
│   └── sequence-diagrams.md
└── docker-compose.yml      # Local development setup
```

See [Architecture Documentation](docs/architecture.md) for detailed system design.

## 🛠 Tech Stack

### Backend
- **Language**: Python 3.11
- **Framework**: FastAPI
- **ORM**: SQLAlchemy
- **Validation**: Pydantic

### Databases
- **PostgreSQL 15**: Transactional data (users, wallets, transactions)
- **MongoDB 7**: Audit logs and documents
- **Redis 7**: Caching and session management

### Message Queue
- **Apache Kafka**: Event streaming
- **Zookeeper**: Kafka coordination

### Monitoring
- **Prometheus**: Metrics collection
- **Grafana**: Visualization and dashboards

### Infrastructure
- **Docker**: Containerization
- **AWS ECS**: Container orchestration
- **AWS RDS**: Managed PostgreSQL
- **AWS ElastiCache**: Managed Redis
- **Terraform**: Infrastructure as Code
- **GitHub Actions**: CI/CD pipeline

## 📦 Prerequisites

### For Local Development
- Docker Desktop 20.10+
- Docker Compose 2.0+
- Python 3.11+ (for local testing)
- Git

### For AWS Deployment
- AWS Account
- AWS CLI configured
- Terraform 1.0+
- kubectl (for Kubernetes deployment)

## 🚀 Quick Start

### 1. Clone the Repository
```bash
git clone https://github.com/username/payment-system.git
cd payment-system
```

### 2. Start All Services
```bash
docker-compose up -d
```

This will start:
- API Gateway (http://localhost:8000)
- User Service (http://localhost:8001)
- Wallet Service (http://localhost:8002)
- Transaction Service (http://localhost:8003)
- Payment Gateway (http://localhost:8004)
- PostgreSQL (localhost:5432)
- MongoDB (localhost:27017)
- Redis (localhost:6379)
- Kafka (localhost:9092)
- Prometheus (http://localhost:9090)
- Grafana (http://localhost:3000)

### 3. Verify Services
```bash
# Check all containers are running
docker-compose ps

# Health check
curl http://localhost:8000/health
```

### 4. Access Dashboards
- **API Documentation**: http://localhost:8000/docs
- **Grafana Dashboard**: http://localhost:3000 (admin/admin)
- **Prometheus**: http://localhost:9090

## 💻 Local Development

### Setting Up Development Environment

```bash
# Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies for all services
pip install -r services/user-service/requirements.txt
pip install -r services/wallet-service/requirements.txt
pip install -r services/transaction-service/requirements.txt
```

### Running Individual Services

```bash
# Terminal 1: Start infrastructure
docker-compose up postgres mongodb redis kafka -d

# Terminal 2: User Service
cd services/user-service
uvicorn main:app --reload --port 8001

# Terminal 3: Wallet Service
cd services/wallet-service
uvicorn main:app --reload --port 8002

# Terminal 4: Transaction Service
cd services/transaction-service
uvicorn main:app --reload --port 8003
```

### Environment Variables

Create `.env` file in the root directory:

```env
# Database
POSTGRES_USER=paymentuser
POSTGRES_PASSWORD=paymentpass
POSTGRES_HOST=localhost
POSTGRES_PORT=5432
POSTGRES_DB=payment_system

# MongoDB
MONGO_USER=mongouser
MONGO_PASSWORD=mongopass
MONGO_HOST=localhost
MONGO_PORT=27017
MONGO_DB=payment_logs

# Redis
REDIS_HOST=localhost
REDIS_PORT=6379
REDIS_DB=0

# Kafka
KAFKA_BOOTSTRAP_SERVERS=localhost:9092

# JWT
JWT_SECRET_KEY=your-super-secret-jwt-key-change-in-production

# Service URLs
USER_SERVICE_URL=http://localhost:8001
WALLET_SERVICE_URL=http://localhost:8002
TRANSACTION_SERVICE_URL=http://localhost:8003
PAYMENT_GATEWAY_URL=http://localhost:8004
```

## 📚 API Documentation

### Interactive API Documentation

Once services are running, visit:
- API Gateway: http://localhost:8000/docs
- User Service: http://localhost:8001/docs
- Wallet Service: http://localhost:8002/docs
- Transaction Service: http://localhost:8003/docs

### Sample API Calls

#### 1. Register User
```bash
curl -X POST http://localhost:8000/users/register \
  -H "Content-Type: application/json" \
  -d '{
    "email": "user@example.com",
    "password": "SecurePass123",
    "first_name": "John",
    "last_name": "Doe",
    "phone": "+1234567890",
    "role": "user"
  }'
```

#### 2. Login
```bash
curl -X POST http://localhost:8000/users/login \
  -H "Content-Type: application/json" \
  -d '{
    "email": "user@example.com",
    "password": "SecurePass123"
  }'
```

#### 3. Create Wallet
```bash
curl -X POST http://localhost:8000/wallets/create \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "user_id": "USER_ID",
    "currency": "USD"
  }'
```

#### 4. Deposit Funds
```bash
curl -X POST http://localhost:8000/transactions/create \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "transaction_type": "deposit",
    "amount": 100.00,
    "currency": "USD",
    "to_wallet_id": "WALLET_ID",
    "payment_method": "credit_card",
    "description": "Initial deposit"
  }'
```

#### 5. Transfer Funds
```bash
curl -X POST http://localhost:8000/transactions/create \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "transaction_type": "transfer",
    "amount": 50.00,
    "currency": "USD",
    "from_wallet_id": "SENDER_WALLET_ID",
    "to_wallet_id": "RECEIVER_WALLET_ID",
    "description": "Payment for services"
  }'
```

## 🧪 Testing

### Run Unit Tests
```bash
# Install test dependencies
pip install pytest pytest-asyncio pytest-cov httpx

# Run tests for each service
pytest services/user-service/tests/
pytest services/wallet-service/tests/
pytest services/transaction-service/tests/
```

### Run Integration Tests
```bash
# Start test environment
docker-compose -f docker-compose.test.yml up -d

# Run integration tests
pytest tests/integration/
```

### Load Testing
```bash
# Install locust
pip install locust

# Run load tests
locust -f tests/load/locustfile.py --host http://localhost:8000
```

## 🚢 Deployment

### Deploy to AWS ECS

#### 1. Configure AWS Credentials
```bash
aws configure
```

#### 2. Provision Infrastructure with Terraform
```bash
cd deployments/terraform

# Initialize Terraform
terraform init

# Review plan
terraform plan

# Apply infrastructure
terraform apply
```

#### 3. Build and Push Docker Images
```bash
# Login to ECR
aws ecr get-login-password --region us-east-1 | docker login --username AWS --password-stdin YOUR_ACCOUNT.dkr.ecr.us-east-1.amazonaws.com

# Build and push all services
for service in user-service wallet-service transaction-service payment-gateway notification-service api-gateway; do
  docker build -t $service -f services/$service/Dockerfile .
  docker tag $service:latest YOUR_ACCOUNT.dkr.ecr.us-east-1.amazonaws.com/$service:latest
  docker push YOUR_ACCOUNT.dkr.ecr.us-east-1.amazonaws.com/$service:latest
done
```

#### 4. Deploy to ECS
```bash
# Update ECS services
aws ecs update-service --cluster payment-system-cluster --service user-service --force-new-deployment
aws ecs update-service --cluster payment-system-cluster --service wallet-service --force-new-deployment
aws ecs update-service --cluster payment-system-cluster --service transaction-service --force-new-deployment
aws ecs update-service --cluster payment-system-cluster --service api-gateway --force-new-deployment
```

### CI/CD with GitHub Actions

The repository includes automated CI/CD pipeline:

1. **On Pull Request**: Run tests and linting
2. **On Push to Main**: Build, test, and deploy to AWS

#### Setup GitHub Secrets
```
AWS_ACCESS_KEY_ID
AWS_SECRET_ACCESS_KEY
```

### Auto-Scaling Configuration

Auto-scaling is configured in Terraform:
- **Min Replicas**: 2
- **Max Replicas**: 10
- **CPU Target**: 70%
- **Memory Target**: 80%

The system automatically scales based on:
- Request rate
- CPU utilization
- Memory utilization
- Custom CloudWatch metrics

## 📊 Monitoring

### Access Grafana Dashboard

1. Open http://localhost:3000 (local) or your Grafana URL (production)
2. Login with admin/admin
3. Navigate to Dashboards → Payment System

### Key Metrics Monitored

- **Request Metrics**: Total requests, requests per second, error rate
- **Latency**: p50, p95, p99 response times
- **Service Health**: Uptime, health check status
- **Database**: Connection pool usage, query performance
- **Cache**: Hit/miss rates, eviction rates
- **Kafka**: Consumer lag, message throughput

### Setting Up Alerts

Configure alerts in `infrastructure/grafana/alerts/`:
- High error rate (> 5%)
- High latency (p95 > 1s)
- Service downtime
- Database connection errors
- High memory usage (> 90%)

## 🔒 Security

### Implemented Security Measures

1. **Authentication**: JWT-based with token expiration
2. **Password Security**: Bcrypt hashing with salt
3. **Rate Limiting**: 100 requests/minute per IP
4. **Input Validation**: Pydantic models for all inputs
5. **SQL Injection Protection**: SQLAlchemy ORM
6. **CORS**: Configured for specific origins
7. **HTTPS**: TLS/SSL in production
8. **Secrets Management**: AWS Secrets Manager
9. **Database Encryption**: Encryption at rest
10. **Audit Logging**: All transactions logged to MongoDB

### Security Best Practices

- Change default passwords and secrets
- Use AWS Secrets Manager for sensitive data
- Enable MFA for AWS account
- Regularly update dependencies
- Monitor security advisories
- Implement Web Application Firewall (WAF)
- Use VPC for network isolation

## 📈 Performance

### Expected Performance Metrics

- **Throughput**: 1000+ requests/second
- **Latency**: < 100ms (p95)
- **Availability**: 99.9% uptime
- **Cache Hit Rate**: > 80%

### Optimization Tips

1. **Database Indexing**: Ensure proper indexes on frequently queried fields
2. **Connection Pooling**: Configured in all services
3. **Caching Strategy**: 5-30 minute TTL based on data type
4. **Async Processing**: Use Kafka for non-critical operations
5. **CDN**: Use CloudFront for static assets

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Style

- Follow PEP 8 for Python code
- Use type hints
- Write docstrings for all functions
- Add unit tests for new features

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- FastAPI for the excellent web framework
- PostgreSQL, MongoDB, and Redis communities
- Apache Kafka team
- Prometheus and Grafana projects

## 📞 Support

For issues and questions:
- Create an issue on GitHub
- Email: support@paymentsystem.com
- Documentation: [docs/](docs/)

## 🗺 Roadmap

- [ ] Implement fraud detection system
- [ ] Add support for cryptocurrency payments
- [ ] Implement recurring payments
- [ ] Add multi-factor authentication
- [ ] Create mobile SDK
- [ ] Implement GraphQL API
- [ ] Add support for international payments
- [ ] Implement dispute resolution system
- [ ] Add advanced analytics dashboard
- [ ] Create webhooks for third-party integrations

---

**Built with ❤️ using FastAPI and modern cloud technologies**

# Event-Driven Microservices Platform

A comprehensive, production-ready event-driven microservices platform built with modern technologies, featuring complete observability, scalability, and cloud-native deployment.

## 🏗️ Architecture Overview

This platform demonstrates a complete event-driven microservices architecture with:

- **5 Microservices** communicating asynchronously via Kafka
- **Multiple databases** (PostgreSQL, MongoDB) with Redis caching
- **React frontend** for user interaction
- **Complete observability** with Prometheus, Grafana, and ELK stack
- **AWS cloud deployment** with Terraform IaC
- **CI/CD pipeline** with GitHub Actions

### System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         React Frontend                          │
│                      (Port 3000 / 80)                           │
└──────────────┬──────────────────────────────────────────────────┘
               │
               ├──────────┬──────────┬──────────┬──────────┐
               │          │          │          │          │
               ▼          ▼          ▼          ▼          ▼
         ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌──────────┐ ┌──────────┐
         │  User   │ │Product  │ │  Order  │ │Inventory │ │Notification│
         │ Service │ │ Service │ │ Service │ │ Service  │ │  Service  │
         │  :8001  │ │  :8002  │ │  :8003  │ │  :8004   │ │   :8005   │
         └────┬────┘ └────┬────┘ └────┬────┘ └────┬─────┘ └─────┬────┘
              │           │           │           │             │
         PostgreSQL  PostgreSQL  PostgreSQL   MongoDB         Kafka
              │           │           │           │         (Events)
              └───────────┴───────────┴───────────┴─────────────┘
                                  │
                            Redis (Cache)
                                  │
              ┌───────────────────┼───────────────────┐
              │                   │                   │
         Prometheus           ELK Stack          Grafana
         (Metrics)           (Logging)       (Visualization)
```

## 📦 Microservices

### 1. User Service (Port 8001)
- **Technology**: FastAPI + PostgreSQL + Redis
- **Features**:
  - User registration and management
  - Password hashing with bcrypt
  - JWT token generation
  - Redis caching for user data
  - Kafka event publishing (user_created, user_updated, user_deleted)

### 2. Product Service (Port 8002)
- **Technology**: FastAPI + PostgreSQL + Redis
- **Features**:
  - Product catalog management
  - Category-based filtering
  - SKU-based unique identification
  - Redis caching for frequently accessed products
  - Kafka event publishing (product_created, product_updated)

### 3. Order Service (Port 8003)
- **Technology**: FastAPI + PostgreSQL + Kafka Producer
- **Features**:
  - Order creation with multiple items
  - Inter-service communication (calls User & Product services)
  - Order status management (pending, processing, completed, cancelled)
  - Kafka event publishing (order_created, order_status_updated)
  - Transaction management for order items

### 4. Inventory Service (Port 8004)
- **Technology**: FastAPI + MongoDB + Kafka Consumer
- **Features**:
  - Real-time inventory tracking
  - Stock reservation system
  - Kafka event consumption (order events)
  - Low stock monitoring
  - MongoDB for flexible inventory schema

### 5. Notification Service (Port 8005)
- **Technology**: FastAPI + Kafka Consumer
- **Features**:
  - Multi-event consumption (user, product, order events)
  - Real-time notification generation
  - Event-driven notification dispatching
  - In-memory notification store (extendable to email/SMS/push)

## 🚀 Technologies Used

### Backend
- **Framework**: FastAPI (Python 3.11)
- **Databases**:
  - PostgreSQL 15 (User, Product, Order services)
  - MongoDB 7 (Inventory service)
- **Caching**: Redis 7
- **Message Broker**: Apache Kafka 3.5
- **ORM**: SQLAlchemy (async)
- **MongoDB Client**: Motor (async)

### Frontend
- **Framework**: React 18
- **Routing**: React Router DOM
- **HTTP Client**: Axios
- **Build Tool**: Create React App

### Infrastructure & DevOps
- **Containerization**: Docker & Docker Compose
- **Orchestration**: AWS ECS Fargate
- **IaC**: Terraform
- **CI/CD**: GitHub Actions
- **Monitoring**: Prometheus + Grafana
- **Logging**: ELK Stack (Elasticsearch, Logstash, Kibana)
- **Cloud**: AWS (ECS, RDS, ElastiCache, MSK, ECR, ALB)

## 📋 Prerequisites

- Docker & Docker Compose (v20+)
- Python 3.11+
- Node.js 18+
- AWS CLI (for deployment)
- Terraform (for infrastructure)

## 🏃 Quick Start

### Local Development with Docker Compose

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd event-driven-platform
   ```

2. **Start all services**
   ```bash
   docker-compose up -d
   ```

3. **Access the applications**
   - Frontend: http://localhost:3000
   - User Service: http://localhost:8001/docs
   - Product Service: http://localhost:8002/docs
   - Order Service: http://localhost:8003/docs
   - Inventory Service: http://localhost:8004/docs
   - Notification Service: http://localhost:8005/docs
   - Prometheus: http://localhost:9090
   - Grafana: http://localhost:3001 (admin/admin123)
   - Kibana: http://localhost:5601

4. **View logs**
   ```bash
   docker-compose logs -f [service-name]
   ```

5. **Stop all services**
   ```bash
   docker-compose down
   ```

### Manual Service Startup (Development)

#### User Service
```bash
cd user-service
pip install -r requirements.txt
uvicorn app.main:app --reload --port 8001
```

#### Product Service
```bash
cd product-service
pip install -r requirements.txt
uvicorn app.main:app --reload --port 8002
```

#### Order Service
```bash
cd order-service
pip install -r requirements.txt
uvicorn app.main:app --reload --port 8003
```

#### Inventory Service
```bash
cd inventory-service
pip install -r requirements.txt
uvicorn app.main:app --reload --port 8004
```

#### Notification Service
```bash
cd notification-service
pip install -r requirements.txt
uvicorn app.main:app --reload --port 8005
```

#### Frontend
```bash
cd frontend
npm install
npm start
```

## 🌩️ AWS Deployment

### Prerequisites
1. AWS account with appropriate permissions
2. AWS CLI configured
3. Terraform installed
4. GitHub repository with secrets configured

### Deployment Steps

1. **Configure Terraform variables**
   ```bash
   cd infrastructure/terraform
   cp terraform.tfvars.example terraform.tfvars
   # Edit terraform.tfvars with your values
   ```

2. **Initialize Terraform**
   ```bash
   terraform init
   ```

3. **Plan infrastructure**
   ```bash
   terraform plan
   ```

4. **Apply infrastructure**
   ```bash
   terraform apply
   ```

5. **Configure GitHub Secrets**
   Add the following secrets to your GitHub repository:
   - `AWS_ACCESS_KEY_ID`
   - `AWS_SECRET_ACCESS_KEY`
   - `AWS_ACCOUNT_ID`
   - `DB_PASSWORD`
   - `MONGODB_PASSWORD`

6. **Push to trigger CI/CD**
   ```bash
   git push origin main
   ```

## 📊 Monitoring & Observability

### Prometheus Metrics
Each service exposes metrics at `/metrics` endpoint:
- Request count and duration
- Service-specific metrics (users created, orders processed, etc.)
- System metrics (CPU, memory)

### Grafana Dashboards
Access Grafana at `http://localhost:3001`:
- Service health dashboard
- Request rate and latency
- Database connection pools
- Kafka consumer lag

### Centralized Logging (ELK)
Access Kibana at `http://localhost:5601`:
- Aggregated logs from all services
- Log level filtering
- Full-text search
- Custom visualizations

## 🔒 Security Features

- Password hashing with bcrypt
- JWT token authentication
- CORS configuration
- SQL injection prevention (parameterized queries)
- Input validation with Pydantic
- Environment variable management
- Secrets management (AWS Secrets Manager in production)

## 🧪 Testing

### Run Backend Tests
```bash
cd user-service
pytest tests/ --cov=app
```

### Run Frontend Tests
```bash
cd frontend
npm test
```

## 📈 Scalability

- **Horizontal scaling**: All services are stateless and can scale independently
- **Auto-scaling**: ECS services configured with target tracking scaling
- **Database**: RDS with read replicas (configurable)
- **Caching**: Redis reduces database load
- **Asynchronous processing**: Kafka enables decoupled, scalable communication

## 🏗️ Project Structure

```
event-driven-platform/
├── user-service/              # User management microservice
├── product-service/           # Product catalog microservice
├── order-service/             # Order processing microservice
├── inventory-service/         # Inventory management microservice
├── notification-service/      # Notification microservice
├── frontend/                  # React web application
├── infrastructure/
│   ├── docker/               # Docker configuration
│   └── terraform/            # AWS infrastructure as code
├── monitoring/
│   ├── prometheus/           # Metrics collection
│   └── grafana/             # Metrics visualization
├── logging/                  # ELK stack configuration
├── .github/
│   └── workflows/           # CI/CD pipelines
└── docker-compose.yml       # Local development orchestration
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📝 API Documentation

Each service provides interactive API documentation via Swagger UI:
- User Service: http://localhost:8001/docs
- Product Service: http://localhost:8002/docs
- Order Service: http://localhost:8003/docs
- Inventory Service: http://localhost:8004/docs
- Notification Service: http://localhost:8005/docs

## 🐛 Troubleshooting

### Kafka Connection Issues
```bash
# Check Kafka is running
docker-compose ps kafka

# View Kafka logs
docker-compose logs kafka
```

### Database Connection Issues
```bash
# Check PostgreSQL
docker-compose ps postgres

# Check MongoDB
docker-compose ps mongodb
```

### Service Health Checks
```bash
curl http://localhost:8001/health
curl http://localhost:8002/health
curl http://localhost:8003/health
curl http://localhost:8004/health
curl http://localhost:8005/health
```

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 👥 Authors

- Your Name - Initial work

## 🙏 Acknowledgments

- FastAPI for the excellent async framework
- Apache Kafka for reliable event streaming
- AWS for cloud infrastructure
- The open-source community

## 🔮 Future Enhancements

- [ ] Add authentication/authorization middleware
- [ ] Implement rate limiting
- [ ] Add GraphQL API gateway
- [ ] Implement CQRS pattern
- [ ] Add event sourcing
- [ ] Implement saga pattern for distributed transactions
- [ ] Add service mesh (Istio)
- [ ] Implement blue-green deployment
- [ ] Add chaos engineering tests
- [ ] Implement multi-region deployment

# Architecture Documentation

## System Overview

This document describes the architecture of the Event-Driven Microservices Platform, a production-ready system demonstrating modern cloud-native patterns and best practices.

## Core Architectural Principles

1. **Microservices Architecture**: Services are independently deployable and scalable
2. **Event-Driven Communication**: Asynchronous messaging via Kafka for loose coupling
3. **Database per Service**: Each service owns its data store
4. **API Gateway Pattern**: Centralized entry point (ALB) for external requests
5. **CQRS Ready**: Separation of concerns enables easy CQRS implementation
6. **Observability First**: Built-in monitoring, logging, and tracing

## System Components

### Microservices

#### 1. User Service
**Technology Stack**: FastAPI + PostgreSQL + Redis

**Responsibilities**:
- User registration and authentication
- User profile management
- Password hashing and validation
- JWT token generation

**Database Schema**:
```sql
users (
  id SERIAL PRIMARY KEY,
  username VARCHAR(100) UNIQUE NOT NULL,
  email VARCHAR(255) UNIQUE NOT NULL,
  password_hash VARCHAR(255) NOT NULL,
  full_name VARCHAR(255),
  is_active BOOLEAN DEFAULT TRUE,
  created_at TIMESTAMP,
  updated_at TIMESTAMP
)
```

**Events Published**:
- `user_created`: When a new user registers
- `user_updated`: When user profile is updated
- `user_deleted`: When a user is deleted

**Caching Strategy**:
- User profile data cached with 1-hour TTL
- Cache invalidation on update/delete

#### 2. Product Service
**Technology Stack**: FastAPI + PostgreSQL + Redis

**Responsibilities**:
- Product catalog management
- Category management
- SKU-based inventory tracking
- Product search and filtering

**Database Schema**:
```sql
products (
  id SERIAL PRIMARY KEY,
  name VARCHAR(255) NOT NULL,
  description TEXT,
  price NUMERIC(10, 2) NOT NULL,
  category VARCHAR(100),
  sku VARCHAR(100) UNIQUE NOT NULL,
  is_active BOOLEAN DEFAULT TRUE,
  created_at TIMESTAMP,
  updated_at TIMESTAMP
)
```

**Events Published**:
- `product_created`: When a new product is added
- `product_updated`: When product details change
- `product_deleted`: When a product is removed

**Caching Strategy**:
- Product details cached with 1-hour TTL
- Category-based query caching

#### 3. Order Service
**Technology Stack**: FastAPI + PostgreSQL + Kafka Producer

**Responsibilities**:
- Order creation and management
- Order status tracking
- Integration with User and Product services
- Transaction management

**Database Schema**:
```sql
orders (
  id SERIAL PRIMARY KEY,
  user_id INTEGER NOT NULL,
  total_amount NUMERIC(10, 2) NOT NULL,
  status VARCHAR(50) DEFAULT 'pending',
  created_at TIMESTAMP,
  updated_at TIMESTAMP
)

order_items (
  id SERIAL PRIMARY KEY,
  order_id INTEGER REFERENCES orders(id),
  product_id INTEGER NOT NULL,
  quantity INTEGER NOT NULL,
  unit_price NUMERIC(10, 2) NOT NULL,
  subtotal NUMERIC(10, 2) NOT NULL,
  created_at TIMESTAMP
)
```

**Events Published**:
- `order_created`: When a new order is placed
- `order_status_updated`: When order status changes

**Inter-Service Communication**:
- Synchronous HTTP calls to User Service for validation
- Synchronous HTTP calls to Product Service for pricing

#### 4. Inventory Service
**Technology Stack**: FastAPI + MongoDB + Kafka Consumer

**Responsibilities**:
- Real-time inventory tracking
- Stock reservation
- Low stock alerts
- Warehouse location management

**MongoDB Schema**:
```javascript
{
  product_id: Number,
  quantity: Number,
  reserved: Number,
  warehouse_location: String,
  last_updated: Timestamp
}
```

**Events Consumed**:
- `order_created`: Reserves inventory for order items
- `order_status_updated`: Updates inventory on order completion

**Why MongoDB?**:
- Flexible schema for varied inventory attributes
- High write throughput for real-time updates
- Easy horizontal scaling

#### 5. Notification Service
**Technology Stack**: FastAPI + Kafka Consumer

**Responsibilities**:
- Event consumption from multiple topics
- Notification generation and dispatching
- Multi-channel delivery (email, SMS, push - extensible)

**Events Consumed**:
- All events from User, Product, and Order topics
- Transforms events into user-facing notifications

**Storage**:
- In-memory queue for recent notifications
- Extensible to persistent storage (PostgreSQL/MongoDB)

## Data Flow

### Order Creation Flow

```
1. User submits order via Frontend
   ↓
2. Frontend → Order Service (HTTP POST)
   ↓
3. Order Service validates:
   - User exists (HTTP GET → User Service)
   - Products exist (HTTP GET → Product Service)
   ↓
4. Order Service creates order in PostgreSQL
   ↓
5. Order Service publishes 'order_created' event to Kafka
   ↓
6. Kafka broadcasts event to subscribers
   ↓
7. Inventory Service consumes event
   - Reserves stock in MongoDB
   ↓
8. Notification Service consumes event
   - Generates order confirmation notification
   ↓
9. Order Service returns order details to Frontend
```

## Event Schema

### Standard Event Structure

```json
{
  "event_type": "string",
  "timestamp": "ISO8601",
  "event_id": "uuid",
  "version": "1.0",
  "payload": {
    // Event-specific data
  }
}
```

### Example: Order Created Event

```json
{
  "event_type": "order_created",
  "order_id": 123,
  "user_id": 456,
  "total_amount": 299.99,
  "items": [
    {
      "product_id": 789,
      "quantity": 2,
      "unit_price": 149.99
    }
  ],
  "timestamp": "2024-01-15T10:30:00Z"
}
```

## Infrastructure Architecture

### AWS Deployment

```
┌─────────────────────────────────────────────────┐
│                   Route 53                      │
│              (DNS Management)                    │
└────────────────────┬────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│          Application Load Balancer              │
│         (Public Subnets - Multi-AZ)             │
└────────────────────┬────────────────────────────┘
                     ↓
        ┌────────────┴────────────┐
        ↓                         ↓
┌──────────────┐         ┌──────────────┐
│   ECS Task   │         │   ECS Task   │
│  (Service 1) │   ...   │  (Service 5) │
│Private Subnet│         │Private Subnet│
└──────┬───────┘         └──────┬───────┘
       │                        │
       └────────┬───────────────┘
                ↓
    ┌───────────────────────┐
    │   RDS PostgreSQL      │
    │   ElastiCache Redis   │
    │   MSK Kafka           │
    │   (Private Subnets)   │
    └───────────────────────┘
```

### Network Architecture

**VPC Layout**:
- CIDR: 10.0.0.0/16
- 3 Availability Zones
- 3 Public Subnets (10.0.101.0/24, 10.0.102.0/24, 10.0.103.0/24)
- 3 Private Subnets (10.0.1.0/24, 10.0.2.0/24, 10.0.3.0/24)

**Security Groups**:
- ALB Security Group: Allows 80/443 from internet
- ECS Tasks Security Group: Allows traffic from ALB
- RDS Security Group: Allows 5432 from ECS tasks
- ElastiCache Security Group: Allows 6379 from ECS tasks
- MSK Security Group: Allows 9092/9094 from ECS tasks

## Observability

### Metrics (Prometheus + Grafana)

**Service-Level Metrics**:
- Request rate (requests/second)
- Error rate (errors/second)
- Request duration (p50, p95, p99)
- Active connections
- Database connection pool stats

**Business Metrics**:
- Users created
- Orders placed
- Products added
- Inventory updates
- Notifications sent

**Infrastructure Metrics**:
- CPU utilization
- Memory utilization
- Network throughput
- Disk I/O

### Logging (ELK Stack)

**Log Aggregation**:
- All services send structured JSON logs
- Logstash processes and enriches logs
- Elasticsearch stores logs (30-day retention)
- Kibana provides search and visualization

**Log Levels**:
- ERROR: Application errors, exceptions
- WARNING: Degraded performance, retries
- INFO: Normal operations, business events
- DEBUG: Detailed debugging information

**Log Format**:
```json
{
  "timestamp": "2024-01-15T10:30:00Z",
  "level": "INFO",
  "service": "order-service",
  "message": "Order created successfully",
  "order_id": 123,
  "user_id": 456,
  "trace_id": "abc123"
}
```

### Tracing (Future Enhancement)

Recommendation: Implement distributed tracing with:
- Jaeger or AWS X-Ray
- OpenTelemetry instrumentation
- Correlation IDs across services

## Scalability

### Horizontal Scaling

**ECS Auto-Scaling**:
- Target CPU: 70%
- Target Memory: 80%
- Min capacity: 2 tasks per service
- Max capacity: 10 tasks per service
- Scale-out cooldown: 60 seconds
- Scale-in cooldown: 300 seconds

**Database Scaling**:
- RDS: Read replicas for read-heavy workloads
- MongoDB: Sharding by product_id for inventory
- Redis: Cluster mode for high throughput

**Kafka Scaling**:
- 3 broker minimum for HA
- Partition by key for parallel processing
- Consumer groups for load distribution

### Vertical Scaling

**Instance Sizing Guide**:
- Development: t3.micro / t3.small
- Staging: t3.medium / t3.large
- Production: m5.large / m5.xlarge
- High traffic: c5.xlarge / c5.2xlarge

## Security

### Authentication & Authorization

**Current Implementation**:
- Password hashing: bcrypt (12 rounds)
- JWT tokens for stateless authentication
- Token expiry: 30 minutes

**Future Enhancements**:
- OAuth 2.0 / OpenID Connect
- Role-Based Access Control (RBAC)
- API key management
- Rate limiting per user

### Network Security

- VPC isolation
- Private subnets for databases
- Security groups (least privilege)
- NACLs for subnet-level filtering
- AWS WAF for ALB (recommended)

### Data Security

- Encryption at rest:
  - RDS: AES-256
  - ElastiCache: Optional
  - MSK: AES-256
  - S3: AES-256
- Encryption in transit:
  - TLS 1.3 for all external traffic
  - TLS 1.2 for internal traffic
  - Kafka: TLS/SSL

### Secrets Management

**Current**: Environment variables
**Recommended**:
- AWS Secrets Manager
- Parameter Store
- Vault by HashiCorp

## Performance Optimization

### Caching Strategy

**L1 Cache (Application Level)**:
- In-memory caching for static data
- TTL: 5-15 minutes

**L2 Cache (Redis)**:
- User profiles: 1 hour TTL
- Product catalog: 1 hour TTL
- Session data: 30 minutes TTL

**Cache Invalidation**:
- Write-through on updates
- Event-based invalidation
- TTL-based expiration

### Database Optimization

**Indexing**:
- Primary keys on all tables
- Foreign keys indexed
- Frequently queried fields indexed
- Composite indexes for common queries

**Connection Pooling**:
- PostgreSQL: 10-20 connections per service
- MongoDB: 10-100 connections
- Redis: 10 connections

**Query Optimization**:
- Use of SELECT specific columns
- Avoid N+1 queries
- Pagination for large datasets
- Prepared statements

## Disaster Recovery

### Backup Strategy

**RDS PostgreSQL**:
- Automated daily backups (7-day retention)
- Manual snapshots before deployments
- Point-in-time recovery enabled

**MongoDB**:
- Automated snapshots (if using DocumentDB)
- Or manual backup scripts

**Configuration**:
- Terraform state in S3 with versioning
- Infrastructure as Code in Git

### High Availability

**Multi-AZ Deployment**:
- ECS tasks distributed across 3 AZs
- RDS Multi-AZ automatic failover
- ElastiCache with automatic failover
- MSK with multi-AZ replication

**Recovery Time Objective (RTO)**: 15 minutes
**Recovery Point Objective (RPO)**: 5 minutes

## Cost Optimization

### Resource Tagging

All resources tagged with:
- Environment (dev/staging/prod)
- Project name
- Service name
- Owner
- Cost center

### Cost Monitoring

- AWS Cost Explorer
- Budget alerts
- Cost anomaly detection
- Resource utilization dashboards

### Optimization Strategies

1. Use Fargate Spot for dev/staging
2. Right-size instances based on metrics
3. Use Reserved Instances for predictable workloads
4. Auto-scaling to match demand
5. S3 lifecycle policies for logs
6. Delete unused resources

## Future Enhancements

### Short-term (3-6 months)
- [ ] Implement API Gateway (AWS API Gateway or Kong)
- [ ] Add distributed tracing
- [ ] Implement rate limiting
- [ ] Add GraphQL API layer
- [ ] Implement CDC (Change Data Capture)

### Medium-term (6-12 months)
- [ ] CQRS pattern implementation
- [ ] Event Sourcing
- [ ] Saga pattern for distributed transactions
- [ ] Service Mesh (Istio/Linkerd)
- [ ] Multi-region deployment

### Long-term (12+ months)
- [ ] Machine learning for demand forecasting
- [ ] Real-time analytics platform
- [ ] Edge computing integration
- [ ] Blockchain for supply chain tracking
- [ ] Advanced fraud detection

## References

- [Microservices Pattern](https://microservices.io/patterns/)
- [Event-Driven Architecture](https://martinfowler.com/articles/201701-event-driven.html)
- [AWS Well-Architected Framework](https://aws.amazon.com/architecture/well-architected/)
- [Twelve-Factor App](https://12factor.net/)
- [CQRS Pattern](https://martinfowler.com/bliki/CQRS.html)

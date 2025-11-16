# System Architecture

## Overview

The API Uptime Monitor is built as a modern, scalable SaaS platform using microservices architecture with the following components:

## Components

### 1. Frontend (Next.js)
**Technology**: Next.js 14 + TypeScript + Tailwind CSS

**Responsibilities:**
- User authentication and session management
- Real-time dashboard with live updates
- Endpoint configuration UI
- Alert configuration interface
- Data visualization (charts, graphs)
- Historical data analysis

**Port**: 3000

**Key Features:**
- Server-side rendering for SEO
- Client-side routing for SPA experience
- Responsive design for mobile/desktop
- Dark mode support

### 2. Backend API (FastAPI)
**Technology**: FastAPI (Python 3.11)

**Responsibilities:**
- RESTful API endpoints
- User authentication (JWT)
- CRUD operations for endpoints, alerts
- Database interactions
- API documentation (Swagger/OpenAPI)

**Port**: 8000

**Key Endpoints:**
- `/api/v1/auth/*` - Authentication
- `/api/v1/endpoints/*` - Endpoint management
- `/api/v1/health-checks/*` - Health check data
- `/api/v1/alerts/*` - Alert configuration

### 3. Celery Workers
**Technology**: Celery with Redis broker

**Responsibilities:**
- Execute scheduled health checks
- Perform HTTP requests to monitored endpoints
- ML model training and inference
- Alert processing and sending

**Tasks:**
- `check_all_endpoints` - Runs every minute
- `cleanup_old_health_checks` - Daily at 2 AM
- `retrain_ml_models` - Daily at 3 AM
- `check_single_endpoint` - On-demand checks

### 4. Celery Beat
**Technology**: Celery Beat scheduler

**Responsibilities:**
- Schedule periodic tasks
- Trigger health checks at configured intervals
- Schedule maintenance tasks
- Coordinate ML model retraining

### 5. PostgreSQL + TimescaleDB
**Technology**: PostgreSQL 15 with TimescaleDB extension

**Responsibilities:**
- Store user data
- Store endpoint configurations
- Store health check results (time-series)
- Store alert configurations
- Store incident records

**Key Tables:**
- `users` - User accounts
- `endpoints` - Monitored endpoints
- `health_checks` - Health check results (hypertable)
- `alert_configs` - Alert configurations
- `incidents` - Incident tracking

**TimescaleDB Optimizations:**
- Hypertable on `health_checks` for efficient time-series queries
- Automatic data retention policies
- Optimized compression for historical data

### 6. Redis
**Technology**: Redis 7

**Responsibilities:**
- Celery message broker
- Task queue management
- Caching layer
- Session storage

**Port**: 6379

### 7. Flower
**Technology**: Flower (Celery monitoring)

**Responsibilities:**
- Monitor Celery workers
- View task execution stats
- Debug failed tasks
- Real-time worker monitoring

**Port**: 5555

## Data Flow

### Health Check Execution Flow

```
1. Celery Beat triggers scheduled task
   ↓
2. Celery Worker picks up task
   ↓
3. Worker executes HTTP request to endpoint
   ↓
4. Worker measures response time, status code
   ↓
5. Worker performs SSL check (if enabled)
   ↓
6. ML model analyzes for anomalies
   ↓
7. Worker saves HealthCheck record to database
   ↓
8. Worker checks for incident conditions
   ↓
9. If incident detected, create Incident record
   ↓
10. Alert service sends notifications (if configured)
```

### User Interaction Flow

```
1. User logs in via Frontend
   ↓
2. Frontend requests JWT token from Backend API
   ↓
3. Backend validates credentials and returns token
   ↓
4. Frontend stores token in localStorage
   ↓
5. User creates new endpoint
   ↓
6. Frontend sends POST to /api/v1/endpoints
   ↓
7. Backend creates Endpoint record in database
   ↓
8. Celery workers start monitoring endpoint
   ↓
9. Frontend polls for health check updates
   ↓
10. Backend returns latest health check data
    ↓
11. Frontend renders charts and statistics
```

## Database Schema

### Users Table
```sql
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    email VARCHAR UNIQUE NOT NULL,
    username VARCHAR UNIQUE NOT NULL,
    hashed_password VARCHAR NOT NULL,
    full_name VARCHAR,
    is_active BOOLEAN DEFAULT true,
    is_superuser BOOLEAN DEFAULT false,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);
```

### Endpoints Table
```sql
CREATE TABLE endpoints (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id),
    name VARCHAR NOT NULL,
    url VARCHAR NOT NULL,
    method VARCHAR DEFAULT 'GET',
    headers JSONB DEFAULT '{}',
    body JSONB,
    expected_status_codes JSONB DEFAULT '[200]',
    timeout INTEGER DEFAULT 30,
    check_interval INTEGER DEFAULT 300,
    is_active BOOLEAN DEFAULT true,
    verify_ssl BOOLEAN DEFAULT true,
    follow_redirects BOOLEAN DEFAULT true,
    response_time_threshold INTEGER DEFAULT 5000,
    check_ssl_expiry BOOLEAN DEFAULT true,
    ssl_expiry_alert_days INTEGER DEFAULT 7,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW(),
    last_checked_at TIMESTAMP
);
```

### Health Checks Table (TimescaleDB Hypertable)
```sql
CREATE TABLE health_checks (
    id SERIAL,
    endpoint_id INTEGER REFERENCES endpoints(id),
    status_code INTEGER,
    response_time FLOAT,
    is_success BOOLEAN DEFAULT false,
    error_message TEXT,
    ssl_expiry_date TIMESTAMP,
    ssl_days_remaining INTEGER,
    is_anomaly BOOLEAN DEFAULT false,
    anomaly_score FLOAT,
    checked_at TIMESTAMP DEFAULT NOW(),
    PRIMARY KEY (id, checked_at)
);

-- Convert to hypertable for time-series optimization
SELECT create_hypertable('health_checks', 'checked_at');

-- Create index for efficient queries
CREATE INDEX idx_endpoint_checked ON health_checks(endpoint_id, checked_at DESC);
```

## AI/ML Pipeline

### 1. Data Collection
- Continuous collection of health check results
- Minimum 50 data points required for training
- Features extracted:
  - Response time
  - Status code (encoded)
  - Hour of day
  - Day of week
  - Rolling average response time
  - Deviation from average

### 2. Model Training
- Algorithm: Isolation Forest (unsupervised)
- Contamination: 10% (expected anomaly rate)
- Training frequency: Daily at 3 AM
- Storage: Joblib serialized models

### 3. Inference
- Real-time anomaly detection on each health check
- Anomaly score calculation
- Pattern recognition for recurring issues

### 4. Smart Features
- **Dynamic Thresholds**: Auto-adjust based on 95th percentile + 1.5 std
- **Predictive Downtime**: Analyze failure rate + response time trends
- **Alert Intelligence**: Reduce alert fatigue with cooldown periods

## Security Architecture

### Authentication
- JWT tokens with configurable expiry
- Bcrypt password hashing (12 rounds)
- Token-based API authentication

### Authorization
- Multi-tenant architecture (user isolation)
- Row-level security via user_id foreign keys
- API endpoint authorization via dependencies

### Network Security
- HTTPS/TLS for all communications
- SSL certificate verification
- CORS configuration
- Rate limiting (recommended for production)

### Data Security
- Encrypted passwords
- Secure credential storage for alerts
- Environment-based secrets management

## Scalability Considerations

### Horizontal Scaling
- **Frontend**: Deploy multiple Next.js instances behind load balancer
- **Backend**: Run multiple FastAPI instances
- **Celery Workers**: Add workers with `--scale celery-worker=N`
- **Database**: Use read replicas for analytics queries

### Vertical Scaling
- Increase worker resources for ML operations
- Optimize database with proper indexing
- Use Redis clustering for high throughput

### Performance Optimizations
- TimescaleDB for time-series data
- Redis caching for frequent queries
- Async I/O in FastAPI and Celery
- Connection pooling for database
- Batch processing for alerts

## Monitoring & Observability

### Application Monitoring
- Flower for Celery task monitoring
- FastAPI built-in logging
- Custom metrics via Prometheus (future)

### Database Monitoring
- TimescaleDB continuous aggregates
- Query performance tracking
- Connection pool monitoring

### Error Tracking
- Structured logging
- Error aggregation (Sentry integration recommended)
- Alert on critical failures

## Deployment Architecture

### Development
```
Docker Compose:
- All services on single host
- Volume mounts for hot reload
- Debug mode enabled
```

### Production
```
Recommended:
- Kubernetes cluster for orchestration
- Managed PostgreSQL (AWS RDS, Google Cloud SQL)
- Managed Redis (AWS ElastiCache, Redis Cloud)
- Container registry for images
- Load balancer for API and Frontend
- CDN for static assets
```

## Future Enhancements

1. **WebSocket Support**: Real-time dashboard updates
2. **GraphQL API**: Alternative to REST
3. **Multi-region Monitoring**: Distributed health checks
4. **Advanced ML**: LSTM for time-series prediction
5. **API Mocking**: Test endpoint responses
6. **Custom Metrics**: User-defined performance metrics
7. **Compliance**: SOC 2, HIPAA compliance features

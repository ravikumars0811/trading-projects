# Quick Start Guide

Get the Event-Driven Microservices Platform up and running in 5 minutes!

## Prerequisites

- Docker Desktop installed and running
- 8GB RAM minimum (16GB recommended)
- 20GB free disk space

## Step 1: Clone and Start

```bash
# Clone the repository
git clone <repository-url>
cd event-driven-platform

# Start all services
docker-compose up -d

# Wait for services to be healthy (2-3 minutes)
```

## Step 2: Verify Services

Check that all services are running:

```bash
docker-compose ps
```

You should see all services with status "Up" or "Up (healthy)".

## Step 3: Access Applications

Open these URLs in your browser:

### Frontend Application
http://localhost:3000

### API Documentation
- User Service: http://localhost:8001/docs
- Product Service: http://localhost:8002/docs
- Order Service: http://localhost:8003/docs
- Inventory Service: http://localhost:8004/docs
- Notification Service: http://localhost:8005/docs

### Monitoring & Logging
- Prometheus: http://localhost:9090
- Grafana: http://localhost:3001 (admin/admin123)
- Kibana: http://localhost:5601

## Step 4: Try It Out

### Create a User

**Via UI**: Go to http://localhost:3000/users and click "Add User"

**Via API**:
```bash
curl -X POST http://localhost:8001/users \
  -H "Content-Type: application/json" \
  -d '{
    "username": "johndoe",
    "email": "john@example.com",
    "password": "password123",
    "full_name": "John Doe"
  }'
```

### Create a Product

**Via UI**: Go to http://localhost:3000/products and click "Add Product"

**Via API**:
```bash
curl -X POST http://localhost:8002/products \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Laptop",
    "description": "High-performance laptop",
    "price": 999.99,
    "category": "Electronics",
    "sku": "LAP-001"
  }'
```

### Create an Order

**Via UI**:
1. Go to http://localhost:3000/orders
2. Click "Create Order"
3. Select a user and products
4. Submit the order

**Via API**:
```bash
curl -X POST http://localhost:8003/orders \
  -H "Content-Type: application/json" \
  -d '{
    "user_id": 1,
    "items": [
      {
        "product_id": 1,
        "quantity": 2
      }
    ]
  }'
```

### View Notifications

Go to http://localhost:3000/notifications to see all system notifications generated from your actions.

## Step 5: Explore Monitoring

### Prometheus
1. Go to http://localhost:9090
2. Try these queries:
   - `user_service_requests_total` - Total requests to user service
   - `rate(user_service_request_duration_seconds_sum[5m])` - Request rate
   - `order_service_orders_created_total` - Total orders created

### Grafana
1. Go to http://localhost:3001
2. Login with admin/admin123
3. Create a dashboard:
   - Click "+" → "Dashboard"
   - Add panel
   - Select Prometheus as data source
   - Add metrics and visualizations

### Kibana
1. Go to http://localhost:5601
2. Set up index pattern:
   - Go to Management → Stack Management → Index Patterns
   - Create pattern: `microservices-logs-*`
   - Select `@timestamp` as time field
3. View logs:
   - Go to Analytics → Discover
   - Filter by service, level, etc.

## Common Commands

### View Logs
```bash
# All services
docker-compose logs -f

# Specific service
docker-compose logs -f user-service
docker-compose logs -f order-service
```

### Restart Services
```bash
# All services
docker-compose restart

# Specific service
docker-compose restart user-service
```

### Stop All Services
```bash
docker-compose down
```

### Stop and Remove All Data
```bash
docker-compose down -v
```

### Check Service Health
```bash
curl http://localhost:8001/health
curl http://localhost:8002/health
curl http://localhost:8003/health
curl http://localhost:8004/health
curl http://localhost:8005/health
```

## Using the Makefile (Optional)

If you have `make` installed:

```bash
# Start all services
make up

# View logs
make logs

# Check health
make health

# Stop services
make down

# Run tests
make test-all

# Seed sample data
make seed-data
```

See all available commands:
```bash
make help
```

## Architecture Overview

```
Frontend (React)
    ↓
┌───────────────────────────────────────────┐
│  User │ Product │ Order │ Inventory │ Notification
│  :8001│  :8002  │ :8003 │   :8004   │    :8005
└───┬───────┬───────┬───────────┬────────────┬──┘
    │       │       │           │            │
PostgreSQL PostgreSQL PostgreSQL  MongoDB   Kafka
    │       │       │           │      (Events)
    └───────┴───────┴───────────┴────────────┘
              Redis (Cache)
```

## Event Flow Example

When you create an order:
1. **Frontend** sends request to **Order Service**
2. **Order Service**:
   - Validates user (calls **User Service**)
   - Validates products (calls **Product Service**)
   - Creates order in PostgreSQL
   - Publishes `order_created` event to Kafka
3. **Inventory Service** (Kafka consumer):
   - Receives event
   - Reserves stock in MongoDB
4. **Notification Service** (Kafka consumer):
   - Receives event
   - Creates order confirmation notification

## Troubleshooting

### Services Won't Start

```bash
# Check Docker resources
docker system df

# Clean up if needed
docker system prune -a

# Restart Docker Desktop
```

### Can't Access Services

```bash
# Check if ports are in use
lsof -i :3000
lsof -i :8001

# Check service logs
docker-compose logs user-service
```

### Database Connection Errors

```bash
# Wait for databases to be ready
docker-compose up -d postgres mongodb redis
sleep 10
docker-compose up -d
```

### Kafka Connection Issues

```bash
# Restart Kafka and Zookeeper
docker-compose restart zookeeper kafka
sleep 15
docker-compose restart inventory-service notification-service
```

## Next Steps

1. **Explore the Code**: Each service is in its own directory
2. **Read the Docs**: Check `README.md` and `ARCHITECTURE.md`
3. **Customize**: Modify services to fit your needs
4. **Deploy**: Follow `DEPLOYMENT.md` for AWS deployment

## Need Help?

- Check the full documentation in `README.md`
- Review architecture in `ARCHITECTURE.md`
- See deployment guide in `DEPLOYMENT.md`
- Open an issue on GitHub

## Sample Data

The system comes with sample data:
- 2 sample users (john_doe, jane_smith)
- 5 sample products (Laptop, Mouse, Cable, Keyboard, Monitor)

Use these to test the system immediately!

---

**Enjoy building with the Event-Driven Microservices Platform!** 🚀

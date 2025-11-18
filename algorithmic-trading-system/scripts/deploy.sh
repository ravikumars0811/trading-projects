#!/bin/bash
# Deployment script for Algorithmic Trading System

set -e

echo "==================================================="
echo "Deploying Algorithmic Trading System"
echo "==================================================="

# Check if .env file exists
if [ ! -f ".env" ]; then
    echo "Error: .env file not found!"
    echo "Please create .env file with required environment variables:"
    echo "  ALPACA_API_KEY=your_api_key"
    echo "  ALPACA_API_SECRET=your_api_secret"
    echo "  DB_PASSWORD=your_db_password"
    echo "  GRAFANA_PASSWORD=your_grafana_password"
    exit 1
fi

# Load environment variables
export $(cat .env | grep -v '^#' | xargs)

# Build Docker images
echo "Building Docker images..."
docker-compose build

# Start services
echo "Starting services..."
docker-compose up -d

# Wait for services to be ready
echo "Waiting for services to initialize..."
sleep 10

# Check if services are running
echo "Checking service health..."
docker-compose ps

# Display logs
echo ""
echo "Services started successfully!"
echo ""
echo "Access points:"
echo "  Trading System: http://localhost:8080"
echo "  Prometheus: http://localhost:9091"
echo "  Grafana: http://localhost:3000 (user: admin, password: \$GRAFANA_PASSWORD)"
echo ""
echo "To view logs:"
echo "  docker-compose logs -f trading-system"
echo ""
echo "To stop services:"
echo "  docker-compose down"
echo ""

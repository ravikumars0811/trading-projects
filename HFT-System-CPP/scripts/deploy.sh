#!/bin/bash

set -e

echo "Deploying HFT System..."

# Build Docker image
echo "Building Docker image..."
docker-compose -f docker/docker-compose.yml build

# Start the system
echo "Starting HFT System..."
docker-compose -f docker/docker-compose.yml up -d

echo "HFT System deployed successfully!"
echo "View logs with: docker-compose -f docker/docker-compose.yml logs -f"
echo "Stop with: docker-compose -f docker/docker-compose.yml down"

#!/bin/bash
# Watchdog360 Setup Script
# Quick setup for development environment

set -e

echo "╔═══════════════════════════════════════╗"
echo "║   Watchdog360 Development Setup      ║"
echo "╚═══════════════════════════════════════╝"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check prerequisites
echo -e "${YELLOW}Checking prerequisites...${NC}"

# Check Docker
if ! command -v docker &> /dev/null; then
    echo -e "${RED}✗ Docker not found. Please install Docker first.${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Docker found${NC}"

# Check Docker Compose
if ! command -v docker-compose &> /dev/null; then
    echo -e "${RED}✗ Docker Compose not found. Please install Docker Compose first.${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Docker Compose found${NC}"

# Setup environment
echo ""
echo -e "${YELLOW}Setting up environment...${NC}"

cd deployment

if [ ! -f .env ]; then
    cp .env.example .env
    echo -e "${GREEN}✓ Created .env file${NC}"

    # Generate random secret key
    SECRET_KEY=$(openssl rand -hex 32)
    sed -i "s/your-secret-key-change-this/$SECRET_KEY/" .env
    echo -e "${GREEN}✓ Generated SECRET_KEY${NC}"

    # Generate random password
    DB_PASSWORD=$(openssl rand -hex 16)
    sed -i "s/changeme-to-secure-password/$DB_PASSWORD/" .env
    echo -e "${GREEN}✓ Generated DB_PASSWORD${NC}"
else
    echo -e "${YELLOW}⚠ .env already exists, skipping${NC}"
fi

# Create backend .env
cd ../backend
if [ ! -f .env ]; then
    cp .env.example .env
    echo -e "${GREEN}✓ Created backend .env file${NC}"
fi

cd ..

# Start services
echo ""
echo -e "${YELLOW}Starting Docker services...${NC}"

cd deployment
docker-compose up -d

echo ""
echo -e "${GREEN}Waiting for services to be ready...${NC}"
sleep 10

# Check service health
echo ""
echo -e "${YELLOW}Checking service health...${NC}"

if docker-compose ps | grep -q "Up (healthy)"; then
    echo -e "${GREEN}✓ Services are healthy${NC}"
else
    echo -e "${YELLOW}⚠ Services starting... (this may take a moment)${NC}"
fi

# Display status
echo ""
echo "╔═══════════════════════════════════════╗"
echo "║   Setup Complete!                     ║"
echo "╚═══════════════════════════════════════╝"
echo ""
echo "Services:"
echo "  - Frontend:  http://localhost:3000"
echo "  - Backend:   http://localhost:8000"
echo "  - API Docs:  http://localhost:8000/docs"
echo ""
echo "Next steps:"
echo "  1. Visit http://localhost:3000 and register an account"
echo "  2. Install agent on your servers:"
echo "     curl -s http://localhost:8000/install.sh | sudo bash"
echo ""
echo "Useful commands:"
echo "  - View logs:    docker-compose logs -f"
echo "  - Stop:         docker-compose down"
echo "  - Restart:      docker-compose restart"
echo ""
echo -e "${GREEN}Happy monitoring! 🚀${NC}"

.PHONY: help build up down logs clean test deploy-aws

help: ## Show this help message
	@echo 'Usage: make [target]'
	@echo ''
	@echo 'Available targets:'
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'

build: ## Build all Docker images
	docker-compose build

up: ## Start all services
	docker-compose up -d
	@echo "Waiting for services to be healthy..."
	@sleep 10
	@echo "\n=== Services Status ==="
	@docker-compose ps
	@echo "\n=== Access URLs ==="
	@echo "Frontend:           http://localhost:3000"
	@echo "User Service:       http://localhost:8001/docs"
	@echo "Product Service:    http://localhost:8002/docs"
	@echo "Order Service:      http://localhost:8003/docs"
	@echo "Inventory Service:  http://localhost:8004/docs"
	@echo "Notification Service: http://localhost:8005/docs"
	@echo "Prometheus:         http://localhost:9090"
	@echo "Grafana:           http://localhost:3001 (admin/admin123)"
	@echo "Kibana:            http://localhost:5601"

down: ## Stop all services
	docker-compose down

down-v: ## Stop all services and remove volumes
	docker-compose down -v

logs: ## View logs from all services
	docker-compose logs -f

logs-user: ## View User Service logs
	docker-compose logs -f user-service

logs-product: ## View Product Service logs
	docker-compose logs -f product-service

logs-order: ## View Order Service logs
	docker-compose logs -f order-service

logs-inventory: ## View Inventory Service logs
	docker-compose logs -f inventory-service

logs-notification: ## View Notification Service logs
	docker-compose logs -f notification-service

restart: down up ## Restart all services

clean: ## Clean up Docker resources
	docker-compose down -v --remove-orphans
	docker system prune -f

health: ## Check health of all services
	@echo "=== Health Check ==="
	@curl -s http://localhost:8001/health | jq '.' || echo "User Service: DOWN"
	@curl -s http://localhost:8002/health | jq '.' || echo "Product Service: DOWN"
	@curl -s http://localhost:8003/health | jq '.' || echo "Order Service: DOWN"
	@curl -s http://localhost:8004/health | jq '.' || echo "Inventory Service: DOWN"
	@curl -s http://localhost:8005/health | jq '.' || echo "Notification Service: DOWN"

test-user: ## Run User Service tests
	cd user-service && python -m pytest tests/ -v

test-product: ## Run Product Service tests
	cd product-service && python -m pytest tests/ -v

test-order: ## Run Order Service tests
	cd order-service && python -m pytest tests/ -v

test-inventory: ## Run Inventory Service tests
	cd inventory-service && python -m pytest tests/ -v

test-notification: ## Run Notification Service tests
	cd notification-service && python -m pytest tests/ -v

test-frontend: ## Run Frontend tests
	cd frontend && npm test

test-all: test-user test-product test-order test-inventory test-notification test-frontend ## Run all tests

# AWS Deployment
tf-init: ## Initialize Terraform
	cd infrastructure/terraform && terraform init

tf-plan: ## Run Terraform plan
	cd infrastructure/terraform && terraform plan

tf-apply: ## Apply Terraform changes
	cd infrastructure/terraform && terraform apply

tf-destroy: ## Destroy Terraform infrastructure
	cd infrastructure/terraform && terraform destroy

# Development helpers
install-user: ## Install User Service dependencies
	cd user-service && pip install -r requirements.txt

install-product: ## Install Product Service dependencies
	cd product-service && pip install -r requirements.txt

install-order: ## Install Order Service dependencies
	cd order-service && pip install -r requirements.txt

install-inventory: ## Install Inventory Service dependencies
	cd inventory-service && pip install -r requirements.txt

install-notification: ## Install Notification Service dependencies
	cd notification-service && pip install -r requirements.txt

install-frontend: ## Install Frontend dependencies
	cd frontend && npm install

install-all: install-user install-product install-order install-inventory install-notification install-frontend ## Install all dependencies

# Monitoring
prometheus: ## Open Prometheus
	@open http://localhost:9090 || xdg-open http://localhost:9090 || echo "Open http://localhost:9090 in your browser"

grafana: ## Open Grafana
	@open http://localhost:3001 || xdg-open http://localhost:3001 || echo "Open http://localhost:3001 in your browser (admin/admin123)"

kibana: ## Open Kibana
	@open http://localhost:5601 || xdg-open http://localhost:5601 || echo "Open http://localhost:5601 in your browser"

# Data seeding
seed-data: ## Seed initial data
	@echo "Seeding initial data..."
	@curl -X POST http://localhost:8001/users \
		-H "Content-Type: application/json" \
		-d '{"username":"testuser","email":"test@example.com","password":"testpass123","full_name":"Test User"}'
	@curl -X POST http://localhost:8002/products \
		-H "Content-Type: application/json" \
		-d '{"name":"Test Product","description":"A test product","price":99.99,"category":"Electronics","sku":"TEST-001"}'
	@echo "\nData seeded successfully!"

# Database operations
db-migrate: ## Run database migrations
	@echo "Running database migrations..."
	cd user-service && alembic upgrade head
	cd product-service && alembic upgrade head
	cd order-service && alembic upgrade head

db-reset: ## Reset all databases
	docker-compose down -v
	docker-compose up -d postgres mongodb redis
	@sleep 5
	docker-compose up -d

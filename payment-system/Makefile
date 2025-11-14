.PHONY: help build up down logs clean test lint format

help: ## Show this help message
	@echo 'Usage: make [target]'
	@echo ''
	@echo 'Available targets:'
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "  %-20s %s\n", $$1, $$2}' $(MAKEFILE_LIST)

build: ## Build all Docker images
	docker-compose build

up: ## Start all services
	docker-compose up -d

down: ## Stop all services
	docker-compose down

logs: ## View logs from all services
	docker-compose logs -f

logs-api: ## View API Gateway logs
	docker-compose logs -f api-gateway

logs-user: ## View User Service logs
	docker-compose logs -f user-service

logs-wallet: ## View Wallet Service logs
	docker-compose logs -f wallet-service

logs-transaction: ## View Transaction Service logs
	docker-compose logs -f transaction-service

restart: ## Restart all services
	docker-compose restart

clean: ## Remove all containers, volumes, and images
	docker-compose down -v
	docker system prune -af

ps: ## Show running containers
	docker-compose ps

test: ## Run tests
	pytest tests/ -v

test-coverage: ## Run tests with coverage
	pytest tests/ --cov=. --cov-report=html

lint: ## Run linting
	flake8 services/ shared/ api-gateway/

format: ## Format code with black
	black services/ shared/ api-gateway/

install-dev: ## Install development dependencies
	pip install pytest pytest-asyncio pytest-cov httpx flake8 black

db-migrate: ## Run database migrations
	@echo "Running migrations..."
	# Add migration commands here

db-seed: ## Seed database with test data
	@echo "Seeding database..."
	python scripts/seed_data.py

aws-login: ## Login to AWS ECR
	aws ecr get-login-password --region us-east-1 | docker login --username AWS --password-stdin $(AWS_ACCOUNT_ID).dkr.ecr.us-east-1.amazonaws.com

aws-push: ## Build and push images to AWS ECR
	@echo "Building and pushing images to ECR..."
	./scripts/push_to_ecr.sh

terraform-init: ## Initialize Terraform
	cd deployments/terraform && terraform init

terraform-plan: ## Run Terraform plan
	cd deployments/terraform && terraform plan

terraform-apply: ## Apply Terraform changes
	cd deployments/terraform && terraform apply

terraform-destroy: ## Destroy Terraform infrastructure
	cd deployments/terraform && terraform destroy

monitoring: ## Open Grafana dashboard
	open http://localhost:3000

prometheus: ## Open Prometheus dashboard
	open http://localhost:9090

docs: ## Open API documentation
	open http://localhost:8000/docs

health: ## Check health of all services
	@curl -s http://localhost:8000/health | jq

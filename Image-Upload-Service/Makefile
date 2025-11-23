.PHONY: help install install-test start stop setup test clean

help:
	@echo "Image Upload Service - Makefile Commands"
	@echo ""
	@echo "Available commands:"
	@echo "  make install       - Install project dependencies"
	@echo "  make install-test  - Install test dependencies"
	@echo "  make start         - Start LocalStack services"
	@echo "  make stop          - Stop LocalStack services"
	@echo "  make setup         - Setup LocalStack resources (run after start)"
	@echo "  make test          - Run unit tests"
	@echo "  make clean         - Clean up temporary files and containers"
	@echo ""

install:
	@echo "Installing dependencies..."
	pip install -r requirements.txt

install-test:
	@echo "Installing test dependencies..."
	pip install -r requirements-test.txt

start:
	@echo "Starting LocalStack..."
	docker-compose up -d
	@echo "Waiting for LocalStack to be ready..."
	@sleep 5
	@echo "LocalStack is running!"

stop:
	@echo "Stopping LocalStack..."
	docker-compose down

setup:
	@echo "Setting up LocalStack resources..."
	chmod +x scripts/setup_localstack.sh
	./scripts/setup_localstack.sh

test:
	@echo "Running unit tests..."
	python -m pytest tests/ -v --cov=lambda_functions --cov-report=term-missing

clean:
	@echo "Cleaning up..."
	docker-compose down -v
	find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete 2>/dev/null || true
	find . -type f -name "*.zip" -delete 2>/dev/null || true
	rm -rf .pytest_cache .coverage htmlcov
	@echo "Cleanup complete!"

restart: stop start setup

all: install install-test start setup
	@echo "All setup complete! Service is ready."

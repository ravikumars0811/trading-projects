# Financial LLM Production Dockerfile
# Multi-stage build for optimized image

# Stage 1: Builder
FROM nvidia/cuda:12.1.0-devel-ubuntu22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    python3.10 \
    python3.10-dev \
    python3-pip \
    git \
    wget \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Set Python 3.10 as default
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.10 1
RUN update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 1

# Upgrade pip
RUN pip install --no-cache-dir --upgrade pip setuptools wheel

# Copy requirements
WORKDIR /build
COPY requirements.txt .

# Install Python packages
RUN pip install --no-cache-dir -r requirements.txt

# Stage 2: Runtime
FROM nvidia/cuda:12.1.0-runtime-ubuntu22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    python3.10 \
    python3-pip \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Set Python 3.10 as default
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.10 1

# Copy Python packages from builder
COPY --from=builder /usr/local/lib/python3.10/dist-packages /usr/local/lib/python3.10/dist-packages

# Create app user
RUN useradd -m -u 1000 -s /bin/bash llm-service

# Set working directory
WORKDIR /app

# Copy application code
COPY --chown=llm-service:llm-service . .

# Create necessary directories
RUN mkdir -p /app/logs /app/checkpoints && \
    chown -R llm-service:llm-service /app

# Switch to app user
USER llm-service

# Expose ports
EXPOSE 8000 50051

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=60s --retries=3 \
  CMD curl -f http://localhost:8000/health || exit 1

# Default environment variables
ENV PYTHONUNBUFFERED=1
ENV MODEL_PATH=/app/checkpoints/best_model.pt
ENV TOKENIZER_PATH=/app/checkpoints/tokenizer.pkl
ENV DEVICE=cuda
ENV API_TYPE=rest

# Entry point script
COPY --chown=llm-service:llm-service deployments/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh

ENTRYPOINT ["/app/entrypoint.sh"]

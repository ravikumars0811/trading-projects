# Multi-stage build for production-grade algorithmic trading system

# Stage 1: Build C++ components
FROM ubuntu:22.04 AS cpp-builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    libpq-dev \
    libssl-dev \
    libhiredis-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy source code
COPY include ./include
COPY src ./src
COPY CMakeLists.txt .

# Build
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# Stage 2: Python ML environment
FROM python:3.10-slim AS python-builder

WORKDIR /app

# Install Python dependencies
COPY python/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY python ./python

# Stage 3: Final production image
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    libpq5 \
    libssl3 \
    libhiredis0.14 \
    python3.10 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy C++ binaries
COPY --from=cpp-builder /build/build/trading_system_main ./bin/
COPY --from=cpp-builder /build/build/backtester ./bin/

# Copy Python components
COPY --from=python-builder /usr/local/lib/python3.10/site-packages /usr/local/lib/python3.10/site-packages
COPY --from=python-builder /app/python ./python

# Copy configuration
COPY config ./config
COPY scripts ./scripts

# Create necessary directories
RUN mkdir -p logs data models

# Set environment variables
ENV PYTHONPATH=/app/python
ENV LD_LIBRARY_PATH=/usr/local/lib

# Expose ports
EXPOSE 8080 9090

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD pgrep -f trading_system_main || exit 1

# Run the trading system
CMD ["./bin/trading_system_main"]

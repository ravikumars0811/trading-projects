#!/bin/bash
set -e

# Financial LLM Docker Entrypoint Script

echo "Starting Financial LLM Service..."
echo "API Type: ${API_TYPE}"
echo "Device: ${DEVICE}"
echo "Model Path: ${MODEL_PATH}"

# Wait for Redis if needed
if [ ! -z "${REDIS_HOST}" ]; then
    echo "Waiting for Redis at ${REDIS_HOST}:${REDIS_PORT}..."
    while ! nc -z ${REDIS_HOST} ${REDIS_PORT}; do
        sleep 1
    done
    echo "Redis is ready!"
fi

# Start appropriate API
if [ "${API_TYPE}" = "grpc" ]; then
    echo "Starting gRPC API on port 50051..."
    exec python -m src.api.grpc_server \
        --model-path ${MODEL_PATH} \
        --tokenizer-path ${TOKENIZER_PATH} \
        --host 0.0.0.0 \
        --port 50051 \
        --workers ${GRPC_WORKERS:-10} \
        --device ${DEVICE}
else
    echo "Starting REST API on port 8000..."
    exec python -m src.api.rest_api \
        --model-path ${MODEL_PATH} \
        --tokenizer-path ${TOKENIZER_PATH} \
        --host 0.0.0.0 \
        --port 8000 \
        --workers ${API_WORKERS:-4} \
        --device ${DEVICE}
fi

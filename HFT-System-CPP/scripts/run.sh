#!/bin/bash

set -e

# Create logs directory if it doesn't exist
mkdir -p logs

# Check if config file exists
CONFIG_FILE="${1:-config/hft_config.txt}"

if [ ! -f "$CONFIG_FILE" ]; then
    echo "Error: Configuration file not found: $CONFIG_FILE"
    exit 1
fi

echo "Starting HFT System with config: $CONFIG_FILE"
echo "Logs will be written to: logs/"
echo ""

# Run the system
./build/hft_system "$CONFIG_FILE"

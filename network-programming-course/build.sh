#!/bin/bash

# Build script for Network Programming Course examples

set -e  # Exit on error

echo "=================================="
echo "Network Programming Course Builder"
echo "=================================="

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo ""
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build all targets
echo ""
echo "Building all examples..."
make -j$(nproc)

echo ""
echo "=================================="
echo "Build Complete!"
echo "=================================="
echo ""
echo "Executables are in: build/"
echo ""
echo "Module 2 examples:"
echo "  - tcp_server"
echo "  - tcp_client"
echo "  - udp_server"
echo "  - udp_client"
echo "  - optimized_tcp_server"
echo ""
echo "Module 3 examples:"
echo "  - epoll_server"
echo ""
echo "To run an example:"
echo "  ./build/tcp_server"
echo ""

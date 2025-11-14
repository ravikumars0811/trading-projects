#!/bin/bash

# Build script for Pricing Engine
# This script builds the C++ library, Python bindings, and installs dependencies

set -e  # Exit on error

echo "========================================="
echo "Building Pricing Engine"
echo "========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check prerequisites
echo -e "${YELLOW}Checking prerequisites...${NC}"

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}CMake not found. Please install CMake 3.15+${NC}"
    exit 1
fi

if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}C++ compiler not found. Please install GCC or Clang${NC}"
    exit 1
fi

if ! command -v python3 &> /dev/null; then
    echo -e "${RED}Python 3 not found. Please install Python 3.11+${NC}"
    exit 1
fi

echo -e "${GREEN}Prerequisites check passed!${NC}"

# Build C++ library
echo ""
echo -e "${YELLOW}Building C++ library...${NC}"
cd cpp
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
echo -e "${GREEN}C++ library built successfully!${NC}"

# Install Python dependencies
echo ""
echo -e "${YELLOW}Installing Python dependencies...${NC}"
cd ../../backend
pip install -r requirements.txt
echo -e "${GREEN}Python dependencies installed!${NC}"

# Build Python bindings (if pybind11 is available)
echo ""
echo -e "${YELLOW}Building Python bindings...${NC}"
cd ../cpp/build
if cmake .. -DCMAKE_BUILD_TYPE=Release 2>&1 | grep -q "pybind11"; then
    make pricing_engine_py -j$(nproc)
    echo -e "${GREEN}Python bindings built successfully!${NC}"
else
    echo -e "${YELLOW}pybind11 not found, skipping Python bindings${NC}"
    echo -e "${YELLOW}Installing pybind11...${NC}"
    pip install pybind11
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make pricing_engine_py -j$(nproc)
    echo -e "${GREEN}Python bindings built successfully!${NC}"
fi

# Run tests
echo ""
echo -e "${YELLOW}Running tests...${NC}"
if command -v ctest &> /dev/null; then
    ctest --output-on-failure
    echo -e "${GREEN}C++ tests passed!${NC}"
else
    echo -e "${YELLOW}CTest not available, skipping C++ tests${NC}"
fi

cd ../../backend
if command -v pytest &> /dev/null; then
    echo -e "${YELLOW}Running Python tests...${NC}"
    pytest test_api.py -v --disable-warnings || echo -e "${YELLOW}Some tests failed or backend not running${NC}"
else
    echo -e "${YELLOW}pytest not installed, skipping Python tests${NC}"
fi

# Install frontend dependencies
echo ""
echo -e "${YELLOW}Installing frontend dependencies...${NC}"
cd ../frontend
if command -v npm &> /dev/null; then
    npm install
    echo -e "${GREEN}Frontend dependencies installed!${NC}"
else
    echo -e "${YELLOW}npm not found, skipping frontend setup${NC}"
fi

# Summary
echo ""
echo "========================================="
echo -e "${GREEN}Build Complete!${NC}"
echo "========================================="
echo ""
echo "To run the application:"
echo ""
echo "Option 1 - Docker (Recommended):"
echo "  cd deployment"
echo "  docker-compose up --build"
echo ""
echo "Option 2 - Local Development:"
echo "  Terminal 1 - Backend:"
echo "    cd backend"
echo "    uvicorn main:app --reload"
echo ""
echo "  Terminal 2 - Frontend:"
echo "    cd frontend"
echo "    npm run dev"
echo ""
echo "Access the application:"
echo "  Frontend: http://localhost:3000"
echo "  Backend API: http://localhost:8000"
echo "  API Docs: http://localhost:8000/api/docs"
echo ""

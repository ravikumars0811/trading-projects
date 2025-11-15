#!/bin/bash

# Build script for C++ Interview Solutions

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}C++ Interview Solutions Build Script${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN=false
RUN_EXAMPLES=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -r|--run)
            RUN_EXAMPLES=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -d, --debug    Build in Debug mode (default: Release)"
            echo "  -c, --clean    Clean build directory before building"
            echo "  -r, --run      Run all examples after building"
            echo "  -h, --help     Show this help message"
            echo ""
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf build
fi

# Create build directory
mkdir -p build
cd build

# Configure
echo -e "${YELLOW}Configuring with CMake (Build Type: $BUILD_TYPE)...${NC}"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

# Build
echo -e "${YELLOW}Building...${NC}"
NUM_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
cmake --build . -j$NUM_CORES

echo ""
echo -e "${GREEN}Build completed successfully!${NC}"
echo ""
echo -e "Executables are in: ${YELLOW}build/bin/${NC}"
echo ""
echo "Available programs:"
echo "  - move_semantics"
echo "  - smart_pointers"
echo "  - templates_metaprogramming"
echo "  - lock_free_queue"
echo "  - order_book"
echo "  - trading_algorithms"
echo "  - trading_patterns"
echo "  - thread_pool"
echo "  - optimization_techniques"
echo "  - backtesting_engine"
echo ""

# Run examples if requested
if [ "$RUN_EXAMPLES" = true ]; then
    echo -e "${GREEN}Running all examples...${NC}"
    echo ""
    make run_all
fi

echo -e "${GREEN}Done!${NC}"

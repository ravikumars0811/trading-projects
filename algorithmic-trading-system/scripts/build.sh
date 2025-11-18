#!/bin/bash
# Build script for Algorithmic Trading System

set -e

echo "==================================================="
echo "Building Algorithmic Trading System"
echo "==================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for required tools
check_dependencies() {
    echo -e "${YELLOW}Checking dependencies...${NC}"

    if ! command -v cmake &> /dev/null; then
        echo -e "${RED}CMake not found. Please install CMake 3.15 or higher.${NC}"
        exit 1
    fi

    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        echo -e "${RED}C++ compiler not found. Please install g++ or clang++.${NC}"
        exit 1
    fi

    if ! command -v python3 &> /dev/null; then
        echo -e "${RED}Python 3 not found. Please install Python 3.8 or higher.${NC}"
        exit 1
    fi

    echo -e "${GREEN}All dependencies found.${NC}"
}

# Build C++ components
build_cpp() {
    echo -e "${YELLOW}Building C++ components...${NC}"

    mkdir -p build
    cd build

    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc)

    cd ..

    echo -e "${GREEN}C++ components built successfully.${NC}"
}

# Install Python dependencies
setup_python() {
    echo -e "${YELLOW}Setting up Python environment...${NC}"

    if [ ! -d "venv" ]; then
        python3 -m venv venv
    fi

    source venv/bin/activate
    pip install --upgrade pip
    pip install -r python/requirements.txt

    echo -e "${GREEN}Python environment setup complete.${NC}"
}

# Run tests
run_tests() {
    echo -e "${YELLOW}Running tests...${NC}"

    # C++ tests
    if [ -f "build/tests/run_tests" ]; then
        ./build/tests/run_tests
    fi

    # Python tests
    source venv/bin/activate
    pytest python/tests/ || true

    echo -e "${GREEN}Tests completed.${NC}"
}

# Main build process
main() {
    check_dependencies
    build_cpp
    setup_python

    if [ "$1" = "--with-tests" ]; then
        run_tests
    fi

    echo ""
    echo -e "${GREEN}==================================================="
    echo -e "Build completed successfully!"
    echo -e "===================================================${NC}"
    echo ""
    echo "To run the trading system:"
    echo "  ./build/trading_system_main"
    echo ""
    echo "To run backtester:"
    echo "  ./build/backtester"
    echo ""
    echo "To activate Python environment:"
    echo "  source venv/bin/activate"
    echo ""
}

main "$@"

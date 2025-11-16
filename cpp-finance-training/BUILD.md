# Build Instructions

## Quick Start

### Option 1: Manual Compilation (Easiest)

Compile individual examples:
```bash
cd cpp-finance-training/01-basics
g++ -std=c++17 -O2 -Wall 01_variables_datatypes.cpp -o test
./test
```

### Option 2: Using CMake (Recommended for all examples)

Build all examples:
```bash
cd cpp-finance-training
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

Run examples:
```bash
# Examples are in bin/ directory organized by module
./bin/01-basics/01-basics_01_variables_datatypes
./bin/07-hft-specific/07-hft-specific_01_order_book
```

### Option 3: Build Script

Create a helper script:
```bash
cat > build.sh << 'EOF'
#!/bin/bash
MODULE=$1
FILE=$2

if [ -z "$MODULE" ] || [ -z "$FILE" ]; then
    echo "Usage: ./build.sh MODULE FILE"
    echo "Example: ./build.sh 01-basics 01_variables_datatypes.cpp"
    exit 1
fi

g++ -std=c++20 -O3 -Wall -Wextra -pthread \
    -march=native -mtune=native \
    "$MODULE/$FILE" -o test && ./test
EOF

chmod +x build.sh
./build.sh 01-basics 01_variables_datatypes.cpp
```

## Compilation Modes

### Release Build (Maximum Performance)
```bash
g++ -std=c++20 -O3 -DNDEBUG -march=native -mtune=native \
    -flto -Wall -Wextra filename.cpp -o program
```

**Use for**: Performance testing, benchmarking, production code

### Debug Build (Development)
```bash
g++ -std=c++20 -O0 -g -Wall -Wextra -Wpedantic \
    filename.cpp -o program_debug
```

**Use for**: Development, debugging, finding bugs

### Sanitizer Build (Bug Detection)
```bash
g++ -std=c++20 -O1 -g -fsanitize=address,undefined \
    -fno-omit-frame-pointer filename.cpp -o program_san
./program_san
```

**Use for**: Detecting memory leaks, buffer overflows, undefined behavior

### Profile Build (Performance Analysis)
```bash
g++ -std=c++20 -O3 -g -pg filename.cpp -o program_prof
./program_prof
gprof program_prof gmon.out > analysis.txt
```

**Use for**: Identifying performance bottlenecks

## CMake Build Types

### Release Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```
- Maximum optimization (-O3)
- Native CPU features
- Link-time optimization

### Debug Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```
- No optimization (-O0)
- Debug symbols (-g)
- Easier debugging

### RelWithDebInfo Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
```
- Optimized (-O2)
- With debug symbols
- Best of both worlds for profiling

## Platform-Specific Instructions

### Ubuntu/Debian
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake g++ git

# Build
cd cpp-finance-training
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake
brew install cmake

# Build
cd cpp-finance-training
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

### Windows (MinGW)
```bash
# Install MinGW-w64
# Download from: https://www.mingw-w64.org/

# Build
cd cpp-finance-training
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make -j4
```

### Windows (Visual Studio)
```bash
# Open Developer Command Prompt
cd cpp-finance-training
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Compiler Requirements

### Minimum Versions
- **GCC**: 11.0 or later (for C++20)
- **Clang**: 14.0 or later (for C++20)
- **MSVC**: Visual Studio 2022 (for C++20)

### Check Your Version
```bash
g++ --version
clang++ --version
```

### Install Newer GCC (Ubuntu)
```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-13
g++-13 --version
```

## Module-Specific Build Notes

### Module 1-4: Standard Compilation
```bash
g++ -std=c++17 -O2 filename.cpp -o test
```

### Module 5: System Programming (needs pthread)
```bash
g++ -std=c++20 -O3 -pthread filename.cpp -o test
```

### Module 6: Memory Optimization (needs SIMD)
```bash
g++ -std=c++20 -O3 -march=native -mavx2 filename.cpp -o test
```

### Module 7: HFT (maximum optimization)
```bash
g++ -std=c++20 -O3 -march=native -mtune=native -flto \
    -pthread -DNDEBUG filename.cpp -o test
```

## Troubleshooting

### Error: "ISO C++17 does not allow"
**Solution**: Update compiler or use older standard
```bash
g++ -std=c++14 filename.cpp -o test
```

### Error: "undefined reference to pthread"
**Solution**: Add pthread flag
```bash
g++ -std=c++20 -pthread filename.cpp -o test
```

### Error: "filesystem not found"
**Solution**: Link filesystem library (older compilers)
```bash
g++ -std=c++17 filename.cpp -lstdc++fs -o test
```

### Warning: "unused variable"
**Solution**: Fix the code or disable warning temporarily
```bash
g++ -Wno-unused-variable filename.cpp -o test
```

### Slow Compilation
**Solution**: Use parallel compilation
```bash
make -j$(nproc)  # Use all cores
```

## Performance Validation

### Verify Optimization
```bash
# Compile with optimization
g++ -std=c++20 -O3 -march=native filename.cpp -o optimized

# Check assembly (should be optimized)
objdump -d optimized | less

# Or use compiler explorer online
# https://godbolt.org/
```

### Benchmark
```bash
# Compile
g++ -std=c++20 -O3 -march=native filename.cpp -o test

# Run with time measurement
time ./test

# Run with perf
perf stat ./test
```

## Cleaning Build Artifacts

### CMake Build
```bash
cd build
make clean
# Or delete entire build directory
cd .. && rm -rf build
```

### Manual Build
```bash
rm -f test test.o *.out
```

## Advanced Build Options

### Enable Link-Time Optimization (LTO)
```bash
g++ -std=c++20 -O3 -flto filename.cpp -o test
```

### Profile-Guided Optimization (PGO)
```bash
# Step 1: Compile with instrumentation
g++ -std=c++20 -O3 -fprofile-generate filename.cpp -o test_prof

# Step 2: Run with representative data
./test_prof

# Step 3: Recompile with profile data
g++ -std=c++20 -O3 -fprofile-use filename.cpp -o test_optimized
```

### Generate Assembly
```bash
g++ -std=c++20 -O3 -S -masm=intel filename.cpp
cat filename.s
```

## Static Analysis

### Clang-Tidy
```bash
clang-tidy filename.cpp -- -std=c++20
```

### Cppcheck
```bash
cppcheck --enable=all filename.cpp
```

## Memory Profiling

### Valgrind (Memory Leaks)
```bash
g++ -std=c++20 -O0 -g filename.cpp -o test
valgrind --leak-check=full ./test
```

### AddressSanitizer (Fast Memory Errors)
```bash
g++ -std=c++20 -O1 -g -fsanitize=address filename.cpp -o test
./test
```

## Recommended Build Command

For **development**:
```bash
g++ -std=c++20 -O0 -g -Wall -Wextra -Wpedantic filename.cpp -o test
```

For **production/benchmarking**:
```bash
g++ -std=c++20 -O3 -DNDEBUG -march=native -mtune=native -flto \
    -Wall -Wextra -pthread filename.cpp -o test
```

---

**Ready to build?**
```bash
cd cpp-finance-training/01-basics
g++ -std=c++17 -O2 01_variables_datatypes.cpp -o test && ./test
```

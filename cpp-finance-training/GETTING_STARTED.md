# Getting Started with C++ Training for HFT & Finance

## Prerequisites

### Required Software
- **C++ Compiler**: GCC 11+ or Clang 14+ (for C++20/23 support)
- **Build System**: CMake 3.20+ (optional but recommended)
- **Text Editor/IDE**: VSCode, CLion, Vim, or your preference
- **Git**: For version control

### Recommended Tools
- **Profilers**: `perf`, `valgrind`, `gprof`
- **Benchmarking**: Google Benchmark (optional)
- **Testing**: Google Test (optional)
- **Static Analysis**: `clang-tidy`, `cppcheck`

## Installation

### On Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential g++ cmake git
sudo apt install valgrind linux-tools-generic  # For profiling
```

### On macOS
```bash
xcode-select --install
brew install cmake
```

### On Windows
- Install Visual Studio 2022 with C++ support, or
- Install MinGW-w64 or WSL2 with Ubuntu

## Verify Installation

```bash
# Check compiler version
g++ --version  # Should be 11.0 or higher
clang++ --version  # Should be 14.0 or higher

# Check C++20 support
echo 'int main() { return 0; }' > test.cpp
g++ -std=c++20 test.cpp -o test && ./test && echo "C++20 works!"
rm test.cpp test
```

## Learning Path

### Week 1-2: Foundations
Start with Module 1 (Basics):
```bash
cd cpp-finance-training/01-basics
g++ -std=c++17 -O2 01_variables_datatypes.cpp -o test
./test
```

Work through all examples in order:
1. Variables and data types
2. Control flow
3. Functions
4. Arrays and strings
5. Pointers and references
6. Dynamic memory
7. I/O operations

### Week 3-4: Intermediate Concepts
Move to Module 2 (Intermediate):
```bash
cd ../02-intermediate
g++ -std=c++17 -O2 01_classes_objects.cpp -o test
./test
```

Focus on:
- Object-oriented programming
- Inheritance and polymorphism
- STL containers
- Templates basics

### Week 5-6: Advanced Topics
Progress to Module 3 (Advanced):
```bash
cd ../03-advanced
g++ -std=c++17 -O3 01_move_semantics.cpp -o test
./test
```

Master:
- Move semantics
- Smart pointers
- Advanced templates
- Lambdas

### Week 7-8: Modern C++
Explore Module 4 (Modern C++):
```bash
cd ../04-modern-cpp
g++ -std=c++20 -O3 01_cpp11_features.cpp -o test
./test
```

Learn:
- C++11 through C++23 features
- High-resolution timing
- Constexpr optimization

### Week 9-10: System Programming
Study Module 5 (System Programming):
```bash
cd ../05-system-programming
g++ -std=c++20 -O3 -pthread 01_high_resolution_timing.cpp -o test
./test
```

Understand:
- Threading and concurrency
- Atomic operations
- Lock-free programming

### Week 11-12: Optimization
Deep dive into Module 6 (Memory Optimization):
```bash
cd ../06-memory-optimization
g++ -std=c++20 -O3 -march=native <file>.cpp -o test
./test
```

Optimize for:
- Cache-friendly code
- Memory layout
- SIMD operations

### Week 13-14: HFT Specifics
Focus on Module 7 (HFT-Specific):
```bash
cd ../07-hft-specific
g++ -std=c++20 -O3 01_order_book.cpp -o test
./test
```

Implement:
- Order books
- Lock-free queues
- Market data handlers

### Week 15-16: Projects
Build projects from Module 8:
```bash
cd ../08-practical-projects
# Follow project-specific instructions
```

## Compilation Flags Explained

### Basic Compilation
```bash
g++ -std=c++17 filename.cpp -o output
```
- `-std=c++17`: Use C++17 standard (or c++20, c++23)
- `-o output`: Name the output file

### Optimized Build (Release)
```bash
g++ -std=c++17 -O3 -DNDEBUG filename.cpp -o output
```
- `-O3`: Maximum optimization
- `-DNDEBUG`: Disable assertions

### Debug Build
```bash
g++ -std=c++17 -O0 -g -Wall -Wextra filename.cpp -o output
```
- `-O0`: No optimization
- `-g`: Debug symbols
- `-Wall -Wextra`: Enable warnings

### Performance Build (HFT)
```bash
g++ -std=c++20 -O3 -march=native -mtune=native -flto -DNDEBUG filename.cpp -o output
```
- `-march=native`: Use CPU-specific instructions
- `-mtune=native`: Optimize for this CPU
- `-flto`: Link-time optimization

### With Threading
```bash
g++ -std=c++20 -O3 -pthread filename.cpp -o output
```
- `-pthread`: Enable POSIX threads

### With Sanitizers (Debugging)
```bash
g++ -std=c++17 -O0 -g -fsanitize=address,undefined filename.cpp -o output
```
- Detects memory leaks, buffer overflows, undefined behavior

## Running Examples

### Single File
```bash
cd cpp-finance-training/01-basics
g++ -std=c++17 -O2 01_variables_datatypes.cpp -o test
./test
```

### With Profiling
```bash
g++ -std=c++17 -O3 -g -pg filename.cpp -o test
./test
gprof test gmon.out > analysis.txt
```

### With Valgrind
```bash
g++ -std=c++17 -O0 -g filename.cpp -o test
valgrind --leak-check=full ./test
```

### With Performance Counter
```bash
g++ -std=c++17 -O3 filename.cpp -o test
perf stat ./test
```

## Common Issues and Solutions

### Issue: "error: 'auto' not found"
**Solution**: Use `-std=c++11` or later

### Issue: "undefined reference to 'pthread_create'"
**Solution**: Add `-pthread` flag

### Issue: "no such file or directory: <filesystem>"
**Solution**: Use C++17 or later: `-std=c++17`

### Issue: Segmentation fault
**Solutions**:
1. Compile with debug symbols: `-g`
2. Run with debugger: `gdb ./test`
3. Use sanitizers: `-fsanitize=address`

### Issue: Performance is slow
**Solutions**:
1. Compile with optimizations: `-O3`
2. Use native CPU features: `-march=native`
3. Profile with `perf` or `gprof`

## Tips for Success

### 1. Compile and Run Every Example
Don't just read the code - compile and run it!

### 2. Modify and Experiment
- Change values
- Add new features
- Break things and fix them

### 3. Use a Debugger
```bash
g++ -g filename.cpp -o test
gdb ./test
```

### 4. Measure Performance
```bash
# Use the Timer class or perf
perf stat ./test
```

### 5. Read Compiler Warnings
```bash
g++ -Wall -Wextra -Wpedantic filename.cpp -o test
```

### 6. Use Version Control
```bash
git init
git add .
git commit -m "Initial commit"
```

### 7. Take Notes
Document what you learn, especially:
- Performance characteristics
- Design patterns
- Common pitfalls

## Resources

### Documentation
- [C++ Reference](https://en.cppreference.com/)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [GCC Manual](https://gcc.gnu.org/onlinedocs/)

### Books
- "Effective Modern C++" by Scott Meyers
- "C++ Concurrency in Action" by Anthony Williams
- "Optimized C++" by Kurt Guntheroth

### Online
- [Compiler Explorer](https://godbolt.org/) - See assembly output
- [Quick Bench](https://quick-bench.com/) - Online benchmarking
- [C++ Insights](https://cppinsights.io/) - See compiler transformations

## Next Steps

1. **Choose your starting module** based on your current level
2. **Work through examples** sequentially
3. **Complete exercises** at the end of each module
4. **Build projects** to consolidate learning
5. **Profile and optimize** your code
6. **Share and get feedback** on your implementations

## Getting Help

If you get stuck:
1. Read the comments in the code
2. Check the module README
3. Look up concepts in C++ reference
4. Review related examples
5. Debug with print statements or gdb
6. Ask questions with specific code examples

## Contributing

Found an error or have a suggestion?
- Create an issue
- Submit a pull request
- Share your improvements

---

**Ready to start?** Begin with:
```bash
cd cpp-finance-training/01-basics
cat README.md
g++ -std=c++17 -O2 01_variables_datatypes.cpp -o test && ./test
```

**Good luck on your C++ journey! 🚀**

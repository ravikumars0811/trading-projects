# Module 4: Modern C++ Features (C++11 to C++23)

## C++11 Features

- **Auto and decltype**: Type inference
- **Nullptr**: Type-safe null pointer
- **Range-based for loops**: Cleaner iteration
- **Lambda expressions**: Anonymous functions
- **Move semantics**: Rvalue references
- **Smart pointers**: unique_ptr, shared_ptr, weak_ptr
- **Variadic templates**: Variable number of template arguments
- **constexpr**: Compile-time evaluation
- **static_assert**: Compile-time assertions
- **std::chrono**: Time utilities
- **std::thread**: Threading support
- **std::atomic**: Lock-free atomics

## C++14 Features

- **Generic lambdas**: auto parameters in lambdas
- **Binary literals**: 0b prefix
- **Digit separators**: 1'000'000
- **std::make_unique**: Factory for unique_ptr
- **Variable templates**: Template variables
- **Return type deduction**: auto return types
- **Relaxed constexpr**: More complex compile-time functions

## C++17 Features

- **Structured bindings**: auto [a, b] = tuple
- **if constexpr**: Compile-time conditionals
- **std::optional**: Maybe type
- **std::variant**: Type-safe union
- **std::string_view**: Non-owning string reference
- **Fold expressions**: Variadic template operations
- **Inline variables**: Header-only variables
- **Filesystem library**: std::filesystem
- **Parallel algorithms**: Execution policies

## C++20 Features

- **Concepts**: Template constraints
- **Ranges**: Composable algorithms
- **Coroutines**: Async/await
- **Modules**: Better compilation model
- **Three-way comparison**: <=>
- **Designated initializers**: .field = value
- **std::span**: Non-owning array view
- **std::format**: Type-safe formatting
- **Calendar and timezone**: chrono extensions

## C++23 Features

- **std::expected**: Result type with error
- **std::mdspan**: Multi-dimensional span
- **if consteval**: Distinguish compile-time context
- **Deducing this**: Explicit object parameter
- **std::print**: Simplified output
- **Ranges improvements**: zip, chunk, etc.
- **std::flat_map/flat_set**: Contiguous associative containers

## Code Examples

1. `01_cpp11_features.cpp` - Core C++11 improvements
2. `02_cpp14_features.cpp` - C++14 enhancements
3. `03_cpp17_features.cpp` - C++17 additions
4. `04_cpp20_features.cpp` - C++20 major features
5. `05_cpp23_features.cpp` - C++23 latest additions
6. `06_chrono_timing.cpp` - High-resolution timing for HFT
7. `07_constexpr_optimization.cpp` - Compile-time optimization

## Compilation

```bash
# C++11
g++ -std=c++11 -O3 filename.cpp -o output

# C++14
g++ -std=c++14 -O3 filename.cpp -o output

# C++17
g++ -std=c++17 -O3 filename.cpp -o output

# C++20
g++ -std=c++20 -O3 filename.cpp -o output

# C++23 (requires GCC 13+ or Clang 16+)
g++ -std=c++23 -O3 filename.cpp -o output
```

## HFT Relevance

- **constexpr**: Move calculations to compile time
- **std::chrono**: Nanosecond-precision timestamps
- **std::atomic**: Lock-free data structures
- **std::optional**: Avoiding exceptions in hot paths
- **Concepts**: Better template error messages
- **Ranges**: Composable, efficient algorithms
- **string_view**: Zero-copy string operations

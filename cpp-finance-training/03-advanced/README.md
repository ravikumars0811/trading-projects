# Module 3: Advanced C++

## Topics Covered

1. **Move Semantics and Rvalue References (C++11)**
   - Lvalue vs Rvalue
   - Move constructors and move assignment
   - std::move and std::forward
   - Perfect forwarding

2. **Smart Pointers (C++11)**
   - std::unique_ptr
   - std::shared_ptr
   - std::weak_ptr
   - RAII and resource management

3. **Advanced Templates**
   - Variadic templates
   - Template metaprogramming
   - SFINAE (Substitution Failure Is Not An Error)
   - Type traits
   - Concepts (C++20)

4. **Lambda Expressions (C++11+)**
   - Capture modes
   - Generic lambdas (C++14)
   - Init captures (C++14)
   - Constexpr lambdas (C++17)

5. **Type Deduction**
   - auto keyword
   - decltype
   - trailing return types

6. **Advanced Type System**
   - std::variant (C++17)
   - std::optional (C++17)
   - std::any (C++17)
   - std::expected (C++23)

## Code Examples

1. `01_move_semantics.cpp` - Move operations for zero-copy efficiency
2. `02_smart_pointers.cpp` - Automatic memory management
3. `03_advanced_templates.cpp` - Template metaprogramming
4. `04_lambdas.cpp` - Function objects and closures
5. `05_type_deduction.cpp` - Auto, decltype, and type inference
6. `06_modern_types.cpp` - Optional, variant, expected

## Key Benefits for HFT

- **Move Semantics**: Zero-copy data transfer for large market data
- **Smart Pointers**: No memory leaks, automatic cleanup
- **Templates**: Zero-overhead generic code
- **Lambdas**: Inline callback functions for strategies
- **Type Safety**: Catch errors at compile time

## Compilation

```bash
# C++17
g++ -std=c++17 -O3 -Wall -Wextra <filename.cpp> -o <output>

# C++20 (for concepts)
g++ -std=c++20 -O3 -Wall -Wextra <filename.cpp> -o <output>

# C++23 (for std::expected)
g++ -std=c++23 -O3 -Wall -Wextra <filename.cpp> -o <output>
```

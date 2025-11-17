# Module 2: Intermediate C++

## Topics Covered

1. Object-Oriented Programming (OOP)
   - Classes and Objects
   - Constructors and Destructors
   - Inheritance
   - Polymorphism (Virtual Functions)
   - Encapsulation and Access Control

2. Operator Overloading
   - Arithmetic operators
   - Comparison operators
   - Stream operators (<<, >>)

3. Standard Template Library (STL)
   - Containers (vector, map, set, unordered_map, deque, etc.)
   - Iterators
   - Algorithms (sort, find, binary_search, etc.)

4. Basic Templates
   - Function templates
   - Class templates
   - Template specialization

5. Exception Handling
   - Try-catch blocks
   - Custom exceptions
   - RAII and exception safety

6. File I/O (Advanced)
   - Serialization/Deserialization
   - Binary formats

7. Namespaces and Scope

## Code Examples

1. `01_classes_objects.cpp` - OOP basics, constructors, destructors
2. `02_inheritance_polymorphism.cpp` - Inheritance hierarchies, virtual functions
3. `03_operator_overloading.cpp` - Custom operators for financial types
4. `04_stl_containers.cpp` - Vector, map, set with trading examples
5. `05_stl_algorithms.cpp` - Sorting, searching market data
6. `06_templates.cpp` - Generic programming
7. `07_exceptions.cpp` - Error handling in trading systems
8. `08_namespaces.cpp` - Code organization

## Compilation

```bash
g++ -std=c++17 -O2 -Wall -Wextra <filename.cpp> -o <output>
```

## Key Concepts for HFT/Finance

- **OOP**: Model financial instruments as objects (Stock, Option, Bond)
- **STL**: Efficient data structures for order books, price history
- **Templates**: Generic algorithms for different asset types
- **Exceptions**: Robust error handling without performance penalty in normal flow
- **Operator Overloading**: Natural syntax for financial calculations

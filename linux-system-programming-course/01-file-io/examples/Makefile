# Makefile for File I/O Examples

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lpthread

# Source files
SOURCES = file_operations.c file_copy.c log_manager.c mmap_example.c
# Executables
TARGETS = file_operations file_copy log_manager mmap_example

.PHONY: all clean test

all: $(TARGETS)

file_operations: file_operations.c
	$(CC) $(CFLAGS) $< -o $@

file_copy: file_copy.c
	$(CC) $(CFLAGS) $< -o $@

log_manager: log_manager.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

mmap_example: mmap_example.c
	$(CC) $(CFLAGS) $< -o $@

# Run basic tests
test: all
	@echo "Running file_operations..."
	./file_operations
	@echo "\nCreating test file for file_copy..."
	echo "Hello, World!" > test_input.txt
	./file_copy test_input.txt test_output_copy.txt
	@echo "\nRunning mmap_example..."
	./mmap_example

clean:
	rm -f $(TARGETS)
	rm -f *.txt *.log *.dat *.o
	@echo "Cleaned up executables and test files"

help:
	@echo "Available targets:"
	@echo "  all          - Build all examples"
	@echo "  clean        - Remove executables and test files"
	@echo "  test         - Build and run basic tests"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Individual targets:"
	@echo "  file_operations - Basic file I/O operations"
	@echo "  file_copy       - File copy utility"
	@echo "  log_manager     - Thread-safe logging system"
	@echo "  mmap_example    - Memory-mapped file I/O"

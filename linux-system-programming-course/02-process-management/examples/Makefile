# Makefile for Process Management Examples

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

TARGETS = fork_basic exec_demo wait_demo daemon_process

.PHONY: all clean test

all: $(TARGETS)

fork_basic: fork_basic.c
	$(CC) $(CFLAGS) $< -o $@

exec_demo: exec_demo.c
	$(CC) $(CFLAGS) $< -o $@

wait_demo: wait_demo.c
	$(CC) $(CFLAGS) $< -o $@

daemon_process: daemon_process.c
	$(CC) $(CFLAGS) $< -o $@

test: all
	@echo "Running fork_basic..."
	./fork_basic
	@echo "\nRunning wait_demo..."
	./wait_demo

clean:
	rm -f $(TARGETS)
	rm -f /tmp/daemon.log /tmp/daemon.pid
	@echo "Cleaned up"

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

TARGETS = pipe_demo shared_memory

all: $(TARGETS)

pipe_demo: pipe_demo.c
	$(CC) $(CFLAGS) $< -o $@

shared_memory: shared_memory.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)
	ipcrm -a 2>/dev/null || true

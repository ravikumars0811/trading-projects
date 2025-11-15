CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lpthread

TARGETS = thread_basic thread_sync

all: $(TARGETS)

thread_basic: thread_basic.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

thread_sync: thread_sync.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)

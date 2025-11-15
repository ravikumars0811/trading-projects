CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

TARGETS = signal_handler

all: $(TARGETS)

signal_handler: signal_handler.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

TARGETS = tcp_server tcp_client

all: $(TARGETS)

tcp_server: tcp_server.c
	$(CC) $(CFLAGS) $< -o $@

tcp_client: tcp_client.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

test: all
	@echo "Start server in one terminal: ./tcp_server"
	@echo "Start client in another terminal: ./tcp_client"

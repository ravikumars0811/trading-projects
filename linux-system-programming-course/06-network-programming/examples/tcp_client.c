/*
 * tcp_client.c - Simple TCP Client
 *
 * Demonstrates:
 * - Creating TCP socket
 * - Connecting to server
 * - Sending and receiving data
 *
 * Usage: ./tcp_client [server_ip] [port]
 * Example: ./tcp_client 127.0.0.1 8080
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8080

int main(int argc, char *argv[]) {
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char *server_ip = "127.0.0.1";
    int port = DEFAULT_PORT;

    if (argc > 1) {
        server_ip = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }

    printf("=== TCP Client ===\n");

    // 1. Create socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        exit(1);
    }
    printf("Socket created\n");

    // 2. Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", server_ip);
        close(sock_fd);
        exit(1);
    }

    // 3. Connect to server
    printf("Connecting to %s:%d...\n", server_ip, port);
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock_fd);
        exit(1);
    }
    printf("Connected to server!\n");
    printf("Type messages (or 'quit' to exit):\n\n");

    // 4. Exchange data
    while (1) {
        printf("> ");
        fflush(stdout);

        // Get user input
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }

        // Remove newline if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        if (len == 0) {
            continue;
        }

        // Send to server
        ssize_t bytes_sent = send(sock_fd, buffer, len, 0);
        if (bytes_sent < 0) {
            perror("send");
            break;
        }

        // Check for quit
        if (strcmp(buffer, "quit") == 0) {
            printf("Closing connection...\n");
            break;
        }

        // Receive echo from server
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received < 0) {
            perror("recv");
            break;
        } else if (bytes_received == 0) {
            printf("Server closed connection\n");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Echo from server: %s\n", buffer);
    }

    // 5. Close connection
    close(sock_fd);
    printf("Connection closed\n");

    return 0;
}

/*
 * tcp_server.c - Simple TCP Echo Server
 *
 * Demonstrates:
 * - Creating TCP socket
 * - Binding to port
 * - Accepting connections
 * - Echoing data back to client
 *
 * Usage: ./tcp_server [port]
 * Test with: telnet localhost 8080
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024
#define BACKLOG 5

int main(int argc, char *argv[]) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    int port = DEFAULT_PORT;

    if (argc > 1) {
        port = atoi(argv[1]);
    }

    printf("=== TCP Echo Server ===\n");

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }
    printf("Socket created\n");

    // Set socket options (reuse address)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    // 2. Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Accept connections on any interface
    server_addr.sin_port = htons(port);

    // 3. Bind socket to address
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(1);
    }
    printf("Bound to port %d\n", port);

    // 4. Listen for connections
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        exit(1);
    }
    printf("Listening for connections... (Ctrl+C to stop)\n");
    printf("Test with: telnet localhost %d\n\n", port);

    // Main server loop
    while (1) {
        client_len = sizeof(client_addr);

        // 5. Accept client connection
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        char *client_ip = inet_ntoa(client_addr.sin_addr);
        int client_port = ntohs(client_addr.sin_port);
        printf("Client connected: %s:%d\n", client_ip, client_port);

        // 6. Exchange data with client
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_received < 0) {
                perror("recv");
                break;
            } else if (bytes_received == 0) {
                printf("Client disconnected: %s:%d\n", client_ip, client_port);
                break;
            }

            buffer[bytes_received] = '\0';
            printf("Received from %s:%d: %s", client_ip, client_port, buffer);

            // Echo back to client
            ssize_t bytes_sent = send(client_fd, buffer, bytes_received, 0);
            if (bytes_sent < 0) {
                perror("send");
                break;
            }

            printf("Echoed %zd bytes back\n", bytes_sent);

            // Check for quit command
            if (strncmp(buffer, "quit", 4) == 0) {
                printf("Client requested disconnect\n");
                break;
            }
        }

        // 7. Close client connection
        close(client_fd);
        printf("Connection closed\n\n");
    }

    close(server_fd);
    return 0;
}

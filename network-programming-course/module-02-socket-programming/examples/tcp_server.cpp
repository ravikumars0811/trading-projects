/**
 * Simple TCP Server Example
 *
 * This server listens on port 8080 and handles one client at a time.
 * It receives data and echoes it back to the client.
 */

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

constexpr int PORT = 8080;
constexpr int BACKLOG = 5;
constexpr int BUFFER_SIZE = 1024;

int main() {
    // 1. Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }
    std::cout << "Socket created successfully\n";

    // 2. Set socket options (allow address reuse)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    // 3. Bind socket to address
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(PORT);       // Convert to network byte order

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }
    std::cout << "Socket bound to port " << PORT << "\n";

    // 4. Listen for connections
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }
    std::cout << "Server listening on port " << PORT << "\n";

    // 5. Accept and handle clients
    while (true) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        // Get client IP address
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        std::cout << "Client connected from " << client_ip
                  << ":" << ntohs(client_addr.sin_port) << "\n";

        // 6. Receive and echo data
        char buffer[BUFFER_SIZE];
        while (true) {
            ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

            if (bytes_received < 0) {
                perror("recv");
                break;
            } else if (bytes_received == 0) {
                // Client closed connection
                std::cout << "Client disconnected\n";
                break;
            }

            // Null-terminate the received data
            buffer[bytes_received] = '\0';
            std::cout << "Received: " << buffer;

            // Echo back to client
            ssize_t bytes_sent = send(client_fd, buffer, bytes_received, 0);
            if (bytes_sent < 0) {
                perror("send");
                break;
            }
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}

/*
 * Compilation:
 * g++ -std=c++17 -O2 -Wall tcp_server.cpp -o tcp_server
 *
 * Usage:
 * ./tcp_server
 *
 * Test with telnet:
 * telnet localhost 8080
 *
 * Or with netcat:
 * nc localhost 8080
 */

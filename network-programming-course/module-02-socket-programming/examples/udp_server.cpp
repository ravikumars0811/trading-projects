/**
 * Simple UDP Server Example
 *
 * This server receives datagrams on port 8080 and echoes them back.
 */

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

int main() {
    // 1. Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }
    std::cout << "UDP socket created\n";

    // 2. Set socket options
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return 1;
    }

    // 3. Bind socket to address
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }
    std::cout << "Socket bound to port " << PORT << "\n";
    std::cout << "Waiting for datagrams...\n";

    // 4. Receive and echo datagrams
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    while (true) {
        // Receive datagram
        ssize_t bytes_received = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                          (struct sockaddr*)&client_addr, &client_len);

        if (bytes_received < 0) {
            perror("recvfrom");
            continue;
        }

        buffer[bytes_received] = '\0';

        // Get client information
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

        std::cout << "Received from " << client_ip << ":"
                  << ntohs(client_addr.sin_port) << ": " << buffer;

        // Echo back to client
        ssize_t bytes_sent = sendto(sockfd, buffer, bytes_received, 0,
                                    (struct sockaddr*)&client_addr, client_len);

        if (bytes_sent < 0) {
            perror("sendto");
        }
    }

    close(sockfd);
    return 0;
}

/*
 * Compilation:
 * g++ -std=c++17 -O2 -Wall udp_server.cpp -o udp_server
 *
 * Usage:
 * ./udp_server
 *
 * Test with netcat:
 * nc -u localhost 8080
 */

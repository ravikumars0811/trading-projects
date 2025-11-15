/**
 * High-Performance TCP Server
 *
 * This server demonstrates various performance optimizations:
 * - TCP_NODELAY (disable Nagle's algorithm)
 * - TCP_QUICKACK (disable delayed ACK)
 * - Large socket buffers
 * - SO_REUSEPORT (for multi-threaded scaling)
 */

#include <iostream>
#include <cstring>
#include <chrono>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

constexpr int PORT = 8080;
constexpr int BACKLOG = 128;
constexpr int BUFFER_SIZE = 8192;
constexpr int SOCKET_BUFFER_SIZE = 4 * 1024 * 1024; // 4MB

void set_socket_options(int sockfd) {
    int opt = 1;

    // SO_REUSEADDR: Allow immediate reuse of the address
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("SO_REUSEADDR");
    }

    // SO_REUSEPORT: Allow multiple sockets to bind to same port
    // Useful for multi-threaded servers
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("SO_REUSEPORT");
    }

    // Increase send buffer size
    int sndbuf = SOCKET_BUFFER_SIZE;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) < 0) {
        perror("SO_SNDBUF");
    }

    // Increase receive buffer size
    int rcvbuf = SOCKET_BUFFER_SIZE;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0) {
        perror("SO_RCVBUF");
    }

    // TCP_NODELAY: Disable Nagle's algorithm for low latency
    // Critical for interactive/real-time applications!
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
        perror("TCP_NODELAY");
    }

    // TCP_QUICKACK: Disable delayed ACK
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &opt, sizeof(opt)) < 0) {
        perror("TCP_QUICKACK");
    }

    // SO_KEEPALIVE: Enable TCP keepalive
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0) {
        perror("SO_KEEPALIVE");
    }

    // TCP keepalive parameters
    int keepidle = 60;  // Start probes after 60 seconds of inactivity
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle)) < 0) {
        perror("TCP_KEEPIDLE");
    }

    int keepintvl = 10; // Send probes every 10 seconds
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl)) < 0) {
        perror("TCP_KEEPINTVL");
    }

    int keepcnt = 3;    // Close after 3 failed probes
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt)) < 0) {
        perror("TCP_KEEPCNT");
    }

    // Verify buffer sizes (kernel may limit them)
    socklen_t len = sizeof(sndbuf);
    getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, &len);
    getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &len);

    std::cout << "Socket buffers - Send: " << sndbuf
              << " bytes, Recv: " << rcvbuf << " bytes\n";
}

void handle_client(int client_fd, const char* client_ip, uint16_t client_port) {
    std::cout << "Client connected from " << client_ip << ":" << client_port << "\n";

    // Set client socket options
    set_socket_options(client_fd);

    char buffer[BUFFER_SIZE];
    size_t total_bytes = 0;
    auto start_time = std::chrono::steady_clock::now();

    while (true) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

        if (bytes_received < 0) {
            if (errno == EINTR) {
                continue; // Interrupted, retry
            }
            perror("recv");
            break;
        } else if (bytes_received == 0) {
            // Client closed connection
            auto end_time = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time).count();

            double throughput = (total_bytes * 8.0 / 1000000.0) / (duration / 1000.0); // Mbps

            std::cout << "Client disconnected. Total bytes: " << total_bytes
                      << ", Duration: " << duration << "ms"
                      << ", Throughput: " << throughput << " Mbps\n";
            break;
        }

        total_bytes += bytes_received;

        // Echo back with proper error handling
        size_t total_sent = 0;
        while (total_sent < static_cast<size_t>(bytes_received)) {
            ssize_t bytes_sent = send(client_fd, buffer + total_sent,
                                     bytes_received - total_sent, 0);
            if (bytes_sent < 0) {
                if (errno == EINTR) {
                    continue;
                }
                perror("send");
                goto cleanup;
            }
            total_sent += bytes_sent;
        }

        // Re-enable TCP_QUICKACK (it's not persistent)
        int opt = 1;
        setsockopt(client_fd, IPPROTO_TCP, TCP_QUICKACK, &opt, sizeof(opt));
    }

cleanup:
    close(client_fd);
}

int main() {
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }
    std::cout << "High-performance TCP server starting...\n";

    // Set server socket options
    set_socket_options(server_fd);

    // Bind
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // Listen with larger backlog for high connection rate
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "\n";
    std::cout << "Optimizations enabled:\n";
    std::cout << "  - TCP_NODELAY (disabled Nagle's algorithm)\n";
    std::cout << "  - TCP_QUICKACK (disabled delayed ACK)\n";
    std::cout << "  - Large socket buffers (" << SOCKET_BUFFER_SIZE << " bytes)\n";
    std::cout << "  - SO_REUSEPORT (multi-thread ready)\n";
    std::cout << "  - TCP keepalive enabled\n";

    // Accept and handle clients
    while (true) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        uint16_t client_port = ntohs(client_addr.sin_port);

        // In production, spawn a thread or use async I/O here
        handle_client(client_fd, client_ip, client_port);
    }

    close(server_fd);
    return 0;
}

/*
 * Compilation:
 * g++ -std=c++17 -O3 -Wall -march=native optimized_tcp_server.cpp -o optimized_tcp_server
 *
 * Usage:
 * ./optimized_tcp_server
 *
 * Benchmark with iperf3:
 * iperf3 -c localhost -p 8080 -t 10
 *
 * Performance tips:
 * - Run with taskset to pin to specific CPU cores
 * - Disable CPU frequency scaling for consistent results
 * - Monitor with: watch -n 1 'netstat -s | grep -i retrans'
 */

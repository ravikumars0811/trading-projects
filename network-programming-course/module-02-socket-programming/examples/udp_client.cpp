/**
 * Simple UDP Client Example
 *
 * This client sends datagrams to a server and receives responses.
 */

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

int main(int argc, char* argv[]) {
    const char* server_ip = (argc > 1) ? argv[1] : "127.0.0.1";

    // 1. Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }
    std::cout << "UDP socket created\n";

    // 2. Setup server address
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << server_ip << "\n";
        close(sockfd);
        return 1;
    }

    std::cout << "Ready to send to " << server_ip << ":" << PORT << "\n";
    std::cout << "Enter messages (Ctrl+D to quit):\n";

    // 3. Send and receive datagrams
    char buffer[BUFFER_SIZE];
    std::string message;

    while (std::getline(std::cin, message)) {
        message += "\n";

        // Send datagram
        ssize_t bytes_sent = sendto(sockfd, message.c_str(), message.length(), 0,
                                    (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (bytes_sent < 0) {
            perror("sendto");
            continue;
        }

        // Receive response
        struct sockaddr_in recv_addr{};
        socklen_t recv_len = sizeof(recv_addr);

        ssize_t bytes_received = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                          (struct sockaddr*)&recv_addr, &recv_len);

        if (bytes_received < 0) {
            perror("recvfrom");
            continue;
        }

        buffer[bytes_received] = '\0';
        std::cout << "Server echo: " << buffer;
    }

    std::cout << "\nClosing socket\n";
    close(sockfd);
    return 0;
}

/*
 * Compilation:
 * g++ -std=c++17 -O2 -Wall udp_client.cpp -o udp_client
 *
 * Usage:
 * ./udp_client [server_ip]
 *
 * Example:
 * ./udp_client 127.0.0.1
 */

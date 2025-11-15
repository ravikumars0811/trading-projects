/**
 * Simple TCP Client Example
 *
 * This client connects to a server and sends/receives messages.
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

    // 1. Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }
    std::cout << "Socket created\n";

    // 2. Setup server address
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IP address from text to binary
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << server_ip << "\n";
        close(sockfd);
        return 1;
    }

    // 3. Connect to server
    std::cout << "Connecting to " << server_ip << ":" << PORT << "...\n";
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }
    std::cout << "Connected to server\n";

    // 4. Send and receive messages
    char buffer[BUFFER_SIZE];
    std::string message;

    std::cout << "Enter messages (Ctrl+D to quit):\n";
    while (std::getline(std::cin, message)) {
        message += "\n"; // Add newline

        // Send message
        ssize_t bytes_sent = send(sockfd, message.c_str(), message.length(), 0);
        if (bytes_sent < 0) {
            perror("send");
            break;
        }

        // Receive echo
        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0) {
            perror("recv");
            break;
        } else if (bytes_received == 0) {
            std::cout << "Server closed connection\n";
            break;
        }

        buffer[bytes_received] = '\0';
        std::cout << "Server echo: " << buffer;
    }

    std::cout << "\nClosing connection\n";
    close(sockfd);
    return 0;
}

/*
 * Compilation:
 * g++ -std=c++17 -O2 -Wall tcp_client.cpp -o tcp_client
 *
 * Usage:
 * ./tcp_client [server_ip]
 *
 * Example:
 * ./tcp_client 127.0.0.1
 */

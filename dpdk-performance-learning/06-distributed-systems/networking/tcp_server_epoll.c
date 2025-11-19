/*
 * High-Performance TCP Server using epoll
 *
 * Demonstrates:
 * - Non-blocking I/O with epoll
 * - Edge-triggered mode
 * - TCP optimizations
 * - Connection handling
 *
 * Build: gcc -O2 -o tcp_server_epoll tcp_server_epoll.c
 * Run: ./tcp_server_epoll 8080
 * Test: telnet localhost 8080
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096
#define BACKLOG 128

/* Set socket to non-blocking mode */
static int set_nonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        return -1;
    }

    return 0;
}

/* Optimize TCP socket */
static void optimize_tcp_socket(int sockfd)
{
    int flag = 1;

    /* Disable Nagle's algorithm for low latency */
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
        perror("setsockopt TCP_NODELAY");
    }

    /* Enable TCP quickack */
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag)) < 0) {
        perror("setsockopt TCP_QUICKACK");
    }

    /* Set receive buffer size */
    int bufsize = 256 * 1024;  /* 256 KB */
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize)) < 0) {
        perror("setsockopt SO_RCVBUF");
    }

    /* Set send buffer size */
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize)) < 0) {
        perror("setsockopt SO_SNDBUF");
    }

    /* Enable SO_REUSEADDR */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        perror("setsockopt SO_REUSEADDR");
    }

    /* Enable SO_REUSEPORT for load balancing */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag)) < 0) {
        perror("setsockopt SO_REUSEPORT");
    }
}

/* Create listening socket */
static int create_listen_socket(int port)
{
    int sockfd;
    struct sockaddr_in addr;

    /* Create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    /* Optimize socket */
    optimize_tcp_socket(sockfd);

    /* Set non-blocking */
    if (set_nonblocking(sockfd) < 0) {
        close(sockfd);
        return -1;
    }

    /* Bind */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    /* Listen */
    if (listen(sockfd, BACKLOG) < 0) {
        perror("listen");
        close(sockfd);
        return -1;
    }

    printf("Listening on port %d\n", port);
    return sockfd;
}

/* Accept new connection */
static void accept_connection(int listen_fd, int epoll_fd)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int conn_fd;
    struct epoll_event event;

    /* Accept connection */
    conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (conn_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("accept");
        }
        return;
    }

    printf("New connection from %s:%d (fd=%d)\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port),
           conn_fd);

    /* Optimize and set non-blocking */
    optimize_tcp_socket(conn_fd);
    set_nonblocking(conn_fd);

    /* Add to epoll - Edge Triggered mode */
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = conn_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event) < 0) {
        perror("epoll_ctl ADD");
        close(conn_fd);
    }
}

/* Handle client data */
static void handle_client(int client_fd, int epoll_fd)
{
    char buffer[BUFFER_SIZE];
    ssize_t n;

    /* Read all available data (edge-triggered) */
    while (1) {
        n = read(client_fd, buffer, sizeof(buffer));

        if (n > 0) {
            /* Echo back to client */
            ssize_t written = 0;
            while (written < n) {
                ssize_t w = write(client_fd, buffer + written, n - written);
                if (w < 0) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        perror("write");
                        goto close_conn;
                    }
                    break;
                }
                written += w;
            }
        } else if (n == 0) {
            /* Connection closed */
            printf("Client disconnected (fd=%d)\n", client_fd);
            goto close_conn;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                /* No more data */
                break;
            }
            perror("read");
            goto close_conn;
        }
    }

    return;

close_conn:
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    close(client_fd);
}

/* Main event loop */
static void event_loop(int listen_fd)
{
    int epoll_fd;
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];
    int nfds;

    /* Create epoll instance */
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        return;
    }

    /* Add listen socket to epoll */
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) < 0) {
        perror("epoll_ctl ADD listen_fd");
        close(epoll_fd);
        return;
    }

    printf("Event loop started (epoll edge-triggered mode)\n");
    printf("Waiting for connections...\n\n");

    /* Event loop */
    while (1) {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }

        /* Process events */
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_fd) {
                /* New connection */
                accept_connection(listen_fd, epoll_fd);
            } else {
                /* Client data */
                handle_client(events[i].data.fd, epoll_fd);
            }
        }
    }

    close(epoll_fd);
}

int main(int argc, char *argv[])
{
    int port = 8080;
    int listen_fd;

    if (argc > 1) {
        port = atoi(argv[1]);
    }

    printf("High-Performance TCP Server with epoll\n");
    printf("======================================\n\n");

    /* Create listening socket */
    listen_fd = create_listen_socket(port);
    if (listen_fd < 0) {
        return 1;
    }

    printf("\nServer configuration:\n");
    printf("  - Non-blocking I/O\n");
    printf("  - epoll edge-triggered mode\n");
    printf("  - TCP_NODELAY enabled\n");
    printf("  - TCP_QUICKACK enabled\n");
    printf("  - Large socket buffers (256KB)\n");
    printf("  - SO_REUSEPORT enabled\n\n");

    /* Run event loop */
    event_loop(listen_fd);

    close(listen_fd);
    return 0;
}

/*
 * Key Performance Features:
 *
 * 1. epoll Edge-Triggered Mode
 *    - More efficient than level-triggered
 *    - Fewer system calls
 *    - Must read all data when notified
 *
 * 2. TCP_NODELAY
 *    - Disables Nagle's algorithm
 *    - Reduces latency for small messages
 *    - Critical for interactive applications
 *
 * 3. Large Socket Buffers
 *    - Reduces packet loss
 *    - Better throughput for bulk transfers
 *
 * 4. SO_REUSEPORT
 *    - Allows multiple processes to bind same port
 *    - Kernel load balances connections
 *    - Perfect for multi-process servers
 *
 * Testing:
 *   telnet localhost 8080
 *   nc localhost 8080
 *   ab -n 100000 -c 100 http://localhost:8080/  (if HTTP)
 */

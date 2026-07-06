#include "server.h"
#include "logger.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "http_request.h"

#define BACKLOG 128
#define READ_BUFFER_SIZE 4096

static int send_fixed_response(int client_fd) {
    const char *body = "Hello from Cerveur\n";

    char response[512];
    int response_len = snprintf(
        response,
        sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        strlen(body),
        body
    );

    if (response_len < 0 || (size_t)response_len >= sizeof(response)) {
        logger_error("Failed to build HTTP response");
        return -1;
    }

    ssize_t sent = send(client_fd, response, (size_t)response_len, 0);

    if (sent < 0) {
        logger_error("send() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

void server_init(Server *server, ServerConfig *config) {
    server->socket_fd = -1;
    server->config = config;
}

int server_start(Server *server) {
    server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server->socket_fd < 0) {
        logger_error("socket() failed: %s", strerror(errno));
        return -1;
    }

    int reuse = 1;

    if (setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        logger_error("setsockopt() failed: %s", strerror(errno));
        return -1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons((uint16_t)server->config->port);

    if (bind(server->socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        logger_error("bind() failed on port %d: %s", server->config->port, strerror(errno));
        return -1;
    }

    if (listen(server->socket_fd, BACKLOG) < 0) {
        logger_error("listen() failed: %s", strerror(errno));
        return -1;
    }

    logger_info("Cerveur listening on port %d", server->config->port);

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        int client_fd = accept(
            server->socket_fd,
            (struct sockaddr *)&client_address,
            &client_address_len
        );

        if (client_fd < 0) {
            logger_error("accept() failed: %s", strerror(errno));
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];

        inet_ntop(
            AF_INET,
            &client_address.sin_addr,
            client_ip,
            sizeof(client_ip)
        );

        logger_info("Accepted connection from %s", client_ip);

        char buffer[READ_BUFFER_SIZE];
        ssize_t received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (received < 0) {
            logger_error("recv() failed: %s", strerror(errno));
            close(client_fd);
            continue;
        }

        buffer[received] = '\0';

        logger_info("Received request:");
        fprintf(stderr, "%s\n", buffer);

        HttpRequest req;
        http_request_init(&req);

        if (http_request_parse(&req, buffer, (size_t)received, server->config) == 0) {
            logger_info("Parsed method: %s", http_method_to_string(req.method));
            logger_info("Parsed path: %s", req.path);

            if (req.query_string != NULL) {
                logger_info("Parsed query: %s", req.query_string);
            }

            logger_info("Parsed version: %s", req.version);
        } else {
            logger_warn("Failed to parse HTTP request line");
        }

        http_request_free(&req);

        send_fixed_response(client_fd);

        close(client_fd);

        logger_info("Connection closed");
    }

    return 0;
}

void server_free(Server *server) {
    if (server->socket_fd >= 0) {
        close(server->socket_fd);
        server->socket_fd = -1;
    }
}
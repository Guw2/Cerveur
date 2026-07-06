#ifndef CERVEUR_SERVER_H
#define CERVEUR_SERVER_H

#include "config.h"

typedef struct {
    int socket_fd;
    ServerConfig *config;
} Server;

void server_init(Server *server, ServerConfig *config);
int server_start(Server *server);
void server_free(Server *server);

#endif
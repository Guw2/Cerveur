#ifndef CERVEUR_CONFIG_H
#define CERVEUR_CONFIG_H

#include <stdbool.h>
#include <stddef.h>

typedef struct{
    int port;
    char *public_dir;
    size_t max_request_size;
    size_t max_headers;
    char *log_level;
    bool log_colors;
} ServerConfig;

void server_config_default(ServerConfig *config);
int server_config_load(ServerConfig *config, const char *path);
void server_config_free(ServerConfig *config);

#endif
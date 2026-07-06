#include "config.h"
#include "logger.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *str_duplicate(const char *src) {
    if (src == NULL) {
        return NULL;
    }

    size_t len = strlen(src);
    char *copy = malloc(len + 1);

    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, src, len + 1);
    return copy;
}

static char *trim(char *str) {
    char *end;

    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '\0') {
        return str;
    }

    end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return str;
}

static bool parse_bool(const char *value, bool fallback) {
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
        return true;
    }

    if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0) {
        return false;
    }

    return fallback;
}

static void replace_string(char **target, const char *value) {
    char *copy = str_duplicate(value);

    if (copy == NULL) {
        logger_error("Failed to allocate memory for config string");
        return;
    }

    free(*target);
    *target = copy;
}

void server_config_default(ServerConfig *config) {
    config->port = 8080;
    config->public_dir = str_duplicate("./public");
    config->max_request_size = 65536;
    config->max_headers = 64;
    config->log_level = str_duplicate("info");
    config->log_colors = true;
}

int server_config_load(ServerConfig *config, const char *path) {
    FILE *file = fopen(path, "r");

    if (file == NULL) {
        logger_warn("Config file '%s' not found. Using defaults.", path);
        return -1;
    }

    char line[1024];

    while (fgets(line, sizeof(line), file) != NULL) {
        char *trimmed = trim(line);

        if (*trimmed == '\0' || *trimmed == '#') {
            continue;
        }

        char *equals = strchr(trimmed, '=');

        if (equals == NULL) {
            logger_warn("Invalid config line ignored: %s", trimmed);
            continue;
        }

        *equals = '\0';

        char *key = trim(trimmed);
        char *value = trim(equals + 1);

        if (strcmp(key, "port") == 0) {
            int port = atoi(value);

            if (port <= 0 || port > 65535) {
                logger_warn("Invalid port '%s'. Keeping default %d.", value, config->port);
                continue;
            }

            config->port = port;
        } else if (strcmp(key, "public_dir") == 0) {
            replace_string(&config->public_dir, value);
        } else if (strcmp(key, "max_request_size") == 0) {
            long parsed = atol(value);

            if (parsed <= 0) {
                logger_warn("Invalid max_request_size '%s'. Keeping default %zu.", value, config->max_request_size);
                continue;
            }

            config->max_request_size = (size_t)parsed;
        } else if (strcmp(key, "max_headers") == 0) {
            long parsed = atol(value);

            if (parsed <= 0) {
                logger_warn("Invalid max_headers '%s'. Keeping default %zu.", value, config->max_headers);
                continue;
            }

            config->max_headers = (size_t)parsed;
        } else if (strcmp(key, "log_level") == 0) {
            replace_string(&config->log_level, value);
        } else if (strcmp(key, "log_colors") == 0) {
            config->log_colors = parse_bool(value, config->log_colors);
        } else {
            logger_warn("Unknown config key ignored: %s", key);
        }
    }

    fclose(file);
    return 0;
}

void server_config_free(ServerConfig *config) {
    free(config->public_dir);
    free(config->log_level);

    config->public_dir = NULL;
    config->log_level = NULL;
}
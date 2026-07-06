#ifndef CERVEUR_HTTP_REQUEST_H
#define CERVEUR_HTTP_REQUEST_H

#include "config.h"
#include "http_method.h"

#include <stddef.h>

typedef struct {
    HttpMethod method;
    char *method_raw;
    char *path;
    char *query_string;
    char *version;

    char *body;
    size_t body_length;
} HttpRequest;

void http_request_init(HttpRequest *req);

int http_request_parse(HttpRequest *req, const char *raw, size_t raw_len, const ServerConfig *config);

void http_request_free(HttpRequest *req);

#endif
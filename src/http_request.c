#include "http_request.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static char *str_duplicate_range(const char *start, size_t len) {
    char *copy = malloc(len + 1);

    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, start, len);
    copy[len] = '\0';

    return copy;
}

static const char *skip_spaces(const char *ptr) {
    while (*ptr != '\0' && isspace((unsigned char)*ptr)) {
        ptr++;
    }

    return ptr;
}

static const char *read_token(const char *start, const char **end) {
    const char *ptr = start;

    while (*ptr != '\0' && !isspace((unsigned char)*ptr)) {
        ptr++;
    }

    *end = ptr;
    return start;
}

static int parse_target(HttpRequest *req, const char *target_start, size_t target_len) {
    const char *question = memchr(target_start, '?', target_len);

    if (question == NULL) {
        req->path = str_duplicate_range(target_start, target_len);
        req->query_string = NULL;

        return req->path == NULL ? -1 : 0;
    }

    size_t path_len = (size_t)(question - target_start);
    size_t query_len = target_len - path_len - 1;

    req->path = str_duplicate_range(target_start, path_len);
    req->query_string = str_duplicate_range(question + 1, query_len);

    if (req->path == NULL || req->query_string == NULL) {
        return -1;
    }

    return 0;
}

void http_request_init(HttpRequest *req) {
    req->method = HTTP_METHOD_UNKNOWN;
    req->method_raw = NULL;
    req->path = NULL;
    req->query_string = NULL;
    req->version = NULL;
    req->body = NULL;
    req->body_length = 0;
}

int http_request_parse(HttpRequest *req, const char *raw, size_t raw_len, const ServerConfig *config) {
    (void)config;

    if (raw == NULL || raw_len == 0) {
        return -1;
    }

    const char *line_end = memchr(raw, '\n', raw_len);

    if (line_end == NULL) {
        return -1;
    }

    size_t line_len = (size_t)(line_end - raw);

    if (line_len > 0 && raw[line_len - 1] == '\r') {
        line_len--;
    }

    const char *line_start = raw;
    const char *line_limit = raw + line_len;

    const char *method_start = skip_spaces(line_start);
    const char *method_end = NULL;

    read_token(method_start, &method_end);

    if (method_start == method_end || method_end > line_limit) {
        return -1;
    }

    const char *target_start = skip_spaces(method_end);
    const char *target_end = NULL;

    read_token(target_start, &target_end);

    if (target_start == target_end || target_end > line_limit) {
        return -1;
    }

    const char *version_start = skip_spaces(target_end);
    const char *version_end = NULL;

    read_token(version_start, &version_end);

    if (version_start == version_end || version_end > line_limit) {
        return -1;
    }

    const char *extra = skip_spaces(version_end);

    if (extra < line_limit) {
        return -1;
    }

    size_t method_len = (size_t)(method_end - method_start);
    size_t target_len = (size_t)(target_end - target_start);
    size_t version_len = (size_t)(version_end - version_start);

    req->method_raw = str_duplicate_range(method_start, method_len);

    if (req->method_raw == NULL) {
        return -1;
    }

    req->method = http_method_from_string(req->method_raw);

    if (parse_target(req, target_start, target_len) != 0) {
        return -1;
    }

    req->version = str_duplicate_range(version_start, version_len);

    if (req->version == NULL) {
        return -1;
    }

    return 0;
}

void http_request_free(HttpRequest *req) {
    free(req->method_raw);
    free(req->path);
    free(req->query_string);
    free(req->version);
    free(req->body);

    req->method_raw = NULL;
    req->path = NULL;
    req->query_string = NULL;
    req->version = NULL;
    req->body = NULL;
    req->body_length = 0;
}
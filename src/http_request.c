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

static char *str_duplicate_lower_range(const char *start, size_t len) {
    char *copy = malloc(len + 1);

    if (copy == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < len; i++) {
        copy[i] = (char)tolower((unsigned char)start[i]);
    }

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

static char *trim_copy(const char *start, size_t len) {
    while (len > 0 && isspace((unsigned char)*start)) {
        start++;
        len--;
    }

    while (len > 0 && isspace((unsigned char)start[len - 1])) {
        len--;
    }

    return str_duplicate_range(start, len);
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

static int add_header(HttpRequest *req, const char *name_start, size_t name_len, const char *value_start, size_t value_len) {
    if (req->header_count >= req->header_capacity) {
        size_t new_capacity = req->header_capacity == 0 ? 8 : req->header_capacity * 2;

        HttpHeader *new_headers = realloc(req->headers, new_capacity * sizeof(HttpHeader));

        if (new_headers == NULL) {
            return -1;
        }

        req->headers = new_headers;
        req->header_capacity = new_capacity;
    }

    char *name = str_duplicate_lower_range(name_start, name_len);
    char *value = trim_copy(value_start, value_len);

    if (name == NULL || value == NULL) {
        free(name);
        free(value);
        return -1;
    }

    req->headers[req->header_count].name = name;
    req->headers[req->header_count].value = value;
    req->header_count++;

    return 0;
}

static int parse_request_line(HttpRequest *req, const char *line_start, size_t line_len) {
    const char *line_limit = line_start + line_len;

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

static int parse_headers(HttpRequest *req, const char *headers_start, const char *raw_end, const ServerConfig *config) {
    const char *line_start = headers_start;

    while (line_start < raw_end) {
        const char *line_end = memchr(line_start, '\n', (size_t)(raw_end - line_start));

        if (line_end == NULL) {
            break;
        }

        size_t line_len = (size_t)(line_end - line_start);

        if (line_len > 0 && line_start[line_len - 1] == '\r') {
            line_len--;
        }

        if (line_len == 0) {
            return 0;
        }

        if (req->header_count >= config->max_headers) {
            return -1;
        }

        const char *colon = memchr(line_start, ':', line_len);

        if (colon == NULL) {
            return -1;
        }

        size_t name_len = (size_t)(colon - line_start);
        const char *value_start = colon + 1;
        size_t value_len = line_len - name_len - 1;

        if (name_len == 0) {
            return -1;
        }

        if (add_header(req, line_start, name_len, value_start, value_len) != 0) {
            return -1;
        }

        line_start = line_end + 1;
    }

    return 0;
}

static char *lowercase_copy(const char *src) {
    size_t len = strlen(src);
    char *copy = malloc(len + 1);

    if (copy == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < len; i++) {
        copy[i] = (char)tolower((unsigned char)src[i]);
    }

    copy[len] = '\0';

    return copy;
}

void http_request_init(HttpRequest *req) {
    req->method = HTTP_METHOD_UNKNOWN;
    req->method_raw = NULL;
    req->path = NULL;
    req->query_string = NULL;
    req->version = NULL;
    req->headers = NULL;
    req->header_count = 0;
    req->header_capacity = 0;
    req->body = NULL;
    req->body_length = 0;
}

int http_request_parse(HttpRequest *req, const char *raw, size_t raw_len, const ServerConfig *config) {
    if (raw == NULL || raw_len == 0 || config == NULL) {
        return -1;
    }

    if (raw_len > config->max_request_size) {
        return -1;
    }

    const char *raw_end = raw + raw_len;

    const char *line_end = memchr(raw, '\n', raw_len);

    if (line_end == NULL) {
        return -1;
    }

    size_t line_len = (size_t)(line_end - raw);

    if (line_len > 0 && raw[line_len - 1] == '\r') {
        line_len--;
    }

    if (parse_request_line(req, raw, line_len) != 0) {
        return -1;
    }

    const char *headers_start = line_end + 1;

    if (parse_headers(req, headers_start, raw_end, config) != 0) {
        return -1;
    }

    return 0;
}

const char *request_header(const HttpRequest *req, const char *name) {
    if (req == NULL || name == NULL) {
        return NULL;
    }

    char *lower_name = lowercase_copy(name);

    if (lower_name == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < req->header_count; i++) {
        if (strcmp(req->headers[i].name, lower_name) == 0) {
            free(lower_name);
            return req->headers[i].value;
        }
    }

    free(lower_name);
    return NULL;
}

void http_request_free(HttpRequest *req) {
    free(req->method_raw);
    free(req->path);
    free(req->query_string);
    free(req->version);
    free(req->body);

    for (size_t i = 0; i < req->header_count; i++) {
        free(req->headers[i].name);
        free(req->headers[i].value);
    }

    free(req->headers);

    req->method_raw = NULL;
    req->path = NULL;
    req->query_string = NULL;
    req->version = NULL;
    req->headers = NULL;
    req->header_count = 0;
    req->header_capacity = 0;
    req->body = NULL;
    req->body_length = 0;
}
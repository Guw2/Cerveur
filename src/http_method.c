#include "http_method.h"

#include <string.h>

HttpMethod http_method_from_string(const char *method) {
    if (method == NULL) {
        return HTTP_METHOD_UNKNOWN;
    }

    if (strcmp(method, "GET") == 0) {
        return HTTP_GET;
    }

    if (strcmp(method, "POST") == 0) {
        return HTTP_POST;
    }

    if (strcmp(method, "PUT") == 0) {
        return HTTP_PUT;
    }

    if (strcmp(method, "PATCH") == 0) {
        return HTTP_PATCH;
    }

    if (strcmp(method, "DELETE") == 0) {
        return HTTP_DELETE;
    }

    if (strcmp(method, "OPTIONS") == 0) {
        return HTTP_OPTIONS;
    }

    if (strcmp(method, "HEAD") == 0) {
        return HTTP_HEAD;
    }

    return HTTP_METHOD_UNKNOWN;
}

const char *http_method_to_string(HttpMethod method) {
    switch (method) {
        case HTTP_GET:
            return "GET";
        case HTTP_POST:
            return "POST";
        case HTTP_PUT:
            return "PUT";
        case HTTP_PATCH:
            return "PATCH";
        case HTTP_DELETE:
            return "DELETE";
        case HTTP_OPTIONS:
            return "OPTIONS";
        case HTTP_HEAD:
            return "HEAD";
        case HTTP_METHOD_UNKNOWN:
        default:
            return "UNKNOWN";
    }
}
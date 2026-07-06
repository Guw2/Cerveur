#ifndef CERVEUR_HTTP_METHOD_H
#define CERVEUR_HTTP_METHOD_H

typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_PATCH,
    HTTP_HEAD,
    HTTP_OPTIONS,
    HTTP_METHOD_UNKNOWN
} HttpMethod;

HttpMethod http_method_from_string(const char *method);
const char *http_method_to_string(HttpMethod method);

#endif
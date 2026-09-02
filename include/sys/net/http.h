#ifndef HTTP_H
#define HTTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct HttpClient HttpClient;
typedef struct HttpRequest HttpRequest;
typedef struct HttpResponse HttpResponse;

typedef enum HttpMethod {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_PATCH,
    HTTP_DELETE,
    HTTP_HEAD,
    HTTP_OPTIONS
} HttpMethod;

typedef enum HttpProtocol {
    HTTP,
    HTTPS
} HttpProtocol;


/* Client */

HttpClient *http_client_create(void);

int http_client_request(
    HttpClient *client,
    HttpMethod method,
    const char *url,
    HttpRequest **request
);

int http_client_execute(
    HttpClient *client,
    HttpRequest *request,
    HttpResponse **response
);

void http_client_destroy(
    HttpClient *client
);


/* Request */

HttpRequest *http_request_create(
    HttpMethod method,
    const char *url
);

int http_request_set_header(
    HttpRequest *request,
    const char *name,
    const char *value
);

int http_request_set_body(
    HttpRequest *request,
    const void *data,
    size_t size
);

int http_request_set_timeout(
    HttpRequest *request,
    uint32_t timeout_ms
);

void http_request_destroy(
    HttpRequest *request
);


/* Response */

int http_response_status_code(
    const HttpResponse *response
);

const char *http_response_header(
    const HttpResponse *response,
    const char *name
);

const void *http_response_body(
    const HttpResponse *response
);

size_t http_response_body_size(
    const HttpResponse *response
);

void http_response_destroy(
    HttpResponse *response
);

#ifdef __cplusplus
}
#endif

#endif

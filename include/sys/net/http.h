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

typedef struct HttpHeader {
    const char *name;
    const char *value;
} HttpHeader;


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


/* Helper Functions */

/**
 * @brief Executes an HTTP request and returns the response body as a null-terminated string.
 *
 * Sends an HTTP request using the specified URL, HTTP method, headers, and optional
 * request body. The response body is returned as a dynamically allocated,
 * null-terminated string.
 *
 * The returned string must be released by the caller using
 * @ref http_call_free_response_text() when it is no longer needed.
 *
 * @param[in] url
 *     Null-terminated string containing the URL of the HTTP request.
 *
 * @param[in] method
 *     HTTP method to use for the request (e.g. GET, POST, PUT, DELETE).
 *
 * @param[in] headers
 *     Pointer to an array of @ref HttpHeader structures containing the HTTP headers.
 *     May be NULL if no headers are required.
 *
 * @param[in] header_count
 *     Number of elements in the @p headers array.
 *
 * @param[in] body
 *     Null-terminated string containing the request body.
 *     May be NULL if the request does not require a body.
 *
 * @param[out] out_status_code
 *     Pointer to an integer where the HTTP status code returned by the server
 *     will be stored (e.g. 200, 404, 500). May be NULL if the status code
 *     is not required.
 *
 * @return
 *     A dynamically allocated, null-terminated string containing the response
 *     body on success.
 *
 *     Returns NULL if the request could not be created, executed, or if the
 *     response could not be obtained.
 *
 * @note
 *     HTTP error status codes such as 400, 404, or 500 are valid server
 *     responses. The HTTP status code is returned separately through
 *     @p out_status_code.
 *
 * @warning
 *     The returned string must be freed using
 *     @ref http_call_free_response_text().
 */
const char* http_call_get_response_text(
    const char* url,
    HttpMethod method,
    const HttpHeader* headers,
    size_t header_count,
    const char* body,
    int* out_status_code
);

/**
 * @brief Frees a response string returned by http_call_get_response_text().
 *
 * Releases the memory allocated for the response body by
 * @ref http_call_get_response_text().
 *
 * @param[in] responseText
 *     Pointer to the response string returned by
 *     @ref http_call_get_response_text().
 *     May be NULL, in which case no action is performed.
 *
 * @note
 *     Do not use @p responseText after calling this function.
 */
void http_call_free_response_text(
    const char* responseText
);


#ifdef __cplusplus
}
#endif

#endif

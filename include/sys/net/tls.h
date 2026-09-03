#ifndef TLS_H
#define TLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct TlsClient TlsClient;
typedef struct TcpClient TcpClient;


/* ============================================================
 * Client
 * ============================================================ */

TlsClient *tls_client_create(
    void
);

int tls_client_set_hostname(
    TlsClient *client,
    const char *hostname
);

int tls_client_set_ca_file(
    TlsClient *client,
    const char *path
);

int tls_client_set_ca_pem(
    TlsClient *client,
    const void *data,
    size_t size
);

int tls_client_connect(
    TlsClient *client,
    TcpClient *tcp
);

int tls_client_send(
    TlsClient *client,
    const void *data,
    size_t size
);

int tls_client_receive(
    TlsClient *client,
    void *buffer,
    size_t size
);

int tls_client_close(
    TlsClient *client
);

void tls_client_destroy(
    TlsClient *client
);

#ifdef __cplusplus
}
#endif

#endif

#ifndef TCP_H
#define TCP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct TcpClient TcpClient;
typedef struct TcpServer TcpServer;
typedef struct TcpConnection TcpConnection;

/* Client */

TcpClient *tcp_client_create(void);

int tcp_client_connect(
    TcpClient *client,
    const char *host,
    uint16_t port
);

int tcp_client_send(
    TcpClient *client,
    const void *data,
    size_t size
);

int tcp_client_receive(
    TcpClient *client,
    void *buffer,
    size_t size
);

int tcp_client_close(TcpClient *client);

void tcp_client_destroy(TcpClient *client);


/* Server */

TcpServer *tcp_server_create(void);

int tcp_server_bind(
    TcpServer *server,
    const char *host,
    uint16_t port
);

int tcp_server_listen(
    TcpServer *server,
    int backlog
);

TcpConnection *tcp_server_accept(
    TcpServer *server
);


/* Connection */

int tcp_connection_send(
    TcpConnection *connection,
    const void *data,
    size_t size
);

int tcp_connection_receive(
    TcpConnection *connection,
    void *buffer,
    size_t size
);

int tcp_connection_close(
    TcpConnection *connection
);

void tcp_connection_destroy(
    TcpConnection *connection
);

int tcp_server_close(
    TcpServer *server
);

void tcp_server_destroy(
    TcpServer *server
);

#ifdef __cplusplus
}
#endif

#endif

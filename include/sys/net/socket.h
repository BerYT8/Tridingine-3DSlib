#ifndef SOCKET_H
#define SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct Socket Socket;

typedef enum SocketType {
    SOCKET_STREAM,
    SOCKET_DATAGRAM
} SocketType;

typedef enum SocketFamily {
    SOCKET_IPV4,
    SOCKET_IPV6
} SocketFamily;

Socket *socket_create(SocketFamily family, SocketType type);

int socket_bind(Socket *socket, const char *address, uint16_t port);

int socket_listen(Socket *socket, int backlog);

Socket *socket_accept(Socket *socket);

int socket_connect(Socket *socket, const char *address, uint16_t port);

int socket_send(Socket *socket, const void *data, size_t size);

int socket_receive(Socket *socket, void *buffer, size_t size);

int socket_close(Socket *socket);

void socket_destroy(Socket *socket);

#ifdef __cplusplus
}
#endif

#endif

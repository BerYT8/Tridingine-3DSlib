#include <sys/net/tcp.h>
#include <sys/net/socket.h>

#include <cstddef>
#include <cstdint>


struct TcpClient
{
    Socket *socket;
};


struct TcpServer
{
    Socket *socket;
};


struct TcpConnection
{
    Socket *socket;
};


/* ============================================================
 * Client
 * ============================================================ */

TcpClient *tcp_client_create(void)
{
    TcpClient *client = new TcpClient;

    client->socket = socket_create(
        SOCKET_IPV4,
        SOCKET_STREAM
    );

    if (!client->socket)
    {
        delete client;
        return nullptr;
    }

    return client;
}


int tcp_client_connect(
    TcpClient *client,
    const char *host,
    uint16_t port)
{
    if (!client || !client->socket)
        return -1;

    return socket_connect(
        client->socket,
        host,
        port
    );
}


int tcp_client_send(
    TcpClient *client,
    const void *data,
    size_t size)
{
    if (!client || !client->socket)
        return -1;

    return socket_send(
        client->socket,
        data,
        size
    );
}


int tcp_client_receive(
    TcpClient *client,
    void *buffer,
    size_t size)
{
    if (!client || !client->socket)
        return -1;

    return socket_receive(
        client->socket,
        buffer,
        size
    );
}


int tcp_client_close(
    TcpClient *client)
{
    if (!client || !client->socket)
        return -1;

    return socket_close(
        client->socket
    );
}


void tcp_client_destroy(
    TcpClient *client)
{
    if (!client)
        return;

    if (client->socket)
        socket_destroy(client->socket);

    delete client;
}


/* ============================================================
 * Server
 * ============================================================ */

TcpServer *tcp_server_create(void)
{
    TcpServer *server = new TcpServer;

    server->socket = socket_create(
        SOCKET_IPV4,
        SOCKET_STREAM
    );

    if (!server->socket)
    {
        delete server;
        return nullptr;
    }

    return server;
}


int tcp_server_bind(
    TcpServer *server,
    const char *host,
    uint16_t port)
{
    if (!server || !server->socket)
        return -1;

    return socket_bind(
        server->socket,
        host,
        port
    );
}


int tcp_server_listen(
    TcpServer *server,
    int backlog)
{
    if (!server || !server->socket)
        return -1;

    return socket_listen(
        server->socket,
        backlog
    );
}


TcpConnection *tcp_server_accept(
    TcpServer *server)
{
    if (!server || !server->socket)
        return nullptr;

    Socket *socket =
        socket_accept(server->socket);

    if (!socket)
        return nullptr;

    TcpConnection *connection =
        new TcpConnection;

    connection->socket = socket;

    return connection;
}


/* ============================================================
 * Connection
 * ============================================================ */

int tcp_connection_send(
    TcpConnection *connection,
    const void *data,
    size_t size)
{
    if (!connection || !connection->socket)
        return -1;

    return socket_send(
        connection->socket,
        data,
        size
    );
}


int tcp_connection_receive(
    TcpConnection *connection,
    void *buffer,
    size_t size)
{
    if (!connection || !connection->socket)
        return -1;

    return socket_receive(
        connection->socket,
        buffer,
        size
    );
}


int tcp_connection_close(
    TcpConnection *connection)
{
    if (!connection || !connection->socket)
        return -1;

    return socket_close(
        connection->socket
    );
}


void tcp_connection_destroy(
    TcpConnection *connection)
{
    if (!connection)
        return;

    if (connection->socket)
        socket_destroy(connection->socket);

    delete connection;
}


/* ============================================================
 * Server cleanup
 * ============================================================ */

int tcp_server_close(
    TcpServer *server)
{
    if (!server || !server->socket)
        return -1;

    return socket_close(
        server->socket
    );
}


void tcp_server_destroy(
    TcpServer *server)
{
    if (!server)
        return;

    if (server->socket)
        socket_destroy(server->socket);

    delete server;
}

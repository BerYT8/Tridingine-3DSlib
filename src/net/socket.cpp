#include <sys/net/socket.h>
#include <sys/net/net.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

#if defined(PLATFORM_PC)

    #if defined(_WIN32)

        #include <winsock2.h>
        #include <ws2tcpip.h>
        #include <cerrno>

    #elif defined(__linux__) || defined(__APPLE__)

        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
        #include <unistd.h>
        #include <netdb.h>

    #endif

#elif defined(PLATFORM_3DS)

    #include <3ds.h>
    #include <malloc.h>

    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>

    static u32* socBuffer = nullptr;

#endif


struct Socket
{
    int handle;
};

static bool net_system_initialized = false;

bool net_system_init()
{
    if (net_system_initialized)
        return true;

#if defined(PLATFORM_3DS)

    constexpr u32 SOC_ALIGN = 0x1000;
    constexpr u32 SOC_BUFFER_SIZE = 0x100000;

    socBuffer = static_cast<u32*>(
        memalign(SOC_ALIGN, SOC_BUFFER_SIZE)
    );

    if (socBuffer == nullptr)
        return false;

    Result rc = socInit(
        socBuffer,
        SOC_BUFFER_SIZE
    );

    if (R_FAILED(rc))
    {
        free(socBuffer);
        socBuffer = nullptr;

        return false;
    }

#endif

    net_system_initialized = true;
    return true;
}

void net_system_shutdown()
{
    if (!net_system_initialized)
        return;

#if defined(PLATFORM_3DS)

    socExit();

    if (socBuffer)
    {
        free(socBuffer);
        socBuffer = nullptr;
    }

#endif

    net_system_initialized = false;
}

Socket *socket_create(SocketFamily family, SocketType type)
{
    if(!net_system_initialized)
        return nullptr;

    int af = AF_INET;
    int sockType = SOCK_STREAM;

    if (family == SOCKET_IPV6)
        af = AF_INET6;

    if (type == SOCKET_DATAGRAM)
        sockType = SOCK_DGRAM;

    int handle = ::socket(af, sockType, 0);

    if (handle < 0)
        return nullptr;

    Socket *socket = new Socket;
    socket->handle = handle;

    return socket;
}


int socket_bind(
    Socket *socket,
    const char *address,
    uint16_t port)
{
    if(!net_system_initialized)
        return -1;

    if (!socket)
        return -1;

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (address == nullptr)
    {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        if (inet_pton(AF_INET, address, &addr.sin_addr) <= 0)
            return -1;
    }

    return ::bind(
        socket->handle,
        reinterpret_cast<sockaddr *>(&addr),
        sizeof(addr)
    );
}


int socket_listen(
    Socket *socket,
    int backlog)
{
    if(!net_system_initialized)
        return -1;

    if (!socket)
        return -1;

    return ::listen(
        socket->handle,
        backlog
    );
}


Socket *socket_accept(
    Socket *socket)
{
    if(!net_system_initialized)
        return nullptr;

    if (!socket)
        return nullptr;

    sockaddr_in clientAddress{};

#if defined(_WIN32)

    int addressLength = sizeof(clientAddress);

#else

    socklen_t addressLength = sizeof(clientAddress);

#endif

    int clientHandle = ::accept(
        socket->handle,
        reinterpret_cast<sockaddr *>(&clientAddress),
        &addressLength
    );

#if defined(_WIN32)

    if (clientHandle == INVALID_SOCKET)
        return nullptr;

#else

    if (clientHandle < 0)
        return nullptr;

#endif

    Socket *client = new Socket;
    client->handle = clientHandle;

    return client;
}


int socket_connect(
    Socket *socket,
    const char *address,
    uint16_t port)
{
    if(!net_system_initialized)
        return -1;

    if (!socket || !address)
        return -1;

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    /*
     * First try interpreting address as an IP.
     */
    if (inet_pton(AF_INET, address, &addr.sin_addr) == 1)
    {
        return ::connect(
            socket->handle,
            reinterpret_cast<sockaddr *>(&addr),
            sizeof(addr)
        );
    }

    /*
     * Otherwise resolve hostname.
     */
    addrinfo hints{};
    addrinfo *result = nullptr;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int error = getaddrinfo(
        address,
        nullptr,
        &hints,
        &result
    );

    if (error != 0 || result == nullptr)
        return -1;

    sockaddr_in *resolved =
        reinterpret_cast<sockaddr_in *>(result->ai_addr);

    addr.sin_addr = resolved->sin_addr;

    int resultCode = ::connect(
        socket->handle,
        reinterpret_cast<sockaddr *>(&addr),
        sizeof(addr)
    );

    freeaddrinfo(result);

    return resultCode;
}


int socket_send(
    Socket *socket,
    const void *data,
    size_t size)
{
    if(!net_system_initialized)
        return -1;

    if (!socket || !data)
        return -1;

    return static_cast<int>(
        ::send(
            socket->handle,
            static_cast<const char *>(data),
            static_cast<int>(size),
            0
        )
    );
}


int socket_receive(
    Socket *socket,
    void *buffer,
    size_t size)
{
    if(!net_system_initialized)
        return -1;

    if (!socket || !buffer)
        return -1;

    return static_cast<int>(
        ::recv(
            socket->handle,
            static_cast<char *>(buffer),
            static_cast<int>(size),
            0
        )
    );
}


int socket_close(
    Socket *socket)
{
    if(!net_system_initialized)
        return -1;

    if (!socket)
        return -1;

#if defined(_WIN32)

    return ::closesocket(socket->handle);

#else

    return ::close(socket->handle);

#endif
}


void socket_destroy(
    Socket *socket)
{
    if(!net_system_initialized)
        return;

    if (!socket)
        return;

    socket_close(socket);

    delete socket;
}
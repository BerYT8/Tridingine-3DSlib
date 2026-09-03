#include <sys/net/http.h>
#include <sys/net/tls.h>
#include <sys/net/tcp.h>

#include "../romfs_path.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdlib>

#include <string>
#include <vector>


/* ============================================================
 * HTTP STRUCTURES
 * ============================================================ */

struct HttpRequest
{
    HttpMethod method;

    std::string url;

    std::vector<
        std::pair<
            std::string,
            std::string
        >
    > headers;

    std::vector<uint8_t> body;

    uint32_t timeout_ms;
};


struct HttpResponse
{
    int status_code;

    std::vector<
        std::pair<
            std::string,
            std::string
        >
    > headers;

    std::vector<uint8_t> body;
};


struct HttpClient
{
};


/* ============================================================
 * HELPERS
 * ============================================================ */

static const char *method_to_string(
    HttpMethod method)
{
    switch (method)
    {
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

        case HTTP_HEAD:
            return "HEAD";

        case HTTP_OPTIONS:
            return "OPTIONS";
    }

    return "GET";
}


static bool starts_with(
    const std::string &string,
    const char *prefix)
{
    return string.rfind(
        prefix,
        0
    ) == 0;
}


static std::string trim(
    const std::string &string)
{
    size_t begin = 0;
    size_t end = string.size();


    while (begin < end &&
           (string[begin] == ' ' ||
            string[begin] == '\t' ||
            string[begin] == '\r' ||
            string[begin] == '\n'))
    {
        ++begin;
    }


    while (end > begin &&
           (string[end - 1] == ' ' ||
            string[end - 1] == '\t' ||
            string[end - 1] == '\r' ||
            string[end - 1] == '\n'))
    {
        --end;
    }


    return string.substr(
        begin,
        end - begin
    );
}


static std::string to_lower(
    const std::string &string)
{
    std::string result =
        string;


    for (char &c : result)
    {
        if (c >= 'A' &&
            c <= 'Z')
        {
            c =
                static_cast<char>(
                    c + ('a' - 'A')
                );
        }
    }


    return result;
}


/* ============================================================
 * URL PARSER
 * ============================================================ */

static bool parse_url(
    const char *url,
    bool &https,
    std::string &host,
    uint16_t &port,
    std::string &path)
{
    if (!url)
        return false;


    std::string value(url);


    if (starts_with(
            value,
            "http://"))
    {
        https = false;
        port = 80;

        value.erase(
            0,
            7
        );
    }
    else if (starts_with(
                value,
                "https://"))
    {
        https = true;
        port = 443;

        value.erase(
            0,
            8
        );
    }
    else
    {
        return false;
    }


    /*
     * Host/path.
     */

    size_t pathPosition =
        value.find('/');


    if (pathPosition ==
        std::string::npos)
    {
        host =
            value;

        path =
            "/";
    }
    else
    {
        host =
            value.substr(
                0,
                pathPosition
            );

        path =
            value.substr(
                pathPosition
            );
    }


    /*
     * host:port
     */

    size_t portPosition =
        host.rfind(':');


    if (portPosition !=
        std::string::npos)
    {
        /*
         * Evitamos interpretar mal
         * un IPv6 sin soporte explícito.
         */
        if (host.find(':') !=
            portPosition)
        {
            return false;
        }


        std::string portString =
            host.substr(
                portPosition + 1
            );


        host.erase(
            portPosition
        );


        if (portString.empty())
            return false;


        int parsedPort = 0;


        for (char c :
             portString)
        {
            if (c < '0' ||
                c > '9')
            {
                return false;
            }


            parsedPort =
                parsedPort * 10 +
                (c - '0');


            if (parsedPort >
                65535)
            {
                return false;
            }
        }


        port =
            static_cast<uint16_t>(
                parsedPort
            );
    }


    return !host.empty();
}


/* ============================================================
 * FIND HEADER END
 * ============================================================ */

static bool find_header_end(
    const std::vector<uint8_t> &data,
    size_t &position)
{
    if (data.size() < 4)
        return false;


    for (size_t i = 0;
         i + 3 < data.size();
         ++i)
    {
        if (data[i]     == '\r' &&
            data[i + 1] == '\n' &&
            data[i + 2] == '\r' &&
            data[i + 3] == '\n')
        {
            position =
                i + 4;

            return true;
        }
    }


    return false;
}


/* ============================================================
 * CHUNKED
 * ============================================================ */

static bool decode_chunked_body(
    const std::vector<uint8_t> &input,
    std::vector<uint8_t> &output)
{
    size_t position = 0;


    while (position < input.size())
    {
        /*
         * Buscar CRLF del tamaño.
         */

        size_t lineEnd =
            position;


        while (lineEnd + 1 <
               input.size())
        {
            if (input[lineEnd] ==
                    '\r' &&
                input[lineEnd + 1] ==
                    '\n')
            {
                break;
            }


            ++lineEnd;
        }


        if (lineEnd + 1 >=
            input.size())
        {
            return false;
        }


        std::string sizeString(
            reinterpret_cast<
                const char *
            >(
                input.data() +
                position
            ),
            lineEnd -
            position
        );


        /*
         * Chunk extensions.
         */

        size_t extensionPosition =
            sizeString.find(';');


        if (extensionPosition !=
            std::string::npos)
        {
            sizeString.erase(
                extensionPosition
            );
        }


        sizeString =
            trim(
                sizeString
            );


        if (sizeString.empty())
            return false;


        size_t chunkSize = 0;


        for (char c :
             sizeString)
        {
            uint8_t value;


            if (c >= '0' &&
                c <= '9')
            {
                value =
                    static_cast<uint8_t>(
                        c - '0'
                    );
            }
            else if (c >= 'a' &&
                     c <= 'f')
            {
                value =
                    static_cast<uint8_t>(
                        c - 'a' + 10
                    );
            }
            else if (c >= 'A' &&
                     c <= 'F')
            {
                value =
                    static_cast<uint8_t>(
                        c - 'A' + 10
                    );
            }
            else
            {
                return false;
            }


            /*
             * Protección contra overflow.
             */

            if (chunkSize >
                (static_cast<size_t>(-1) -
                 value) / 16)
            {
                return false;
            }


            chunkSize =
                chunkSize * 16 +
                value;
        }


        position =
            lineEnd + 2;


        /*
         * Fin.
         */

        if (chunkSize == 0)
        {
            /*
             * No necesitamos interpretar
             * trailers para devolver el body.
             */
            return true;
        }


        if (chunkSize >
            input.size() - position)
        {
            return false;
        }


        output.insert(
            output.end(),
            input.begin() + position,
            input.begin() +
                position +
                chunkSize
        );


        position +=
            chunkSize;


        /*
         * CRLF después del chunk.
         */

        if (position + 1 >=
            input.size())
        {
            return false;
        }


        if (input[position] != '\r' ||
            input[position + 1] != '\n')
        {
            return false;
        }


        position += 2;
    }


    return false;
}


/* ============================================================
 * RESPONSE PARSER
 * ============================================================ */

static bool parse_response(
    const std::vector<uint8_t> &data,
    HttpResponse *response)
{
    if (!response)
        return false;


    size_t headerEnd = 0;


    if (!find_header_end(
            data,
            headerEnd))
    {
        return false;
    }


    std::string headers(
        reinterpret_cast<
            const char *
        >(
            data.data()
        ),
        headerEnd
    );


    /*
     * Status line.
     */

    size_t firstLineEnd =
        headers.find(
            "\r\n"
        );


    if (firstLineEnd ==
        std::string::npos)
    {
        return false;
    }


    std::string statusLine =
        headers.substr(
            0,
            firstLineEnd
        );


    size_t firstSpace =
        statusLine.find(' ');


    if (firstSpace ==
        std::string::npos)
    {
        return false;
    }


    size_t secondSpace =
        statusLine.find(
            ' ',
            firstSpace + 1
        );


    std::string statusCodeString;


    if (secondSpace ==
        std::string::npos)
    {
        statusCodeString =
            statusLine.substr(
                firstSpace + 1
            );
    }
    else
    {
        statusCodeString =
            statusLine.substr(
                firstSpace + 1,
                secondSpace -
                firstSpace -
                1
            );
    }


    if (statusCodeString.empty())
        return false;


    int statusCode = 0;


    for (char c :
         statusCodeString)
    {
        if (c < '0' ||
            c > '9')
        {
            return false;
        }


        statusCode =
            statusCode * 10 +
            (c - '0');
    }


    response->status_code =
        statusCode;


    /*
     * Headers.
     */

    size_t position =
        firstLineEnd + 2;


    while (position <
           headerEnd - 2)
    {
        size_t lineEnd =
            headers.find(
                "\r\n",
                position
            );


        if (lineEnd ==
            std::string::npos)
        {
            break;
        }


        if (lineEnd ==
            position)
        {
            break;
        }


        std::string line =
            headers.substr(
                position,
                lineEnd -
                position
            );


        size_t colon =
            line.find(':');


        if (colon !=
            std::string::npos)
        {
            std::string name =
                trim(
                    line.substr(
                        0,
                        colon
                    )
                );


            std::string value =
                trim(
                    line.substr(
                        colon + 1
                    )
                );


            response->headers.emplace_back(
                name,
                value
            );
        }


        position =
            lineEnd + 2;
    }


    /*
     * Body.
     */

    if (headerEnd <
        data.size())
    {
        std::vector<uint8_t> bodyData(
            data.begin() + headerEnd,
            data.end()
        );


        bool chunked =
            false;


        for (const auto &header :
             response->headers)
        {
            if (to_lower(
                    header.first
                ) ==
                    "transfer-encoding" &&
                to_lower(
                    header.second
                ) ==
                    "chunked")
            {
                chunked =
                    true;

                break;
            }
        }


        if (chunked)
        {
            if (!decode_chunked_body(
                    bodyData,
                    response->body))
            {
                return false;
            }
        }
        else
        {
            response->body =
                std::move(
                    bodyData
                );
        }
    }


    return true;
}


/* ============================================================
 * CLIENT
 * ============================================================ */

HttpClient *http_client_create(void)
{
    return new HttpClient;
}


int http_client_request(
    HttpClient *client,
    HttpMethod method,
    const char *url,
    HttpRequest **request)
{
    if (!client ||
        !url ||
        !request)
    {
        return -1;
    }


    HttpRequest *result =
        http_request_create(
            method,
            url
        );


    if (!result)
        return -1;


    *request =
        result;


    return 0;
}


/* ============================================================
 * EXECUTE
 * ============================================================ */

int http_client_execute(
    HttpClient *client,
    HttpRequest *request,
    HttpResponse **response)
{
    if (!client ||
        !request ||
        !response)
    {
        return -1;
    }


    *response =
        nullptr;


    bool https =
        false;


    std::string host;
    std::string path;


    uint16_t port =
        0;


    /*
     * URL.
     */

    if (!parse_url(
            request->url.c_str(),
            https,
            host,
            port,
            path))
    {
        return -1;
    }


    /* ========================================================
     * TCP
     * ======================================================== */

    TcpClient *tcp =
        tcp_client_create();


    if (!tcp)
        return -1;


    int result =
        tcp_client_connect(
            tcp,
            host.c_str(),
            port
        );


    if (result != 0)
    {
        tcp_client_destroy(
            tcp
        );

        return -1;
    }


    /* ========================================================
     * TLS
     * ======================================================== */

    TlsClient *tls =
        nullptr;


    if (https)
    {
        tls =
            tls_client_create();


        if (!tls)
        {
            tcp_client_close(
                tcp
            );

            tcp_client_destroy(
                tcp
            );

            return -1;
        }


        /*
         * Hostname.
         */

        result =
            tls_client_set_hostname(
                tls,
                host.c_str()
            );


        if (result != 0)
        {
            tls_client_destroy(
                tls
            );

            tcp_client_close(
                tcp
            );

            tcp_client_destroy(
                tcp
            );

            return -1;
        }

#ifndef TRIDINGINE_CA_FILE
#define TRIDINGINE_CA_FILE "certs/ca-bundle.crt"
#endif

        result =
            tls_client_set_ca_file(
                tls,
                getRomfsPath(TRIDINGINE_CA_FILE)
            );


        if (result != 0)
        {
            std::printf(
                "HTTP 3DS: unable to load CA bundle: %s\n",
                TRIDINGINE_CA_FILE
            );


            tls_client_destroy(
                tls
            );


            tcp_client_close(
                tcp
            );

            tcp_client_destroy(
                tcp
            );

            return -1;
        }


        /*
         * TLS handshake sobre EL MISMO TCP.
         */

        result =
            tls_client_connect(
                tls,
                tcp
            );


        if (result != 0)
        {
            tls_client_destroy(
                tls
            );


            tcp_client_close(
                tcp
            );

            tcp_client_destroy(
                tcp
            );

            return -1;
        }
    }


    /* ========================================================
     * BUILD HTTP REQUEST
     * ======================================================== */

    std::string message;


    message +=
        method_to_string(
            request->method
        );


    message +=
        " ";


    message +=
        path;


    message +=
        " HTTP/1.1\r\n";


    bool hasHost =
        false;


    bool hasContentLength =
        false;


    bool hasConnection =
        false;


    for (const auto &header :
         request->headers)
    {
        std::string name =
            to_lower(
                header.first
            );


        if (name == "host")
            hasHost = true;


        if (name == "content-length")
            hasContentLength = true;


        if (name == "connection")
            hasConnection = true;


        message +=
            header.first;


        message +=
            ": ";


        message +=
            header.second;


        message +=
            "\r\n";
    }


    /*
     * Host.
     */

    if (!hasHost)
    {
        message +=
            "Host: ";


        message +=
            host;


        /*
         * Si se usa un puerto no estándar,
         * debe formar parte del Host.
         */

        if ((https && port != 443) ||
            (!https && port != 80))
        {
            message +=
                ":";


            message +=
                std::to_string(
                    port
                );
        }


        message +=
            "\r\n";
    }


    /*
     * Content-Length.
     */

    if (!hasContentLength &&
        !request->body.empty())
    {
        message +=
            "Content-Length: ";


        message +=
            std::to_string(
                request->body.size()
            );


        message +=
            "\r\n";
    }


    /*
     * Connection.
     */

    if (!hasConnection)
    {
        message +=
            "Connection: close\r\n";
    }


    /*
     * Fin headers.
     */

    message +=
        "\r\n";


    /* ========================================================
     * SEND HEADERS
     * ======================================================== */

    if (https)
    {
        result =
            tls_client_send(
                tls,
                message.data(),
                message.size()
            );
    }
    else
    {
        result =
            tcp_client_send(
                tcp,
                message.data(),
                message.size()
            );
    }


    if (result < 0)
    {
        if (tls)
        {
            tls_client_close(
                tls
            );

            tls_client_destroy(
                tls
            );

            tls =
                nullptr;
        }


        tcp_client_close(
            tcp
        );


        tcp_client_destroy(
            tcp
        );


        return -1;
    }


    /* ========================================================
     * SEND BODY
     * ======================================================== */

    if (!request->body.empty())
    {
        if (https)
        {
            result =
                tls_client_send(
                    tls,
                    request->body.data(),
                    request->body.size()
                );
        }
        else
        {
            result =
                tcp_client_send(
                    tcp,
                    request->body.data(),
                    request->body.size()
                );
        }


        if (result < 0)
        {
            if (tls)
            {
                tls_client_close(
                    tls
                );

                tls_client_destroy(
                    tls
                );

                tls =
                    nullptr;
            }


            tcp_client_close(
                tcp
            );


            tcp_client_destroy(
                tcp
            );


            return -1;
        }
    }


    /* ========================================================
     * RECEIVE RESPONSE
     * ======================================================== */

    std::vector<uint8_t> data;


    uint8_t buffer[4096];


    while (true)
    {
        int received;


        if (https)
        {
            received =
                tls_client_receive(
                    tls,
                    buffer,
                    sizeof(buffer)
                );
        }
        else
        {
            received =
                tcp_client_receive(
                    tcp,
                    buffer,
                    sizeof(buffer)
                );
        }


        if (received < 0)
        {
            /*
             * Error real.
             */
            if (tls)
            {
                tls_client_close(
                    tls
                );

                tls_client_destroy(
                    tls
                );

                tls =
                    nullptr;
            }


            tcp_client_close(
                tcp
            );


            tcp_client_destroy(
                tcp
            );


            return -1;
        }


        if (received == 0)
        {
            /*
             * EOF / conexión cerrada.
             */
            break;
        }


        data.insert(
            data.end(),
            buffer,
            buffer + received
        );
    }


    /* ========================================================
     * CLOSE TLS
     * ======================================================== */

    if (tls)
    {
        tls_client_close(
            tls
        );


        tls_client_destroy(
            tls
        );


        tls =
            nullptr;
    }


    /* ========================================================
     * CLOSE TCP
     * ======================================================== */

    if (tcp)
    {
        tcp_client_close(
            tcp
        );


        tcp_client_destroy(
            tcp
        );


        tcp =
            nullptr;
    }


    /* ========================================================
     * PARSE RESPONSE
     * ======================================================== */

    HttpResponse *resultResponse =
        new HttpResponse;


    if (!resultResponse)
        return -1;


    if (!parse_response(
            data,
            resultResponse))
    {
        delete resultResponse;

        return -1;
    }


    *response =
        resultResponse;


    return 0;
}


/* ============================================================
 * CLIENT DESTROY
 * ============================================================ */

void http_client_destroy(
    HttpClient *client)
{
    delete client;
}


/* ============================================================
 * REQUEST
 * ============================================================ */

HttpRequest *http_request_create(
    HttpMethod method,
    const char *url)
{
    if (!url)
        return nullptr;


    HttpRequest *request =
        new HttpRequest;


    request->method =
        method;


    request->url =
        url;


    request->timeout_ms =
        0;


    return request;
}


int http_request_set_header(
    HttpRequest *request,
    const char *name,
    const char *value)
{
    if (!request ||
        !name ||
        !value)
    {
        return -1;
    }


    request->headers.emplace_back(
        name,
        value
    );


    return 0;
}


int http_request_set_body(
    HttpRequest *request,
    const void *data,
    size_t size)
{
    if (!request)
        return -1;


    if (!data &&
        size != 0)
    {
        return -1;
    }


    const uint8_t *bytes =
        static_cast<
            const uint8_t *
        >(
            data
        );


    request->body.assign(
        bytes,
        bytes + size
    );


    return 0;
}


int http_request_set_timeout(
    HttpRequest *request,
    uint32_t timeout_ms)
{
    if (!request)
        return -1;


    request->timeout_ms =
        timeout_ms;


    return 0;
}


void http_request_destroy(
    HttpRequest *request)
{
    delete request;
}


/* ============================================================
 * RESPONSE
 * ============================================================ */

int http_response_status_code(
    const HttpResponse *response)
{
    if (!response)
        return -1;


    return response->status_code;
}


const char *http_response_header(
    const HttpResponse *response,
    const char *name)
{
    if (!response ||
        !name)
    {
        return nullptr;
    }


    std::string requested =
        to_lower(
            name
        );


    for (const auto &header :
         response->headers)
    {
        if (to_lower(
                header.first
            ) ==
            requested)
        {
            return header.second.c_str();
        }
    }


    return nullptr;
}


const void *http_response_body(
    const HttpResponse *response)
{
    if (!response ||
        response->body.empty())
    {
        return nullptr;
    }


    return response->body.data();
}


size_t http_response_body_size(
    const HttpResponse *response)
{
    if (!response)
        return 0;


    return response->body.size();
}


void http_response_destroy(
    HttpResponse *response)
{
    delete response;
}


/* ============================================================
 * HELPER GET/REQUEST
 * ============================================================ */

const char *http_call_get_response_text(
    const char *url,
    HttpMethod method,
    const HttpHeader *headers,
    size_t header_count,
    const char *body,
    int *out_status_code)
{
    std::string responseText;

    if (out_status_code)
    {
        *out_status_code = -1;
    }

    /*
     * Client.
     */

    HttpClient *client =
        http_client_create();


    if (!client)
        return nullptr;


    /*
     * Request.
     */

    HttpRequest *request =
        http_request_create(
            method,
            url
        );


    if (!request)
    {
        http_client_destroy(
            client
        );

        return nullptr;
    }


    /*
     * Body.
     */

    if (body)
    {
        if (http_request_set_body(
                request,
                body,
                std::strlen(body)
            ) != 0)
        {
            http_request_destroy(
                request
            );

            http_client_destroy(
                client
            );

            return nullptr;
        }
    }


    /*
     * Headers.
     */

    if (headers)
    {
        for (size_t i = 0;
             i < header_count;
             ++i)
        {
            const HttpHeader *header =
                &headers[i];


            if (!header->name ||
                !header->value)
            {
                continue;
            }


            http_request_set_header(
                request,
                header->name,
                header->value
            );
        }
    }


    /*
     * Execute.
     */

    HttpResponse *response =
        nullptr;


    int result =
        http_client_execute(
            client,
            request,
            &response
        );


    if (result == 0 &&
        response)
    {
        /*
         * Body.
         */

        const void *resBody =
            http_response_body(
                response
            );


        size_t resBodySize =
            http_response_body_size(
                response
            );


        if (resBody &&
            resBodySize > 0)
        {
            responseText.assign(
                static_cast<
                    const char *
                >(
                    resBody
                ),
                resBodySize
            );
        }


        /*
         * Status.
         */

        int statusCode =
            http_response_status_code(
                response
            );


        if (out_status_code)
        {
            *out_status_code =
                statusCode;
        }


        http_response_destroy(
            response
        );
    }


    /*
     * Destroy.
     */

    http_request_destroy(
        request
    );


    http_client_destroy(
        client
    );


    /*
     * C string.
     */

    char *resultText =
        static_cast<char *>(
            std::malloc(
                responseText.size() + 1
            )
        );


    if (!resultText)
        return nullptr;


    std::memcpy(
        resultText,
        responseText.data(),
        responseText.size()
    );


    resultText[
        responseText.size()
    ] =
        '\0';


    return resultText;
}


/* ============================================================
 * FREE RESPONSE TEXT
 * ============================================================ */

void http_call_free_response_text(
    const char *responseText)
{
    if (responseText)
    {
        std::free(
            const_cast<char *>(
                responseText
            )
        );
    }
}

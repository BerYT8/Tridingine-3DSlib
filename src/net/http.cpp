#include <sys/net/http.h>
#include <sys/net/tcp.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <string>
#include <vector>


struct HttpRequest
{
    HttpMethod method;
    std::string url;

    std::vector<std::pair<std::string, std::string>> headers;

    std::vector<uint8_t> body;

    uint32_t timeout_ms;
};


struct HttpResponse
{
    int status_code;

    std::vector<std::pair<std::string, std::string>> headers;

    std::vector<uint8_t> body;
};


struct HttpClient
{
};


/* ============================================================
 * Helpers
 * ============================================================ */

static const char *method_to_string(HttpMethod method)
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
    return string.rfind(prefix, 0) == 0;
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
    std::string result = string;

    for (char &c : result)
    {
        if (c >= 'A' && c <= 'Z')
            c = static_cast<char>(c + ('a' - 'A'));
    }

    return result;
}


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

    if (starts_with(value, "http://"))
    {
        https = false;
        port = 80;

        value.erase(0, 7);
    }
    else if (starts_with(value, "https://"))
    {
        https = true;
        port = 443;

        value.erase(0, 8);
    }
    else
    {
        return false;
    }

    size_t pathPosition = value.find('/');

    if (pathPosition == std::string::npos)
    {
        host = value;
        path = "/";
    }
    else
    {
        host = value.substr(
            0,
            pathPosition
        );

        path = value.substr(
            pathPosition
        );
    }

    /*
     * Support host:port.
     */
    size_t portPosition = host.rfind(':');

    if (portPosition != std::string::npos)
    {
        std::string portString =
            host.substr(portPosition + 1);

        host.erase(portPosition);

        int parsedPort = 0;

        for (char c : portString)
        {
            if (c < '0' || c > '9')
                return false;

            parsedPort =
                parsedPort * 10 +
                (c - '0');

            if (parsedPort > 65535)
                return false;
        }

        port = static_cast<uint16_t>(
            parsedPort
        );
    }

    return !host.empty();
}


static bool find_header_end(
    const std::vector<uint8_t> &data,
    size_t &position)
{
    if (data.size() < 4)
        return false;

    for (size_t i = 0; i + 3 < data.size(); ++i)
    {
        if (data[i]     == '\r' &&
            data[i + 1] == '\n' &&
            data[i + 2] == '\r' &&
            data[i + 3] == '\n')
        {
            position = i + 4;
            return true;
        }
    }

    return false;
}

static bool decode_chunked_body(
    const std::vector<uint8_t> &input,
    std::vector<uint8_t> &output)
{
    size_t position = 0;

    while (position < input.size())
    {
        /*
         * Buscar el final de la línea del tamaño.
         */
        size_t lineEnd = position;

        while (lineEnd + 1 < input.size())
        {
            if (input[lineEnd] == '\r' &&
                input[lineEnd + 1] == '\n')
            {
                break;
            }

            ++lineEnd;
        }

        if (lineEnd + 1 >= input.size())
            return false;

        /*
         * Tamaño del chunk en hexadecimal.
         *
         * Ejemplo:
         *
         * 2a
         */
        std::string sizeString(
            reinterpret_cast<const char *>(
                input.data() + position
            ),
            lineEnd - position
        );

        /*
         * El chunk puede tener extensiones:
         *
         * 2a;extension=value
         *
         * Por ahora las ignoramos.
         */
        size_t extensionPosition =
            sizeString.find(';');

        if (extensionPosition != std::string::npos)
        {
            sizeString.erase(extensionPosition);
        }

        sizeString = trim(sizeString);

        if (sizeString.empty())
            return false;

        size_t chunkSize = 0;

        for (char c : sizeString)
        {
            uint8_t value;

            if (c >= '0' && c <= '9')
            {
                value = c - '0';
            }
            else if (c >= 'a' && c <= 'f')
            {
                value = c - 'a' + 10;
            }
            else if (c >= 'A' && c <= 'F')
            {
                value = c - 'A' + 10;
            }
            else
            {
                return false;
            }

            chunkSize =
                chunkSize * 16 + value;
        }

        /*
         * Avanzar después de:
         *
         * <size>\r\n
         */
        position = lineEnd + 2;

        /*
         * Chunk de tamaño 0 = fin.
         */
        if (chunkSize == 0)
        {
            return true;
        }

        /*
         * Comprobar que tenemos todos los datos
         * del chunk.
         */
        if (position + chunkSize > input.size())
            return false;

        /*
         * Copiar los datos del chunk.
         */
        output.insert(
            output.end(),
            input.begin() + position,
            input.begin() + position + chunkSize
        );

        position += chunkSize;

        /*
         * Cada chunk termina en \r\n.
         */
        if (position + 1 >= input.size())
            return false;

        if (input[position] != '\r' ||
            input[position + 1] != '\n')
        {
            return false;
        }

        position += 2;
    }

    return false;
}


static bool parse_response(
    const std::vector<uint8_t> &data,
    HttpResponse *response)
{
    size_t headerEnd = 0;

    if (!find_header_end(data, headerEnd))
        return false;

    std::string headers(
        reinterpret_cast<const char *>(data.data()),
        headerEnd
    );

    size_t firstLineEnd =
        headers.find("\r\n");

    if (firstLineEnd == std::string::npos)
        return false;

    /*
     * HTTP/1.1 200 OK
     */
    std::string statusLine =
        headers.substr(
            0,
            firstLineEnd
        );

    size_t firstSpace =
        statusLine.find(' ');

    if (firstSpace == std::string::npos)
        return false;

    size_t secondSpace =
        statusLine.find(
            ' ',
            firstSpace + 1
        );

    std::string statusCodeString;

    if (secondSpace == std::string::npos)
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
                secondSpace - firstSpace - 1
            );
    }

    int statusCode = 0;

    for (char c : statusCodeString)
    {
        if (c < '0' || c > '9')
            return false;

        statusCode =
            statusCode * 10 +
            (c - '0');
    }

    response->status_code = statusCode;

    /*
     * Parse headers.
     */
    size_t position =
        firstLineEnd + 2;

    while (position < headerEnd - 2)
    {
        size_t lineEnd =
            headers.find(
                "\r\n",
                position
            );

        if (lineEnd == std::string::npos)
            break;

        if (lineEnd == position)
            break;

        std::string line =
            headers.substr(
                position,
                lineEnd - position
            );

        size_t colon =
            line.find(':');

        if (colon != std::string::npos)
        {
            std::string name =
                trim(line.substr(0, colon));

            std::string value =
                trim(line.substr(colon + 1));

            response->headers.emplace_back(
                name,
                value
            );
        }

        position = lineEnd + 2;
    }

    /*
    * Body.
    */
    if (headerEnd < data.size())
    {
        std::vector<uint8_t> bodyData(
            data.begin() + headerEnd,
            data.end()
        );

        bool chunked = false;

        for (const auto &header : response->headers)
        {
            if (to_lower(header.first) == "transfer-encoding" &&
                to_lower(header.second) == "chunked")
            {
                chunked = true;
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
            response->body = std::move(bodyData);
        }
    }


    return true;
}


/* ============================================================
 * Client
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
    if (!client || !url || !request)
        return -1;

    HttpRequest *result =
        http_request_create(
            method,
            url
        );

    if (!result)
        return -1;

    *request = result;

    return 0;
}


int http_client_execute(
    HttpClient *client,
    HttpRequest *request,
    HttpResponse **response)
{
    if (!client || !request || !response)
        return -1;

    bool https = false;

    std::string host;
    std::string path;

    uint16_t port = 0;

    if (!parse_url(
            request->url.c_str(),
            https,
            host,
            port,
            path))
    {
        return -1;
    }

    /*
     * HTTPS is not supported by the TCP-only
     * implementation yet.
     */
    if (https)
        return -1;

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
        tcp_client_destroy(tcp);
        return -1;
    }

    /*
     * Build HTTP request.
     */
    std::string message;

    message +=
        method_to_string(request->method);

    message += " ";
    message += path;
    message += " HTTP/1.1\r\n";

    /*
     * Host header.
     */
    bool hasHost = false;
    bool hasContentLength = false;
    bool hasConnection = false;

    for (const auto &header : request->headers)
    {
        std::string name =
            to_lower(header.first);

        if (name == "host")
            hasHost = true;

        if (name == "content-length")
            hasContentLength = true;

        if (name == "connection")
            hasConnection = true;

        message += header.first;
        message += ": ";
        message += header.second;
        message += "\r\n";
    }

    if (!hasHost)
    {
        message += "Host: ";
        message += host;
        message += "\r\n";
    }

    if (!hasContentLength &&
        !request->body.empty())
    {
        message += "Content-Length: ";
        message +=
            std::to_string(
                request->body.size()
            );
        message += "\r\n";
    }

    if (!hasConnection)
    {
        message +=
            "Connection: close\r\n";
    }

    message += "\r\n";

    /*
     * Headers.
     */
    result =
        tcp_client_send(
            tcp,
            message.data(),
            message.size()
        );

    if (result < 0)
    {
        tcp_client_close(tcp);
        tcp_client_destroy(tcp);
        return -1;
    }

    /*
     * Body.
     */
    if (!request->body.empty())
    {
        result =
            tcp_client_send(
                tcp,
                request->body.data(),
                request->body.size()
            );

        if (result < 0)
        {
            tcp_client_close(tcp);
            tcp_client_destroy(tcp);
            return -1;
        }
    }

    /*
     * Receive response.
     */
    std::vector<uint8_t> data;

    uint8_t buffer[4096];

    while (true)
    {
        int received =
            tcp_client_receive(
                tcp,
                buffer,
                sizeof(buffer)
            );

        if (received <= 0)
            break;

        data.insert(
            data.end(),
            buffer,
            buffer + received
        );
    }

    tcp_client_close(tcp);
    tcp_client_destroy(tcp);

    /*
     * Parse response.
     */
    HttpResponse *resultResponse =
        new HttpResponse;

    if (!parse_response(
            data,
            resultResponse))
    {
        delete resultResponse;
        return -1;
    }

    *response = resultResponse;

    return 0;
}


void http_client_destroy(
    HttpClient *client)
{
    delete client;
}


/* ============================================================
 * Request
 * ============================================================ */

HttpRequest *http_request_create(
    HttpMethod method,
    const char *url)
{
    if (!url)
        return nullptr;

    HttpRequest *request =
        new HttpRequest;

    request->method = method;
    request->url = url;
    request->timeout_ms = 0;

    return request;
}


int http_request_set_header(
    HttpRequest *request,
    const char *name,
    const char *value)
{
    if (!request || !name || !value)
        return -1;

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

    if (!data && size != 0)
        return -1;

    const uint8_t *bytes =
        static_cast<const uint8_t *>(data);

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
 * Response
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
    if (!response || !name)
        return nullptr;

    std::string requested =
        to_lower(name);

    for (const auto &header :
         response->headers)
    {
        if (to_lower(header.first) ==
            requested)
        {
            /*
             * This pointer is only valid while
             * the response exists.
             */
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

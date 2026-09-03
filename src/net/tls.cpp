#include <sys/net/tls.h>
#include <sys/net/tcp.h>

#include <pak_loader/pak_loader.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include <string>
#include <vector>


/* ============================================================
 * TLS BACKEND
 * ============================================================ */

#if defined(PLATFORM_3DS)

#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>

#else

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#endif


/* ============================================================
 * TlsClient
 * ============================================================ */

struct TlsClient
{
    /*
     * TcpClient NO pertenece a TlsClient.
     *
     * HttpClient es el propietario del TcpClient.
     */
    TcpClient *tcp;

    std::string hostname;
    std::string ca_file;

    bool connected;

    /*
     * PEM completo del CA bundle.
     */
    std::vector<uint8_t> ca_pem;


#if defined(PLATFORM_3DS)

    mbedtls_ssl_context ssl;
    mbedtls_ssl_config config;

    mbedtls_x509_crt ca;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    bool ssl_initialized;
    bool config_initialized;
    bool ca_initialized;
    bool entropy_initialized;
    bool ctr_drbg_initialized;

    bool ssl_setup;
    bool rng_seeded;


#else

    SSL_CTX *ctx;
    SSL *ssl;

#endif
};


/* ============================================================
 * FORWARD DECLARATIONS
 * ============================================================ */

TlsClient *tls_client_create(void);

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


/* ============================================================
 * 3DS / MBED TLS
 * ============================================================ */

#if defined(PLATFORM_3DS)


/* ============================================================
 * MBED TLS ERROR
 * ============================================================ */

static void print_mbedtls_error(
    int error)
{
    char buffer[256];

    std::memset(
        buffer,
        0,
        sizeof(buffer)
    );

    mbedtls_strerror(
        error,
        buffer,
        sizeof(buffer)
    );

    std::printf(
        "MbedTLS error: %s (%d)\n",
        buffer,
        error
    );
}

/* ============================================================
 * MBED TLS CERTIFICATE VERIFY CALLBACK
 * ============================================================
 *
 * La 3DS puede tener el RTC incorrecto (por ejemplo 2011)
 * porque la batería/pila del reloj está agotada.
 *
 * Ignoramos SOLAMENTE:
 *
 *   MBEDTLS_X509_BADCERT_EXPIRED
 *   MBEDTLS_X509_BADCERT_FUTURE
 *
 * El resto de errores de certificado siguen siendo errores.
 *
 * IMPORTANTE:
 * No usamos MBEDTLS_SSL_VERIFY_NONE.
 */

static int mbedtls_verify_certificate(
    void *data,
    mbedtls_x509_crt *crt,
    int depth,
    uint32_t *flags)
{
    (void)data;
    (void)crt;
    (void)depth;

    if (!flags)
        return -1;

    /*
     * Ignorar únicamente los errores relacionados
     * con la fecha/hora del certificado.
     */
    *flags &=
        ~(
            MBEDTLS_X509_BADCERT_EXPIRED |
            MBEDTLS_X509_BADCERT_FUTURE
        );

    /*
     * Si quedan otros errores, Mbed TLS seguirá
     * considerando el certificado inválido.
     *
     * Devolver 0 permite que Mbed TLS continúe
     * utilizando los flags modificados.
     */
    return 0;
}


/* ============================================================
 * TCP SEND CALLBACK
 * ============================================================ */

static int mbedtls_tcp_send(
    void *context,
    const unsigned char *buffer,
    size_t size)
{
    TcpClient *tcp =
        static_cast<TcpClient *>(context);

    if (!tcp)
        return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;

    if (!buffer)
        return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;

    if (size == 0)
        return 0;


    int result =
        tcp_client_send(
            tcp,
            buffer,
            size
        );


    if (result < 0)
    {
        return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
    }


    if (result == 0)
    {
        return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
    }


    return result;
}


/* ============================================================
 * TCP RECEIVE CALLBACK
 * ============================================================ */

static int mbedtls_tcp_receive(
    void *context,
    unsigned char *buffer,
    size_t size)
{
    TcpClient *tcp =
        static_cast<TcpClient *>(context);

    if (!tcp)
        return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;

    if (!buffer)
        return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;

    if (size == 0)
        return 0;


    int result =
        tcp_client_receive(
            tcp,
            buffer,
            size
        );


    if (result < 0)
    {
        return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
    }


    /*
     * 0 significa EOF real.
     */
    if (result == 0)
    {
        return 0;
    }


    return result;
}


#endif


/* ============================================================
 * OPENSSL BIO
 * ============================================================ */

#if !defined(PLATFORM_3DS)


static int openssl_tcp_bio_create(
    BIO *bio)
{
    if (!bio)
        return 0;


    BIO_set_init(
        bio,
        1
    );


    BIO_set_data(
        bio,
        nullptr
    );


    return 1;
}


static int openssl_tcp_bio_destroy(
    BIO *bio)
{
    if (!bio)
        return 0;


    BIO_set_data(
        bio,
        nullptr
    );


    BIO_set_init(
        bio,
        0
    );


    return 1;
}


static int openssl_tcp_bio_read(
    BIO *bio,
    char *buffer,
    int size)
{
    if (!bio ||
        !buffer ||
        size <= 0)
    {
        return 0;
    }


    TcpClient *tcp =
        static_cast<TcpClient *>(
            BIO_get_data(
                bio
            )
        );


    if (!tcp)
        return -1;


    int result =
        tcp_client_receive(
            tcp,
            buffer,
            static_cast<size_t>(
                size
            )
        );


    if (result < 0)
        return -1;


    if (result == 0)
        return 0;


    return result;
}


static int openssl_tcp_bio_write(
    BIO *bio,
    const char *buffer,
    int size)
{
    if (!bio ||
        !buffer ||
        size <= 0)
    {
        return 0;
    }


    TcpClient *tcp =
        static_cast<TcpClient *>(
            BIO_get_data(
                bio
            )
        );


    if (!tcp)
        return -1;


    int result =
        tcp_client_send(
            tcp,
            buffer,
            static_cast<size_t>(
                size
            )
        );


    if (result <= 0)
        return -1;


    return result;
}


static long openssl_tcp_bio_ctrl(
    BIO *bio,
    int command,
    long num,
    void *ptr)
{
    (void)bio;
    (void)num;
    (void)ptr;


    switch (command)
    {
        case BIO_CTRL_FLUSH:
            return 1;

        case BIO_CTRL_PENDING:
            return 0;

        case BIO_CTRL_WPENDING:
            return 0;

        default:
            return 0;
    }
}


static BIO_METHOD *
get_openssl_tcp_bio_method()
{
    static BIO_METHOD *method =
        nullptr;


    if (method)
        return method;


    method =
        BIO_meth_new(
            BIO_TYPE_SOURCE_SINK,
            "Tridingine TCP BIO"
        );


    if (!method)
        return nullptr;


    BIO_meth_set_create(
        method,
        openssl_tcp_bio_create
    );


    BIO_meth_set_destroy(
        method,
        openssl_tcp_bio_destroy
    );


    BIO_meth_set_read(
        method,
        openssl_tcp_bio_read
    );


    BIO_meth_set_write(
        method,
        openssl_tcp_bio_write
    );


    BIO_meth_set_ctrl(
        method,
        openssl_tcp_bio_ctrl
    );


    return method;
}


#endif


/* ============================================================
 * CREATE
 * ============================================================ */

TlsClient *tls_client_create(void)
{
    TlsClient *client =
        new TlsClient;


    client->tcp =
        nullptr;


    client->hostname.clear();
    client->ca_file.clear();
    client->ca_pem.clear();


    client->connected =
        false;


#if defined(PLATFORM_3DS)

    client->ssl_initialized =
        false;

    client->config_initialized =
        false;

    client->ca_initialized =
        false;

    client->entropy_initialized =
        false;

    client->ctr_drbg_initialized =
        false;

    client->ssl_setup =
        false;

    client->rng_seeded =
        false;


    /*
     * ========================================================
     * SSL
     * ========================================================
     */

    mbedtls_ssl_init(
        &client->ssl
    );

    client->ssl_initialized =
        true;


    /*
     * ========================================================
     * CONFIG
     * ========================================================
     */

    mbedtls_ssl_config_init(
        &client->config
    );

    client->config_initialized =
        true;


    /*
     * ========================================================
     * CA
     * ========================================================
     */

    mbedtls_x509_crt_init(
        &client->ca
    );

    client->ca_initialized =
        true;


    /*
     * ========================================================
     * ENTROPY
     * ========================================================
     */

    mbedtls_entropy_init(
        &client->entropy
    );

    client->entropy_initialized =
        true;


    /*
     * ========================================================
     * CTR DRBG
     * ========================================================
     */

    mbedtls_ctr_drbg_init(
        &client->ctr_drbg
    );

    client->ctr_drbg_initialized =
        true;


    /*
     * ========================================================
     * RNG SEED
     * ========================================================
     */

    static const char personalization[] =
        "TridingineTLS";


    int result =
        mbedtls_ctr_drbg_seed(
            &client->ctr_drbg,
            mbedtls_entropy_func,
            &client->entropy,
            reinterpret_cast<
                const unsigned char *
            >(
                personalization
            ),
            sizeof(personalization) - 1
        );


    if (result != 0)
    {
        print_mbedtls_error(
            result
        );


        tls_client_destroy(
            client
        );


        return nullptr;
    }


    client->rng_seeded =
        true;


#else

    client->ctx =
        nullptr;

    client->ssl =
        nullptr;


    client->ctx =
        SSL_CTX_new(
            TLS_client_method()
        );


    if (!client->ctx)
    {
        delete client;
        return nullptr;
    }


    SSL_CTX_set_verify(
        client->ctx,
        SSL_VERIFY_PEER,
        nullptr
    );


    /*
     * Mantener las CAs del sistema como fallback.
     *
     * El bundle propio se añade posteriormente.
     */

    SSL_CTX_set_default_verify_paths(
        client->ctx
    );

#endif


    return client;
}


/* ============================================================
 * SET HOSTNAME
 * ============================================================ */

int tls_client_set_hostname(
    TlsClient *client,
    const char *hostname)
{
    if (!client ||
        !hostname ||
        !hostname[0])
    {
        return -1;
    }


    client->hostname =
        hostname;


    return 0;
}


/* ============================================================
 * SET CA FILE
 * ============================================================ */

int tls_client_set_ca_file(
    TlsClient *client,
    const char *path)
{
    if (!client ||
        !path ||
        !path[0])
    {
        return -1;
    }


    /*
     * ========================================================
     * ABRIR MEDIANTE PAK_FILE
     * ========================================================
     */

    PAK_FILE *file =
        PAKL_LoadFile(
            path
        );


    if (!file)
        return -1;


    /*
     * ========================================================
     * OBTENER TAMAÑO
     * ========================================================
     */

    if (PAKL_fseek(
            file,
            0,
            SEEK_END
        ) != 0)
    {
        PAKL_CloseFile(
            file
        );

        return -1;
    }


    long fileSize =
        PAKL_ftell(
            file
        );


    if (fileSize <= 0)
    {
        PAKL_CloseFile(
            file
        );

        return -1;
    }


    /*
     * Volver al principio.
     */

    if (PAKL_fseek(
            file,
            0,
            SEEK_SET
        ) != 0)
    {
        PAKL_CloseFile(
            file
        );

        return -1;
    }


    /*
     * ========================================================
     * LEER PEM
     * ========================================================
     */

    client->ca_pem.resize(
        static_cast<size_t>(
            fileSize
        ) + 1
    );


    size_t read =
        PAKL_fread(
            client->ca_pem.data(),
            1,
            static_cast<size_t>(
                fileSize
            ),
            file
        );


    PAKL_CloseFile(
        file
    );


    if (read !=
        static_cast<size_t>(
            fileSize
        ))
    {
        client->ca_pem.clear();

        return -1;
    }


    /*
     * NUL final.
     */

    client->ca_pem[
        static_cast<size_t>(
            fileSize
        )
    ] = '\0';


    client->ca_file =
        path;


#if defined(PLATFORM_3DS)

    /*
     * ========================================================
     * MBED TLS
     * ========================================================
     *
     * Limpiar CA anterior.
     */

    if (!client->ca_initialized)
    {
        mbedtls_x509_crt_init(
            &client->ca
        );

        client->ca_initialized =
            true;
    }
    else
    {
        mbedtls_x509_crt_free(
            &client->ca
        );

        mbedtls_x509_crt_init(
            &client->ca
        );
    }


    /*
     * ========================================================
     * PARSEAR BUNDLE
     * ========================================================
     */

    int result =
        mbedtls_x509_crt_parse(
            &client->ca,
            client->ca_pem.data(),
            client->ca_pem.size()
        );


    /*
     * Mbed TLS puede devolver un valor positivo
     * si algunos certificados del bundle no pudieron
     * ser procesados.
     *
     * Lo importante es que al menos haya cargado
     * correctamente un certificado.
     */

    if (result < 0)
    {
        print_mbedtls_error(
            result
        );


        mbedtls_x509_crt_free(
            &client->ca
        );


        mbedtls_x509_crt_init(
            &client->ca
        );


        client->ca_pem.clear();
        client->ca_file.clear();


        return -1;
    }


    /*
     * Comprobar que existe al menos un certificado.
     */

    if (!client->ca.next &&
        client->ca.raw.len == 0)
    {
        client->ca_pem.clear();
        client->ca_file.clear();

        return -1;
    }


    return 0;


#else

    /*
     * ========================================================
     * OPENSSL
     * ========================================================
     */

    if (!client->ctx)
        return -1;


    BIO *bio =
        BIO_new_mem_buf(
            client->ca_pem.data(),
            static_cast<int>(
                client->ca_pem.size() - 1
            )
        );


    if (!bio)
        return -1;


    X509_STORE *store =
        SSL_CTX_get_cert_store(
            client->ctx
        );


    if (!store)
    {
        BIO_free(
            bio
        );

        return -1;
    }


    bool found =
        false;


    while (true)
    {
        X509 *certificate =
            PEM_read_bio_X509(
                bio,
                nullptr,
                nullptr,
                nullptr
            );


        if (!certificate)
            break;


        found =
            true;


        /*
         * Si ya existe, OpenSSL puede devolver 0.
         *
         * No lo tratamos como error.
         */

        X509_STORE_add_cert(
            store,
            certificate
        );


        X509_free(
            certificate
        );
    }


    BIO_free(
        bio
    );


    if (!found)
        return -1;


    return 0;

#endif
}


/* ============================================================
 * SET CA PEM
 * ============================================================ */

int tls_client_set_ca_pem(
    TlsClient *client,
    const void *data,
    size_t size)
{
    if (!client ||
        !data ||
        size == 0)
    {
        return -1;
    }


    /*
     * Guardar copia común.
     */

    client->ca_pem.resize(
        size + 1
    );


    std::memcpy(
        client->ca_pem.data(),
        data,
        size
    );


    client->ca_pem[size] =
        '\0';


#if defined(PLATFORM_3DS)

    /*
     * ========================================================
     * MBED TLS
     * ========================================================
     */

    if (!client->ca_initialized)
    {
        mbedtls_x509_crt_init(
            &client->ca
        );

        client->ca_initialized =
            true;
    }
    else
    {
        mbedtls_x509_crt_free(
            &client->ca
        );

        mbedtls_x509_crt_init(
            &client->ca
        );
    }


    int result =
        mbedtls_x509_crt_parse(
            &client->ca,
            client->ca_pem.data(),
            client->ca_pem.size()
        );


    if (result < 0)
    {
        print_mbedtls_error(
            result
        );


        mbedtls_x509_crt_free(
            &client->ca
        );


        mbedtls_x509_crt_init(
            &client->ca
        );


        client->ca_pem.clear();

        return -1;
    }


    if (!client->ca.next &&
        client->ca.raw.len == 0)
    {
        client->ca_pem.clear();

        return -1;
    }


    client->ca_file.clear();

    return 0;


#else

    /*
     * ========================================================
     * OPENSSL
     * ========================================================
     */

    if (!client->ctx)
        return -1;


    BIO *bio =
        BIO_new_mem_buf(
            data,
            static_cast<int>(
                size
            )
        );


    if (!bio)
        return -1;


    X509_STORE *store =
        SSL_CTX_get_cert_store(
            client->ctx
        );


    if (!store)
    {
        BIO_free(
            bio
        );

        return -1;
    }


    bool found =
        false;


    while (true)
    {
        X509 *certificate =
            PEM_read_bio_X509(
                bio,
                nullptr,
                nullptr,
                nullptr
            );


        if (!certificate)
            break;


        found =
            true;


        X509_STORE_add_cert(
            store,
            certificate
        );


        X509_free(
            certificate
        );
    }


    BIO_free(
        bio
    );


    if (!found)
        return -1;


    client->ca_file.clear();

    return 0;

#endif
}


/* ============================================================
 * CONNECT
 * ============================================================ */

int tls_client_connect(
    TlsClient *client,
    TcpClient *tcp)
{
    if (!client ||
        !tcp)
    {
        return -1;
    }


    if (client->connected)
        return 0;


    if (client->hostname.empty())
        return -1;


    client->tcp =
        tcp;


#if defined(PLATFORM_3DS)

    /*
     * ========================================================
     * MBED TLS CONFIG
     * ========================================================
     *
     * IMPORTANTE:
     *
     * NO hacemos ssl_session_reset() aquí.
     *
     * El contexto se encuentra recién inicializado.
     */


    if (!client->ssl_initialized ||
        !client->config_initialized ||
        !client->ca_initialized ||
        !client->entropy_initialized ||
        !client->ctr_drbg_initialized)
    {
        client->tcp =
            nullptr;

        return -1;
    }


    if (!client->rng_seeded)
    {
        client->tcp =
            nullptr;

        return -1;
    }


    if (client->ca_pem.empty())
    {
        client->tcp =
            nullptr;

        return -1;
    }


    /*
     * ========================================================
     * CONFIG DEFAULTS
     * ========================================================
     */

    int result =
        mbedtls_ssl_config_defaults(
            &client->config,
            MBEDTLS_SSL_IS_CLIENT,
            MBEDTLS_SSL_TRANSPORT_STREAM,
            MBEDTLS_SSL_PRESET_DEFAULT
        );


    if (result != 0)
    {
        print_mbedtls_error(
            result
        );

        client->tcp =
            nullptr;

        return -1;
    }


    /*
     * ========================================================
     * TLS 1.2
     * ========================================================
     */

#if defined(MBEDTLS_SSL_PROTO_TLS1_2)

    mbedtls_ssl_conf_min_version(
        &client->config,
        MBEDTLS_SSL_MAJOR_VERSION_3,
        MBEDTLS_SSL_MINOR_VERSION_3
    );


    mbedtls_ssl_conf_max_version(
        &client->config,
        MBEDTLS_SSL_MAJOR_VERSION_3,
        MBEDTLS_SSL_MINOR_VERSION_3
    );

#endif


    /*
     * ========================================================
     * CERTIFICATE VERIFICATION
     * ========================================================
     */

    mbedtls_ssl_conf_authmode(
        &client->config,
        MBEDTLS_SSL_VERIFY_REQUIRED
    );

    /*
    * ========================================================
    * CERTIFICATE VERIFY CALLBACK
    * ========================================================
    *
    * Mantener la verificación completa del certificado,
    * pero ignorar solamente los errores de fecha.
    */
    mbedtls_ssl_conf_verify(
        &client->config,
        mbedtls_verify_certificate,
        nullptr
    );

    /*
     * ========================================================
     * CA CHAIN
     * ========================================================
     */

    mbedtls_ssl_conf_ca_chain(
        &client->config,
        &client->ca,
        nullptr
    );


    /*
     * ========================================================
     * RNG
     * ========================================================
     */

    mbedtls_ssl_conf_rng(
        &client->config,
        mbedtls_ctr_drbg_random,
        &client->ctr_drbg
    );


    /*
     * ========================================================
     * SSL SETUP
     * ========================================================
     */

    result =
        mbedtls_ssl_setup(
            &client->ssl,
            &client->config
        );


    if (result != 0)
    {
        print_mbedtls_error(
            result
        );

        client->tcp =
            nullptr;

        return -1;
    }


    client->ssl_setup =
        true;


    /*
     * ========================================================
     * HOSTNAME / SNI
     * ========================================================
     */

    result =
        mbedtls_ssl_set_hostname(
            &client->ssl,
            client->hostname.c_str()
        );


    if (result != 0)
    {
        print_mbedtls_error(
            result
        );


        mbedtls_ssl_free(
            &client->ssl
        );


        mbedtls_ssl_init(
            &client->ssl
        );


        client->ssl_setup =
            false;


        client->tcp =
            nullptr;


        return -1;
    }


    /*
     * ========================================================
     * TCP BIO
     * ========================================================
     */

    mbedtls_ssl_set_bio(
        &client->ssl,
        client->tcp,
        mbedtls_tcp_send,
        mbedtls_tcp_receive,
        nullptr
    );


    /*
     * ========================================================
     * HANDSHAKE
     * ========================================================
     */

    while (true)
    {
        result =
            mbedtls_ssl_handshake(
                &client->ssl
            );


        if (result == 0)
        {
            break;
        }


        if (result ==
                MBEDTLS_ERR_SSL_WANT_READ ||
            result ==
                MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            continue;
        }


        /*
         * Error real.
         */

        print_mbedtls_error(
            result
        );


        mbedtls_ssl_free(
            &client->ssl
        );


        mbedtls_ssl_init(
            &client->ssl
        );


        client->ssl_setup =
            false;


        client->tcp =
            nullptr;


        return -1;
    }


    /*
     * ========================================================
     * VERIFY RESULT
     * ========================================================
     */

    uint32_t verifyFlags =
        mbedtls_ssl_get_verify_result(
            &client->ssl
        );


    if (verifyFlags != 0)
    {
        char verifyBuffer[512];


        std::memset(
            verifyBuffer,
            0,
            sizeof(verifyBuffer)
        );


        mbedtls_x509_crt_verify_info(
            verifyBuffer,
            sizeof(verifyBuffer),
            "",
            verifyFlags
        );


        std::printf(
            "TLS certificate verification failed:\n%s\n",
            verifyBuffer
        );


        /*
         * No hacemos close_notify después de un handshake
         * fallido por verificación.
         */

        mbedtls_ssl_free(
            &client->ssl
        );


        mbedtls_ssl_init(
            &client->ssl
        );


        client->ssl_setup =
            false;


        client->tcp =
            nullptr;


        return -1;
    }


#else

    /*
     * ========================================================
     * OPENSSL
     * ========================================================
     */

    if (!client->ctx)
    {
        client->tcp =
            nullptr;

        return -1;
    }


    client->ssl =
        SSL_new(
            client->ctx
        );


    if (!client->ssl)
    {
        client->tcp =
            nullptr;

        return -1;
    }


    /*
     * ========================================================
     * SNI
     * ========================================================
     */

    if (!SSL_set_tlsext_host_name(
            client->ssl,
            client->hostname.c_str()
        ))
    {
        SSL_free(
            client->ssl
        );


        client->ssl =
            nullptr;


        client->tcp =
            nullptr;


        return -1;
    }


    /*
     * ========================================================
     * HOSTNAME VERIFICATION
     * ========================================================
     */

#if OPENSSL_VERSION_NUMBER >= 0x10100000L

    if (!SSL_set1_host(
            client->ssl,
            client->hostname.c_str()
        ))
    {
        SSL_free(
            client->ssl
        );


        client->ssl =
            nullptr;


        client->tcp =
            nullptr;


        return -1;
    }

#endif


    /*
     * ========================================================
     * BIO
     * ========================================================
     */

    BIO_METHOD *bioMethod =
        get_openssl_tcp_bio_method();


    if (!bioMethod)
    {
        SSL_free(
            client->ssl
        );


        client->ssl =
            nullptr;


        client->tcp =
            nullptr;


        return -1;
    }


    BIO *bio =
        BIO_new(
            bioMethod
        );


    if (!bio)
    {
        SSL_free(
            client->ssl
        );


        client->ssl =
            nullptr;


        client->tcp =
            nullptr;


        return -1;
    }


    BIO_set_data(
        bio,
        client->tcp
    );


    SSL_set_bio(
        client->ssl,
        bio,
        bio
    );


    /*
     * ========================================================
     * HANDSHAKE
     * ========================================================
     */

    int result =
        SSL_connect(
            client->ssl
        );


    if (result != 1)
    {
        int error =
            SSL_get_error(
                client->ssl,
                result
            );


        std::printf(
            "OpenSSL SSL_connect failed: %d\n",
            error
        );


        SSL_free(
            client->ssl
        );


        client->ssl =
            nullptr;


        client->tcp =
            nullptr;


        return -1;
    }


    /*
     * ========================================================
     * CERTIFICATE VERIFICATION
     * ========================================================
     */

    long verifyResult =
        SSL_get_verify_result(
            client->ssl
        );


    if (verifyResult != X509_V_OK)
    {
        std::printf(
            "OpenSSL certificate verification failed: %ld\n",
            verifyResult
        );


        SSL_shutdown(
            client->ssl
        );


        SSL_free(
            client->ssl
        );


        client->ssl =
            nullptr;


        client->tcp =
            nullptr;


        return -1;
    }

#endif


    client->connected =
        true;


    return 0;
}


/* ============================================================
 * SEND
 * ============================================================ */

int tls_client_send(
    TlsClient *client,
    const void *data,
    size_t size)
{
    if (!client ||
        !client->connected ||
        !client->tcp ||
        !data)
    {
        return -1;
    }


    if (size == 0)
        return 0;


#if defined(PLATFORM_3DS)

    const unsigned char *bytes =
        static_cast<
            const unsigned char *
        >(data);


    size_t total =
        0;


    while (total < size)
    {
        int result =
            mbedtls_ssl_write(
                &client->ssl,
                bytes + total,
                size - total
            );


        if (result > 0)
        {
            total +=
                static_cast<size_t>(
                    result
                );

            continue;
        }


        if (result ==
                MBEDTLS_ERR_SSL_WANT_READ ||
            result ==
                MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            continue;
        }


        print_mbedtls_error(
            result
        );


        return -1;
    }


    return static_cast<int>(
        total
    );


#else

    const unsigned char *bytes =
        static_cast<
            const unsigned char *
        >(data);


    size_t total =
        0;


    while (total < size)
    {
        size_t remaining =
            size - total;


        int chunkSize =
            remaining > 0x7fffffffU
            ? 0x7fffffff
            : static_cast<int>(
                remaining
            );


        int result =
            SSL_write(
                client->ssl,
                bytes + total,
                chunkSize
            );


        if (result > 0)
        {
            total +=
                static_cast<size_t>(
                    result
                );

            continue;
        }


        int error =
            SSL_get_error(
                client->ssl,
                result
            );


        if (error ==
                SSL_ERROR_WANT_READ ||
            error ==
                SSL_ERROR_WANT_WRITE)
        {
            continue;
        }


        return -1;
    }


    return static_cast<int>(
        total
    );

#endif
}


/* ============================================================
 * RECEIVE
 * ============================================================ */

int tls_client_receive(
    TlsClient *client,
    void *buffer,
    size_t size)
{
    if (!client ||
        !client->connected ||
        !client->tcp ||
        !buffer ||
        size == 0)
    {
        return -1;
    }


#if defined(PLATFORM_3DS)

    while (true)
    {
        int result =
            mbedtls_ssl_read(
                &client->ssl,
                static_cast<
                    unsigned char *
                >(buffer),
                size
            );


        if (result > 0)
            return result;


        /*
         * Cierre TLS limpio.
         */

        if (result == 0 ||
            result ==
                MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
        {
            return 0;
        }


        /*
         * Transporte bloqueante.
         */

        if (result ==
                MBEDTLS_ERR_SSL_WANT_READ ||
            result ==
                MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            continue;
        }


        print_mbedtls_error(
            result
        );


        return -1;
    }


#else

    while (true)
    {
        int result =
            SSL_read(
                client->ssl,
                buffer,
                static_cast<int>(
                    size >
                        0x7fffffffU
                    ? 0x7fffffff
                    : size
                )
            );


        if (result > 0)
            return result;


        int error =
            SSL_get_error(
                client->ssl,
                result
            );


        if (error ==
            SSL_ERROR_ZERO_RETURN)
        {
            return 0;
        }


        if (error ==
                SSL_ERROR_WANT_READ ||
            error ==
                SSL_ERROR_WANT_WRITE)
        {
            continue;
        }


        return -1;
    }

#endif
}


/* ============================================================
 * CLOSE
 * ============================================================ */

int tls_client_close(
    TlsClient *client)
{
    if (!client)
        return -1;


    if (!client->connected)
        return 0;


#if defined(PLATFORM_3DS)

    if (client->ssl_setup)
    {
        mbedtls_ssl_close_notify(
            &client->ssl
        );
    }


#else

    if (client->ssl)
    {
        SSL_shutdown(
            client->ssl
        );
    }

#endif


    client->connected =
        false;


    return 0;
}


/* ============================================================
 * DESTROY
 * ============================================================ */

void tls_client_destroy(
    TlsClient *client)
{
    if (!client)
        return;


    /*
     * Cerrar TLS.
     */

    tls_client_close(
        client
    );


#if defined(PLATFORM_3DS)

    /*
     * ========================================================
     * SSL
     * ========================================================
     */

    if (client->ssl_initialized)
    {
        mbedtls_ssl_free(
            &client->ssl
        );

        client->ssl_initialized =
            false;
    }


    /*
     * ========================================================
     * CONFIG
     * ========================================================
     */

    if (client->config_initialized)
    {
        mbedtls_ssl_config_free(
            &client->config
        );

        client->config_initialized =
            false;
    }


    /*
     * ========================================================
     * CA
     * ========================================================
     */

    if (client->ca_initialized)
    {
        mbedtls_x509_crt_free(
            &client->ca
        );

        client->ca_initialized =
            false;
    }


    /*
     * ========================================================
     * CTR DRBG
     * ========================================================
     */

    if (client->ctr_drbg_initialized)
    {
        mbedtls_ctr_drbg_free(
            &client->ctr_drbg
        );

        client->ctr_drbg_initialized =
            false;
    }


    /*
     * ========================================================
     * ENTROPY
     * ========================================================
     */

    if (client->entropy_initialized)
    {
        mbedtls_entropy_free(
            &client->entropy
        );

        client->entropy_initialized =
            false;
    }


    client->ssl_setup =
        false;

    client->rng_seeded =
        false;


#else

    /*
     * ========================================================
     * SSL
     * ========================================================
     */

    if (client->ssl)
    {
        SSL_free(
            client->ssl
        );

        client->ssl =
            nullptr;
    }


    /*
     * ========================================================
     * CTX
     * ========================================================
     */

    if (client->ctx)
    {
        SSL_CTX_free(
            client->ctx
        );

        client->ctx =
            nullptr;
    }

#endif


    /*
     * PEM.
     */

    client->ca_pem.clear();


    /*
     * TcpClient NO es nuestro.
     */

    client->tcp =
        nullptr;


    delete client;
}
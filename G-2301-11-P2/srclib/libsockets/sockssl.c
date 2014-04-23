#include "sockssl.h"
#include "log.h"

#include <unistd.h>

void inicializar_nivel_SSL()
{
    CRYPTO_malloc_init(); // Initialize malloc, free, etc for OpenSSL's use
    SSL_library_init(); // Initialize OpenSSL's SSL libraries
    SSL_load_error_strings(); // Load SSL error strings
    ERR_load_BIO_strings(); // Load BIO error strings
    OpenSSL_add_all_algorithms(); // Load all available encryption algorithms
}

SSL_CTX *fijar_contexto_SSL(const char *ca_path, const char *key_path, short verify_peer)
{
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_method());

    if (!ctx)
        return NULL;

    if (SSL_CTX_load_verify_locations(ctx, ca_path, NULL) == 0)
    {
        slog(LOG_ERR, "Error cargando certificado raíz.");
        goto error;
    }

    if (SSL_CTX_set_default_verify_paths(ctx) != 1)
	{    
        slog(LOG_ERR, "Error cargando certificados por defecto.");
        goto error;
    }

    if (SSL_CTX_use_certificate_chain_file(ctx, key_path) != 1)
    {
        slog(LOG_ERR, "Error cargando certificado raíz.");
        goto error;
    }

    if (SSL_CTX_use_RSAPrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) != 1)
    {
        slog(LOG_ERR, "Error cargando clave privada.");
        goto error;
    }

    if (SSL_CTX_check_private_key(ctx) != 1)
    {
        slog(LOG_ERR, "Discrepancia entre clave privada/pública.");
        goto error;
    }

    if (verify_peer)
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    else
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    return ctx;

error:
    SSL_CTX_free(ctx);
    return NULL;
}

int conectar_canal_seguro_SSL(SSL_CTX *ctx, int socket, SSL **ssl, const struct sockaddr *addr, socklen_t addr_len)
{
    if (connect(socket, addr, addr_len) == -1)
        return ERR_SOCK;

    *ssl = SSL_new(ctx);

    if (!*ssl)
        return ERR_SSL;

    if (SSL_set_fd(*ssl, socket) == 0)
    {
        cerrar_canal_SSL(*ssl);
        *ssl = NULL;
        return ERR_SSL;
    }

    if (SSL_connect(*ssl) != 1)
        return ERR_SSL;

    return OK;
}

int aceptar_canal_seguro_SSL(SSL_CTX *ctx, int socket, int *newsock, SSL **ssl, struct sockaddr *addr, socklen_t *addr_len)
{
    *newsock = accept(socket, addr, addr_len);

    if (*newsock == -1)
        return ERR_SOCK;

    *ssl = SSL_new(ctx);

    if (!*ssl)
    {
        close(*newsock);
        return ERR_SSL;
    }

    if (SSL_set_fd(*ssl, *newsock) == 0)
    {
        cerrar_canal_SSL(*ssl);
        *ssl = NULL;
        close(*newsock);
        return ERR_SSL;
    }

    if (SSL_accept(*ssl) != 1)
    {
        close(*newsock);
        return ERR_SSL;
    }

    return OK;
}

int evaluar_post_conectar_SSL(SSL_CTX *ctx, SSL *ssl)
{
    X509 *cert = SSL_get_peer_certificate(ssl);

    if (!cert)
        return -1;

    return SSL_get_verify_result(ssl);
}

void cerrar_canal_SSL(SSL *ssl)
{
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

int enviar_datos_SSL(SSL *ssl, const void *msg, ssize_t len)
{
    return SSL_write(ssl, msg, len);
}

ssize_t recibir_datos_SSL(SSL *ssl, void *buffer, ssize_t buflen)
{
    return SSL_read(ssl, buffer, buflen);
}

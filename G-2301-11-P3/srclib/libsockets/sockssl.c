#include "sockssl.h"
#include "log.h"
#include "sockutils.h"
#include "messager.h"
#include "ssltrans.h"

#include <unistd.h>
#include <stdlib.h>

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
    char ca_realpath[512];
    char key_realpath[512];
    SSL_CTX *ctx;

    if (realpath(ca_path, ca_realpath) == NULL)
    {
        slog(LOG_ERR, "Error resolviendo la ruta del certificado CA.");
        return NULL;
    }

    if (realpath(key_path, key_realpath) == NULL)
    {
        slog(LOG_ERR, "Error resolviendo la ruta del certificado.");
        return NULL;
    }

    ctx = SSL_CTX_new(SSLv23_method());

    if (!ctx)
        return NULL;

    if (SSL_CTX_load_verify_locations(ctx, ca_realpath, NULL) == 0)
    {
        slog(LOG_ERR, "Error cargando certificado raíz.");
        goto error;
    }

    if (SSL_CTX_set_default_verify_paths(ctx) != 1)
    {
        slog(LOG_ERR, "Error cargando certificados por defecto.");
        goto error;
    }

    if (SSL_CTX_use_certificate_chain_file(ctx, key_realpath) != 1)
    {
        slog(LOG_ERR, "Error cargando certificado raíz.");
        goto error;
    }

    if (SSL_CTX_use_RSAPrivateKey_file(ctx, key_realpath, SSL_FILETYPE_PEM) != 1)
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
        SSL_free(*ssl);
        *ssl = NULL;
        return ERR_SSL;
    }

    sock_set_block(socket, 1);

    if (SSL_connect(*ssl) != 1)
        return ERR_SSL;
    
    sock_set_block(socket, 0);

    return OK;
}

int aceptar_canal_seguro_SSL(SSL_CTX *ctx, int socket, int *newsock, SSL **ssl, struct sockaddr *addr, socklen_t *addr_len)
{
    *newsock = accept(socket, addr, addr_len);

    if (*newsock == -1)
        return ERR_SOCK;

    sock_set_block(*newsock, 1);

    slog(LOG_DEBUG, "Aceptada conexión en socket %d. Negociando handshake...", *newsock);

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

    sock_set_block(*newsock, 0);

    slog(LOG_DEBUG, "Handshake finalizado con éxito en %d.", *newsock);

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
    if (ssl && ssl != SSL_NOT_CONN)
    {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
}


int enviar_datos_SSL(SSL *ssl, const void *msg, ssize_t len)
{
    return SSL_write(ssl, msg, len);
}

ssize_t recibir_datos_SSL(SSL *ssl, void *buffer, ssize_t buflen)
{
    return SSL_read(ssl, buffer, buflen);
}

void slog_sslerr()
{
    unsigned long error;
    char errorstr[200];
    int count = 0;

    while ((error = ERR_get_error()) != 0)
    {
        count++;
        ERR_error_string_n(error, errorstr, 200);
        slog(LOG_ERR, "Error SSL %d (%s)", error, errorstr,
             ERR_GET_FUNC(error), ERR_GET_LIB(error), ERR_GET_REASON(error));
    }

    if (count == 0)
        slog(LOG_WARNING, "No hay errores SSL.");
    else
        slog(LOG_NOTICE, "Fin errores SSL.");
}

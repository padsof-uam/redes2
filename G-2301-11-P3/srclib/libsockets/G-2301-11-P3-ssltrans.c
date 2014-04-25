#include "G-2301-11-P3-ssltrans.h"
#include "G-2301-11-P3-dictionary.h"
#include "G-2301-11-P3-log.h"

#include <unistd.h>

SSL_CTX *_ctx = NULL;
dictionary *_ssltrans_socket_dic = NULL;
short _verify_peer;

static void _assert_ssltrans_init()
{
    if (!_ctx || !_ssltrans_socket_dic)
    {
        fprintf(stderr, "SSLTRANS not initialized. Please call init_transparent_ssl() and set_ssl_ctx at app start.\n");
        abort();
    }
}

SSL_CTX *get_ssl_ctx()
{
    return _ctx;
}

void set_ssl_ctx(SSL_CTX *ctx)
{
    _ctx = ctx;
}

int init_transparent_ssl()
{
    _ssltrans_socket_dic = dic_new_withint();

    if (!_ssltrans_socket_dic)
        return ERR_MEM;
    else
        return OK;
}

static void _free_ssl_entry(void *entry)
{
    SSL *ssl = entry;

    if (ssl != NULL && ssl != SSL_NOT_CONN)
        cerrar_canal_SSL(ssl);
}

void cleanup_transparent_ssl()
{
    if (_ssltrans_socket_dic)
    {
        dic_destroy(_ssltrans_socket_dic, _free_ssl_entry);
        _ssltrans_socket_dic = NULL;
    }
}

int is_ssl_socket(int socket)
{
    return _ctx != NULL && get_ssl(socket) != NULL;
}

SSL *get_ssl(int socket)
{
    if (_ssltrans_socket_dic)
        return (SSL *) dic_lookup(_ssltrans_socket_dic, &socket);
    else
        return NULL;
}


void set_ssl(int socket, SSL *ssl)
{
    dic_update(_ssltrans_socket_dic, &socket, ssl);
}

int dsocket(int domain, int type, int protocol, short use_ssl)
{
    int sock = socket(domain, type, protocol);

    if (use_ssl)
        _assert_ssltrans_init();

    if (sock != -1 && use_ssl)
        set_ssl(sock, SSL_NOT_CONN);

    return sock;
}

int daccept(int socket, struct sockaddr *addr, socklen_t *addr_len)
{
    int newsock, retval;
    SSL *ssl;

    if (is_ssl_socket(socket))
    {
        retval = aceptar_canal_seguro_SSL(get_ssl_ctx(), socket, &newsock, &ssl, addr, addr_len);

        if (retval != OK)
        {
            slog_sslerr();
            return -1;
        }

        if (_verify_peer)
        {
            retval = evaluar_post_conectar_SSL(get_ssl_ctx(), ssl);

            if (retval != OK)
            {
                slog(LOG_WARNING, "Error de certificado en conexión recibida en %d, retorno %d", newsock, retval);
                slog_sslerr();
                close(retval);
                return -1;
            }
        }

        set_ssl(newsock, ssl);

        return newsock;
    }
    else
    {
        return accept(socket, addr, addr_len);
    }
}

int dconnect(int socket, const struct sockaddr *addr, socklen_t addr_len)
{
    SSL *ssl;
    int retval;

    if (is_ssl_socket(socket))
    {
        retval = conectar_canal_seguro_SSL(get_ssl_ctx(), socket, &ssl, addr, addr_len);

        if (retval != OK)
        {
            slog_sslerr();
            return -1;
        }

        retval = evaluar_post_conectar_SSL(get_ssl_ctx(), ssl);

        if (retval != OK)
        {
            slog_sslerr();
            slog(LOG_WARNING, "Error de certificado en conexión recibida en %d, retorno %d", socket, retval);
            close(retval);
            return -1;
        }

        set_ssl(socket, ssl);

        return OK;
    }
    else
    {
        return connect(socket, addr, addr_len);
    }
}

ssize_t dsend(int socket, const void *buffer, size_t length, int flags)
{
    SSL *ssl = get_ssl(socket);

    if (ssl == SSL_NOT_CONN)
    {
        slog(LOG_ERR, "No se puede escribir en un socket SSL no conectado.");
        return -1;
    }

    if (ssl)
        return enviar_datos_SSL(ssl, buffer, length);
    else
        return send(socket, buffer, length, flags);
}

ssize_t drecv(int socket, void *buffer, size_t length, int flags)
{
    SSL *ssl = get_ssl(socket);

    if (ssl)
        return recibir_datos_SSL(ssl, buffer, length);
    else
        return recv(socket, buffer, length, 0);
}

void dclose(int socket)
{
    SSL *ssl = get_ssl(socket);

    if (ssl && ssl != SSL_NOT_CONN)
    {
        cerrar_canal_SSL(ssl);
        dic_remove(_ssltrans_socket_dic, &socket);
    }

    close(socket);
}

int dshutdown(int socket, int mode)
{
    SSL *ssl = get_ssl(socket);

    if (ssl && ssl != SSL_NOT_CONN)
    {
        cerrar_canal_SSL(ssl);
        dic_remove(_ssltrans_socket_dic, &socket);
    }

    return shutdown(socket, mode);
}


int init_all_ssl(const char *ca_path, const char *ca_key, short verify_peer)
{
    SSL_CTX *ctx;

    init_transparent_ssl();
    inicializar_nivel_SSL();
    ctx = fijar_contexto_SSL(ca_path, ca_key, verify_peer);
    _verify_peer = verify_peer;

    if (!ctx)
    {
        slog(LOG_ERR, "Error al inicializar contexto SSL.");
        ERR_print_errors_fp(stderr);
        return ERR_SSL;
    }

    set_ssl_ctx(ctx);

    slog(LOG_NOTICE, "Contexto SSL inicializado con CA %s, clave %s.", ca_path, ca_key);

    return OK;
}

void cleanup_all_ssl()
{
    cleanup_transparent_ssl();
    SSL_CTX_free(get_ssl_ctx());
    set_ssl_ctx(NULL);
}


int init_all_ssl_default()
{
    return init_all_ssl(CA_PATH, CA_KEY, 0);
}


int init_all_ssl_default_verify()
{
    return init_all_ssl(CA_PATH, CA_KEY, 1);
}

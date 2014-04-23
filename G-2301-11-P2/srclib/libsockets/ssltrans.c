#include "ssltrans.h"
#include "dictionary.h"
#include "log.h"

#include <unistd.h>

SSL_CTX* _ctx = NULL;
dictionary* _sockets = NULL;

SSL_CTX* get_ssl_ctx() { return _ctx; }
void set_ssl_ctx(SSL_CTX* ctx) { _ctx = ctx; }

int init_transparent_ssl()
{
	_sockets = dic_new_withint();

	if(!_sockets)
		return ERR_MEM;
	else
		return OK;
}

void cleanup_transparent_ssl()
{
	if(_sockets)
		dic_destroy(_sockets, (destructor) cerrar_canal_SSL);
}

int is_ssl_dsocket(int socket)
{
	return _ctx != NULL && get_ssl(socket) != NULL;
}

SSL* get_ssl(int socket)
{
	if(_sockets)
		return (SSL*) dic_lookup(_sockets, &socket);
	else
		return NULL;
}


void set_ssl(int socket, SSL* ssl)
{
	dic_update(_sockets, &socket, ssl);
}

int dsocket(int domain, int type, int protocol, short use_ssl)
{
	int sock = socket(domain, type, protocol);

	if(sock != -1 && use_ssl)
		set_ssl(sock, NULL);

	return sock;
}

int daccept(int socket, struct sockaddr* addr, socklen_t* addr_len)
{
	int newsock, retval;
	SSL* ssl;

	if(is_ssl_dsocket(socket))
	{
		retval = aceptar_canal_seguro_SSL(get_ssl_ctx(), socket, &newsock, &ssl, addr, addr_len);

		if(retval != OK)
			return -1;

		if(!evaluar_post_conectar_SSL(get_ssl_ctx(), ssl))		
		{
			slog(LOG_WARNING, "Error de certificado en conexión recibida en %d", retval);
			close(retval);
			return -1;
		}

		return newsock;
	}
	else
	{
		return accept(socket, addr, addr_len);
	}
}

int dconnect(int socket, const struct sockaddr *addr, socklen_t addr_len)
{
	SSL* ssl;
	int retval;

	if(is_ssl_dsocket(socket))
	{
		retval = conectar_canal_seguro_SSL(get_ssl_ctx(), socket, &ssl, addr, addr_len);

		if(retval == OK)
			set_ssl(socket, ssl);

		return retval;
	}
	else
	{
		return connect(socket, addr, addr_len);	
	}
}

ssize_t dsend(int socket, const void *buffer, size_t length, int flags)
{
	SSL* ssl = get_ssl(socket);

	if(ssl)
		return enviar_datos_SSL(ssl, buffer, length);
	else
		return send(socket, buffer, length, flags);
}

ssize_t drecv(int socket, void *buffer, size_t length, int flags)
{
	SSL* ssl = get_ssl(socket);

	if(ssl)
		return recibir_datos_SSL(ssl, buffer, length);
	else
		return recv(socket, buffer, length, 0);
}

void dclose(int socket)
{
	SSL* ssl = get_ssl(socket);

	if(ssl)
	{
		cerrar_canal_SSL(ssl);
		dic_remove(_sockets, &socket);
	}

	close(socket);
}

void dshutdown(int socket, int mode)
{
	SSL* ssl = get_ssl(socket);

	if(ssl)
	{
		cerrar_canal_SSL(ssl);
		dic_remove(_sockets, &socket);
	}

	shutdown(socket, mode);
}

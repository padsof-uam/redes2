#ifndef SSL_TRANS_H
#define SSL_TRANS_H

#include "sockssl.h"

#include <sys/types.h>
#include <sys/socket.h>

SSL_CTX* get_ssl_ctx();
void set_ssl_ctx(SSL_CTX* ctx);

int init_transparent_ssl();
void cleanup_transparent_ssl();

int is_ssl_socket(int socket);
SSL* get_ssl(int socket);
void set_ssl(int socket, SSL* ssl);

int dsocket(int domain, int type, int protocol, short use_ssl);
int daccept(int socket, struct sockaddr* addr, socklen_t* addr_len);
int dconnect(int socket, const struct sockaddr *addr, socklen_t addr_len);
ssize_t dsend(int socket, const void *buffer, size_t length, int flags);
ssize_t drecv(int socket, void *buffer, size_t length, int flags);
void dclose(int socket);
void dshutdown(int socket, int mode);

#endif

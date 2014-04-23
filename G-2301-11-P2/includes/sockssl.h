#ifndef SOCKSSL_H
#define SOCKSSL_H

#include <openssl/bio.h> 
#include <openssl/ssl.h> 
#include <openssl/err.h> 
#include <sys/types.h>
#include <sys/socket.h>

#include "errors.h"

void inicializar_nivel_SSL();
SSL_CTX* fijar_contexto_SSL(const char* ca_path, const char* key_path, short verify_peer);
int conectar_canal_seguro_SSL(SSL_CTX* ctx, int socket, SSL** ssl, const struct sockaddr* addr, socklen_t addr_len);
int aceptar_canal_seguro_SSL(SSL_CTX* ctx, int socket, int* newsock, SSL** ssl);
int evaluar_post_connectar_SSL(SSL_CTX* ctx, SSL* ssl);
void cerrar_canal_SSL(SSL* ssl);
int enviar_datos_SSL(SSL* ssl, const void* msg, ssize_t len);
ssize_t recibir_datos_SSL(SSL* ssl, void* buffer, ssize_t buflen);

#endif

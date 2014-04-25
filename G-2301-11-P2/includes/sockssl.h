#ifndef SOCKSSL_H
#define SOCKSSL_H

#include <openssl/bio.h> 
#include <openssl/ssl.h> 
#include <openssl/err.h> 
#include <sys/types.h>
#include <sys/socket.h>

#include "errors.h"

/**
 * Inicializa las funciones de SSL.
 */
void inicializar_nivel_SSL();

/**
 * Crea el contexto SSL.
 * @param  ca_path     Clave CA.
 * @param  key_path    Clave del cliente.
 * @param  verify_peer 1 si se debe verificar el certificado del par con el que se conecte.
 * @return             Contexto o NULL si ha ocurrido un error.
 */
SSL_CTX* fijar_contexto_SSL(const char* ca_path, const char* key_path, short verify_peer);

/**
 * Conexión SSL a través de un socket-
 * @param  ctx      Contexto SSL.
 * @param  socket   Socket
 * @param  ssl      Puntero donde guardar la estructura SSL resultante
 * @param  addr     Dirección
 * @param  addr_len Longitud de addr.
 * @return          OK/ERR.
 */
int conectar_canal_seguro_SSL(SSL_CTX* ctx, int socket, SSL** ssl, const struct sockaddr* addr, socklen_t addr_len);

/**
 * Acepta una conexión SSL.
 * @param  ctx      Contexto SSL.
 * @param  socket   Socket de escucha.
 * @param  newsock  Devuelve el socket de la nueva conexión.
 * @param  ssl      Devuelve la estructura SSL de la nueva conexión.
 * @param  addr     Dirección.
 * @param  addr_len Longitud de addr.
 * @return          OK/ERR
 */
int aceptar_canal_seguro_SSL(SSL_CTX* ctx, int socket, int* newsock, SSL** ssl, struct sockaddr* addr, socklen_t* addr_len);

/**
 * Comprueba los certificados del par.
 * @param  ctx Contexto SSL.
 * @param  ssl SSL.
 * @return     OK/ERR.
 */
int evaluar_post_conectar_SSL(SSL_CTX* ctx, SSL* ssl);

/**
 * Cierra un canal SSL.
 * @param ssl Canal
 */
void cerrar_canal_SSL(SSL* ssl);

/**
 * Envía datos a través de un canal SSL
 * @param  ssl SSL
 * @param  msg Mensaje a enviar
 * @param  len longitud del mensaje
 * @return     OK/ERR
 */
int enviar_datos_SSL(SSL* ssl, const void* msg, ssize_t len);

/**
 * Recibe datos a través de un canal SSL.
 * @param  ssl    SSL
 * @param  buffer Buffer donde guardar el mensaje
 * @param  buflen Longitud del buffer
 * @return        bytes leídos o -1 si hay error.
 */
ssize_t recibir_datos_SSL(SSL* ssl, void* buffer, ssize_t buflen);

/**
 * Saca por slog los mensajes de error de SSL.
 * @see slog
 */
void slog_sslerr();
#endif

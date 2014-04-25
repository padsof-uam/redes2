#ifndef SSL_TRANS_H
#define SSL_TRANS_H

#include "G-2301-11-P3-sockssl.h"

#include <sys/types.h>
#include <sys/socket.h>

#define CA_PATH "cert/root.pemr"
#define CA_KEY 	"cert/server.pem"

/** Marcador de socket SSL no conectado. */
#define SSL_NOT_CONN ((SSL*) 1)

/**
 * Obtiene el contexto SSL global.
 * @return Contexto.
 */
SSL_CTX* get_ssl_ctx();

/**
 * Guarda el contexto SSL global.
 * @param ctx Contexto.
 */
void set_ssl_ctx(SSL_CTX* ctx);

/**
 * Inicializa los diccionarios internos de SSL transparente.
 * @return OK/ERR.
 */
int init_transparent_ssl();

/**
 * Limpia todas las estructuras de SSL transparente.
 */
void cleanup_transparent_ssl();

/**
 * Inicializa todo lo necesario para funcionar con SSL con
 * los valores por defecto, sin verificar el par.
 * @see init_transparent_ssl
 * @see inicializar_nivel_SSL
 * @see init_all_ssl
 * @return OK/SSL
 */
int init_all_ssl_default();

/**
 * Inicializa todo lo necesario para funcionar con SSL con
 * los valores por defecto, verificando el par.
 * @see init_transparent_ssl
 * @see inicializar_nivel_SSL
 * @return OK/SSL
 */
int init_all_ssl_default_verify();

/**
 * Inicializa las estructuras SSL.
 * @param  ca_path     Clave CA.
 * @param  ca_key      Clave del cliente.
 * @param  verify_peer Indicador de verificación del par.
 * @return             OK/ERR.
 */
int init_all_ssl(const char* ca_path, const char* ca_key, short verify_peer);

/**
 * Limpia todas las estructuras de SSL.
 */
void cleanup_all_ssl();

/**
 * Comprueba si un socket es seguro.
 * @param  socket Socket
 * @return        1 o 0 según sea o no seguro.
 */
int is_ssl_socket(int socket);

/**
 * Devuelve la estructura SSL para un socket dado.
 * @param  socket Socket.
 * @return        Estructura SSL o NULL si no se ha encontrado.
 */
SSL* get_ssl(int socket);

/**
 * Guarda la estructura SSL para un socket dado.
 * @param socket Socket.
 * @param ssl    Estructura SSL.
 */
void set_ssl(int socket, SSL* ssl);

/**
 * Wrapper transparente SSL para socket(2).
 * @param  domain   Dominio
 * @param  type     Tipo de socket.
 * @param  protocol Protocolo
 * @param  use_ssl  Bandera para usar SSL o no.
 * @return          Descriptor de socket o -1
 */
int dsocket(int domain, int type, int protocol, short use_ssl);

/**
 * Wrapper transparente SSL para accept(2).
 * @param  socket   Socket donde aceptar la conexión.
 * @param  addr     Dirección de la conexión aceptada.
 * @param  addr_len Longitud de addr.
 * @return          Descriptor de la nueva conexión o -1 en caso de error.
 */
int daccept(int socket, struct sockaddr* addr, socklen_t* addr_len);

/**
 * Wrapper transparente SSL para connect(2)
 * @param  socket   Socket a conectar
 * @param  addr     Dirección con la que conectar.
 * @param  addr_len Longitud de addr
 * @return          0 ó -1 en caso de error.
 */
int dconnect(int socket, const struct sockaddr *addr, socklen_t addr_len);

/**
 * Wrapper transparente SSL para send(2)
 * @param  socket Socket
 * @param  buffer Buffer
 * @param  length Longitud del buffer
 * @param  flags  Flags de envío
 * @return        bytes enviados.
 */
ssize_t dsend(int socket, const void *buffer, size_t length, int flags);

/**
 * Wrapper transparente SSL para recv(2)
 * @param  socket Socket
 * @param  buffer Buffer
 * @param  length Longitud del buffer
 * @param  flags  Flags.
 * @return        Bytes leídos o -1 si hay error.
 */
ssize_t drecv(int socket, void *buffer, size_t length, int flags);

/**
 * Wrapper transparente SSL para close(2)
 * Libera las estructuras SSL si es necesario.
 * @param socket Socket a cerrar
 */
void dclose(int socket);

/**
 * Wrapper transparente SSL para shutdown(2)
 * Libera las estructuras SSL si es necesario.
 * @param  socket Socket a cerrar
 * @param  mode   Mode
 * @return        -1 si hay error, 0 en otro caso.
 */
int dshutdown(int socket, int mode);

#endif

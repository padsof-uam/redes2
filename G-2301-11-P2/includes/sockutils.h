#ifndef SOCKUTILS_H
#define SOCKUTILS_H

#include <stdlib.h>
#include <stdint.h>

/**
* Función encargada de abrir un socket en un puerto, 
* con una longitud de la cola determinada.
* @param 	port		puerto en el que estará el socket.
* @param	max_long	longitud máxima de la cola de mensajes del socket.
* @return 	Un valor negativo en caso de error o el handler del socket
*			en caso de éxito.
*/
int server_open_socket(int port, int max_long, short use_ssl);

/**
* Función encargada de aceptar una conexión en un socket.
* @param	handler 	Identificador del socket.
* @return 	Un valor negativo en caso de error o el handler del socket
*			en caso de éxito.
*/
int server_listen_connect(int handler);

/**
* Función encargada de cerrar la conexión.
* @param	handler 	Identificador del socket a cerrar.
* @return	OK/ERR.
*/
int server_close_communication(int handler);

/**
 * Devuelve un socket connectado al host y puerto que se pasen como argumentos.
 * @param  host          Host.
 * @param  port          Puerto.
 * @param  resolved_addr Cadena donde se guarda la dirección resuelta.
 * @param  resadr_len    Longitud del buffer donde guardar la dirección resuelta.
 * @param  use_ssl		 Activar uso de SSL o no.
 * @return               Socket o número negativo en caso de error.
 */
int client_connect_to(const char* host, const char* port, char* resolved_addr, size_t resadr_len, short use_ssl);

/**
 * Resuelve un host a una dirección IP4.
 * @param  host Host.
 * @param  ip   IP.
 * @return      OK/ERR.
 */
int resolve_ip4(const char* host, uint32_t* ip);

int ssl_socketpair(int domain, int type, int protocol, int sockets[2]);
int sock_set_block(int socket, short block);

/**
 * Obtiene el puerto en el que está escuchando un socket.
 * @param  sock 	El socket del que queremos saber el puerto.
 * @param  port 	Puntero al puerto donde devolverlo.
 * @return			OK/ERR.
 */
int get_socket_port(int sock, int *port);

/**
 * Abre un socket de escucha UDP.
 * @return	Un valor negativo en caso de error o el handler del socket creado.
 */
int open_listen_udp_socket();

/**
 * Abre un socket de escucha TCP bloqueante.
 * @param  port     Puerto de escucha.
 * @param  max_long Longitud de la cola de escucha del socket.
 * @return	Un valor negativo en caso de error o el handler del socket creado.
 */
int server_open_socket_block(int port, int max_long);
/**
 * Función encargada de aceptar una conexión en un socket (bloqueante)
 * @param  handler Identificador del socket.
 * @return         ERR/Identificador del socket.
 */
int server_listen_connect_block(int handler);

#endif


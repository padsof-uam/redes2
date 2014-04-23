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
int server_open_socket(int port, int max_long);

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
int client_connect_to(const char* host, const char* port, char* resolved_addr, size_t resadr_len);

int resolve_ip4(const char* host, uint32_t* ip);

int get_socket_port(int sock, int *port);

int open_listen_udp_socket();

int open_listen_tcp_socket();

#endif


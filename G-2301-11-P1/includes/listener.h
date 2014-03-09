#ifndef LISTENER_H
#define LISTENER_H 

#include "errors.h"
#include <pthread.h>

#define TCP 6                /* Protocolo TCP                            */
#define UDP 17               /* Protocolo UDP                            */

#define DEFAULT_MAX_QUEUE 150

#define COMM_STOP 1

/**
 * Estructura que almacena los datos necesarios para el funcionamiento
 * 	del hilo de escucha de nuevas conexiones.
 */
struct listener_thdata {
	int port; /**< Puerto de escucha */
	int commsocket; /**< Socket de comunicaciones con el hilo principal. */
	int listen_sock; /**< Socket de escucha. Se guarda en la 
						estructura para poder hacer la limpieza con pthread_cleanup */
};

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

/**
 * Crea un hilo que se encarga de escuchar conexiones entrantes y crear los nuevos hilos
 * 	que las gestionen.
 * @param  pth         Identificador del hilo creado
 * @param  port        Puerto de escucha.
 * @param  commsocket  Socket de comunicaciones con el hilo principal. 
 * @return             OK/ERR.
 */
int spawn_listener_thread(pthread_t* pth, int port, int commsocket);

/**
 * Función que se encarga de escuchar las conexiones entrandes.
 * @param 	data 		Estructura listener_thdata con los datos necesarios.
 * @return	Devuelve siempre NULL:
 */
void* thread_listener(void* data);

/**
 * Crea un nuevo hilo para gestionar una conexión entrante (todavía no aceptada).
 * @param  listen_sock Socket de escucha sobre el que se realiza un accept.
 * @param  commsocket  Socket de comunicaciones con el hilo principal. 
 *                     	Para no usar las comunicaciones con el hilo principal,
 *                     	hay que pasar 0 como argumento.
 * @return             OK/ERR.
 */
int create_new_connection_thread(int listen_sock, int commsocket);


#endif

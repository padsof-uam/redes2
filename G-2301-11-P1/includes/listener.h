#ifndef LISTENER_H
#define LISTENER_H 

#include "errors.h"
#include <pthread.h>

#define TCP 6                /* Protocolo TCP                            */
#define UDP 17               /* Protocolo UDP                            */

#define DEFAULT_MAX_QUEUE 150

#define COMM_STOP 1

typedef void* (*connection_handler)(void*);

struct listener_thdata {
	int port;
	int commsocket;
};

struct newconn_data {
	int commsocket;
};

int server_open_socket(int port, int max_long);
int server_listen_connect(int handler);
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
 * @param data 	Estructura listener_thdata con los datos necesarios.
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

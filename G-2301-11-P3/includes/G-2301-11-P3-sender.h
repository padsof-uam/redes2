#ifndef SENDER_H
#define SENDER_H 

#include "G-2301-11-P3-errors.h"
#include <pthread.h>

/** Estructura que se pasará al hilo de envío como argumento */
struct sender_thdata {
	int queue;
};

/**
 * Crea el hilo encargado de enviar mensajes
 * @param  sender_thread Identificador del hilo.
 * @param  queue         Cola donde se recibirán los mensajes a enviar.
 * @see msg_sockcommdata
 * @return               Código de error si algo sale mal.
 */
int spawn_sender_thread (pthread_t * sender_thread,int queue);

/**
 * Punto de entrada para el hilo de envío.
 * @param  st Datos, como estructura sender_thdata.
 * @return    NULL en todo caso.
 */
void * thread_send(void * st);

#endif

#ifndef SENDER_H
#define SENDER_H 

#include <errors.h>
#include <pthread.h>

struct sender_thdata{
	int socket;
	/*Algo mas?*/
};


/**
* Crea el hilo gestor del envío de mensajes en la comunicación.
* @param	sender_thread		Identificador del hilo creado.
* @return						Código de error.
*/
int spawn_sender_thread (pthread_t * sender_thread);


/** Función que empieza a ejecutar sender.
* @param 	st	La información necesaria, en forma de *********************************
* @return 	**********************************************************
*/
void * thread_send(void * st);

/**
* 
*/
int send_message(int fd, char * message);
#endif
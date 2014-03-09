#ifndef SENDER_H
#define SENDER_H 

#include <errors.h>
#include <pthread.h>

struct sender_thdata{
	int queue;
};


/**
* Crea el hilo gestor del envío de mensajes en la comunicación.
* @param	sender_thread		Identificador del hilo creado.
* @return						Código de error.
*/
int spawn_sender_thread (pthread_t * sender_thread,int queue);


/** Función que empieza a ejecutar sender.
* @param 	st	La información necesaria, en forma de *********************************
* @return 	**********************************************************
*/
void * thread_send(void * st);

/**
* Función auxiliar encargada de enviar un mensaje por un socket. Esta función permite,
* en caso de querer añadir funcionlidades (seguridad, cifrado) cambiar simplemente esta función
* y nada más.
* @param	fd			identificador del socekt por el que mandar el mensaje.
* @param 	message 	mensaje a enviar.
* @param 	len			longitud del mensaje a enviar.
* @return	Código de error.
*/
int send_message(int fd, char * message,int len);
#endif

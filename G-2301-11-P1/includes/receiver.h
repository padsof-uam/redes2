#ifndef RECEIVER_H
#define RECEIVER_H 

#include "errors.h"
#include <pthread.h>
#include "poller.h"

#define MAX_ERR_THRESHOLD 5

struct receiver_thdata{
	int socket;
	/*Algo mas?*/
};

/**
* Función que empieza a ejecutar el hilo receptor.
* @param 	st	La información necesaria, en forma de *********************************
* @return 	**********************************************************
*/
void* thread_receive(void * st);

/**
* Crea un hilo que se encarga de gestionar la recepción en las comunicaciones.
* @param	spawn_receiver_thread 	Identidifacor del hilo creado.
* @return 							Código de error.
*/

int spawn_receiver_thread(pthread_t * recv_thread);

/**
* Añade una nueva conexión a la estructura pollfds dinámica.
* @param	pfds 		La estructura a la que añadir la conexión.
* @param	listener_sk El socket listo para ser leido con la información de la nueva conexión.
* @return				Código de error.
*/
int add_new_connection(struct pollfds* pfds, int listener_sk);

/**
* Elimina una conexión de la estructura pollfds.
* @param 	pfds 	La estructura a la que eliminar la conexión.
* @param 	fd 		El identificador de la conexión para ser cerrada.
* @return 			Código de error.
*/
int remove_connection(struct pollfds* pfds, int fd);

/**
* Recibe y lee los mensajes de una conexión.
* @param 	fd			El socket listo para ser leido.
* @param 	message		Cadena a rellenar con la información leída.
* @return 				Código de error.
*/
int receive_parse_message(int fd,char * message);

/**
* Envía la información al proceso principal.
* @param 	main_process	Estructura con toda la información necesaria para comunicarse con el proceso principal.
* @param 	message			El mensage para ser enviado al proceso principal (sin ser parseado).
* @return 					Código de error.
*/
int send_to_main(void * main_process, const char * message);

#endif
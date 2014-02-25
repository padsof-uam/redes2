#include <sys/socket.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "receiver.h"

void* thread_receive(void * st){
	
	struct pollfds* pfds;
	int sk[2];
	pthread_t listener_th;
	int ready_fds, i, fds_len;
	int errnum = 0;
	int sk_new_connection;
	char message[MAX_IRC_SIZE];


	if(socketpair(PF_LOCAL, SOCK_STREAM, 0, sk) < 0)
	{
		syslog(LOG_CRIT, "No se han podido crear los sockets de comunicación con listener: %s", strerror(errno));
		return NULL;
	}


	pfds = pollfds_init(POLLERR | POLLIN);
	pollfds_add(pfds,sk[0]);

	while(1)
	{
		ready_fds = pollfds_poll(pfds, 1000); /* Esperamos 1 segundo antes de volver. */

		if(ready_fds == -1)
		{
			syslog(LOG_ERR, "error %d ejecutando poll: %s", errnum, strerror(errno));
			errnum++;

			if(errnum >= MAX_ERR_THRESHOLD)
				break; /* Demasiados errores, salimos */
			else
				continue;
		}

		fds_len = pfds->len; /* Evitamos iterar por posibles nuevos descriptores */
		
		if (ready_fds){
			if (pfds->fds[0].revents & POLLERR){
				/*
					¿Qué hacemos en caso de error?
				*/
			}else if(pfds->fds[0].revents & POLLIN) /* El socket de comunicación con listener es el primero por definición*/
			{
				/* Cuidado con POLLERR! */
				add_new_connection(pfds, sk_new_connection);
				ready_fds--;
			}
		}
		for(i = 1; i < fds_len && ready_fds > 0; i++)
		{
			if(pfds->fds[i].revents & POLLERR) /* algo malo ha pasado. Cerramos */
			{
				remove_connection(pfds, pfds->fds[i].fd); /* Cerramos conexión */	
				i--; /* Reexploramos este elemento */
			}
			else if(pfds->fds[i].revents & POLLIN) /* Datos en el socket */
			{
				ready_fds--;
				receive_parse_message(pfds->fds[i].fd,message);
				send_to_main(NULL,message);
			}
		}
	}
	return NULL;


}

int spawn_receiver_thread(pthread_t * recv_thread){

	pthread_create(recv_thread, NULL/*const pthread_attr_t *restrict __attr*/, thread_receive, NULL);
	return OK;
}

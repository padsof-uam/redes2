#include <sys/socket.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "receiver.h"
#include "types.h"

void* thread_receive(void * st){

	struct pollfds* pfds;

	int ready_fds, i, fds_len;
	int errnum = 0;

	int sk_new_connection;

/** De qué tipo tiene que ser????*/

	char * msg_recv = (char *) calloc(150, sizeof(char));

	char *message;
	struct receiver_thdata * recv_data = (struct receiver_thdata *) st;


	pfds = pollfds_init(POLLERR | POLLIN);
	pollfds_add(pfds,recv_data->socket);

	while(1)
	{
		ready_fds = pollfds_poll(pfds, 10); /* Esperamos 10 milisegundos antes de volver. */

		if(ready_fds == -1)
		{
			syslog(LOG_ERR, "Error %d ejecutando poll: %s", errnum, strerror(errno));
			errnum++;

			if(errnum >= MAX_ERR_THRESHOLD)
				break; /* Demasiados errores, salimos */
			else
				continue;
		}

		fds_len = pfds->len; /* Evitamos iterar por posibles nuevos descriptores */

		if (ready_fds){

		/* El socket de comunicación con listener es el primero por definición*/

			if (pfds->fds[0].revents & POLLERR){
		
				/* ¿Qué hacemos en caso de error? */
		
				syslog(LOG_ERR, "Error %d en la comunicación receiver-listener: %s",errnum,strerror(errno));
				errnum++;
				ready_fds--;
			}
			else if(pfds->fds[0].revents & POLLIN) 
			{
				recv(pfds->fds[0].fd, &msg_recv, sizeof(msg_recv), 0);
				sk_new_connection = atoi(msg_recv);
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
				send_to_main(recv_data->queue , pfds->fds[0].fd , message);
			}
		}
	} 

	return NULL;
}

int spawn_receiver_thread(pthread_t * recv_thread,int fd,int queue){
	struct receiver_thdata thdata;
	thdata.queue = queue;
	thdata.socket = fd;
	if (pthread_create(recv_thread, NULL, thread_receive, &thdata))
	{
		syslog(LOG_CRIT, "Error creando hilo receptor de mensajes: %s", strerror(errno));
		return -ERR;
	}
	else
	{
		syslog(LOG_INFO, "Hilo receptor de mensajes creado");
	}

	return OK;
}

int add_new_connection(struct pollfds* pfds, int listener_sk)
{
	return pollfds_add(pfds, listener_sk);
}
int remove_connection(struct pollfds* pfds, int fd)
{
	return pollfds_remove(pfds, fd);
}
int receive_parse_message(int fd,char * message)
{
	int size_read;
	message = (void *) calloc(MAX_IRC_MSG, sizeof(char));
	size_read = recv(fd, message,MAX_IRC_MSG,0);
	if ( size_read == -1 )
	{
		syslog(LOG_ERR, "Error al recibir mensaje de la conexión %d. -- %s",fd,strerror(errno));
		return -ERR;
	}
	syslog(LOG_INFO, "Leidos %d bytes de la conexión: %d",size_read,fd);
	return OK;
}
int send_to_main(int queue,int fd, char * message){

	struct sockcomm_data data_to_send;

	strcpy(data_to_send.data, message);
	data_to_send.fd=fd;
	data_to_send.len=strlen(message);

	/*msgsnd utiliza una estructura msgbuf o es un ejemplo de estructura a usar?*/
	if (! msgsnd(queue, &data_to_send, sizeof(data_to_send), 0)){
		if (errno == EIDRM)
			syslog(LOG_ERR,"La cola de comunicación con el hilo principal ha sido eliminada.");
		else
			syslog(LOG_WARNING, "No se ha podido enviar el mensaje al hilo principal.");
	}

	return OK;
}

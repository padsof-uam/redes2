#include "daemonize.h"
#include "errors.h"
#include "listener.h"
#include "poller.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define IRC_PORT 1500 /* Seguro? */
#define LOG_ID "redirc"
#define MAX_ERR_THRESHOLD 5

int main(int argc, char const *argv[])
{
	struct pollfds* pfds;
	int sk[2];
	int listener_commsock;
	pthread_t listener_th;
	int ready_fds, i, fds_len, todelete_fds;
	int errnum = 0;

	if(daemonize(LOG_ID) != OK)
	{
		syslog(LOG_CRIT, "No se ha podido daemonizar.");
		return EXIT_FAILURE;
	}

	if(socketpair(PF_LOCAL, SOCK_STREAM, 0, sk) < 0)
	{
		syslog(LOG_CRIT, "No se han podido crear los sockets de comunicación con listener: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	listener_commsock = sk[0];

	if(spawn_listener_thread(&listener_th, IRC_PORT, sk[1]) < 0)
	{
		syslog(LOG_CRIT, "No se ha podido crear el hilo de escucha: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	pfds = pollfds_init(POLLERR | POLLIN);
	pollfds_add(pfds, listener_commsock);

	while(1)
	{
		todelete_fds = 0;
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

		for(i = 0; i < fds_len && ready_fds > 0; i++)
		{
			if(pfds->fds[i].revents & POLLERR) /* algo malo ha pasado. Cerramos */
			{
				pfds->fds[i].events = -1; /* Marcamos para eliminación */	
			}
			else if(pfds->fds[i].revents & POLLIN) /* Datos en el socket */
			{
				ready_fds--;
				if(pfds->fds[i].fd == listener_commsock)
					add_new_connection(pfds, listener_commsock);
				else
					receive_parse_message(pfds->fds[i].fd);
			}
		}
	}

	return 0;
}


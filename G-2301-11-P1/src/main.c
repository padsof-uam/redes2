#include "daemonize.h"
#include "errors.h"
#include "listener.h"

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

int main(int argc, char const *argv[])
{
	int sk[2];
	int listener_commsock;
	pthread_t listener_th;
	fd_set fds;
	int nfds;

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

	if(spawn_listener_thread(&listener_th, IRC_PORT, sk[1], NULL) < 0)
	{
		syslog(LOG_CRIT, "No se ha podido crear el hilo de escucha: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	nfds = listener_commsock;

	while(1)
	{
		FD_ZERO(&fds);
		FD_SET(listener_commsock, &fds);

		/* set all fd for threads. */

		
	}

	return 0;
}


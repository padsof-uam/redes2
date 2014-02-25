#include "sender.h"
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include "poller.h"

int spawn_sender_thread (pthread_t * sender_thread){
	struct sender_thdata thdata;
	if (pthread_create(sender_thread, NULL, thread_send, &thdata))
	{
		syslog(LOG_CRIT, "Error creando hilo sender de mensajes: %s", strerror(errno));
		return -ERR;
	}
	else
	{
		syslog(LOG_INFO, "Hilo sender de mensajes creado");
	}
	return OK;
}

void * thread_send(void * st){
	return NULL;
}

int send_message(int fd, char * message){
	return OK;
}

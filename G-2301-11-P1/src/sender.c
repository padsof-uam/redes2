#include <sys/socket.h>
#include "sender.h"
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include "poller.h"
#include "types.h"
#include <unistd.h>

int spawn_sender_thread (pthread_t *sender_thread, int queue)
{
    struct sender_thdata* thdata = malloc(sizeof(struct sender_thdata));

    thdata->queue = queue;

    if (pthread_create(sender_thread, NULL, thread_send, thdata))
    {
        syslog(LOG_CRIT, "Error creando hilo sender de mensajes: %s", strerror(errno));
        return ERR;
    }
    else
    {
        syslog(LOG_INFO, "Hilo sender de mensajes creado");
    }
    return OK;
}

static void send_cleanup(void * data)
{
    syslog(LOG_DEBUG, "sender: limpiando");

    free(data);
}

void *thread_send(void *st)
{
    int retval, errnum = 0;
    struct sender_thdata *recv_data = (struct sender_thdata *) st;
    struct msg_sockcommdata tosend;
    char message[MAX_IRC_MSG];
    int size;

    pthread_cleanup_push(send_cleanup, recv_data);

    syslog(LOG_NOTICE, "Hilo de envío iniciado.");

    while (1)
    {
        retval = msgrcv(recv_data->queue, &tosend, sizeof(tosend), 0, 0);
        if (retval == -1)
        {
            syslog(LOG_ERR, "No se ha podido leer mensajes de la cola main-sender. %s", strerror(errno));
            errnum++;

            if (errnum >= MAX_ERR_THRESHOLD)
                break; /* Demasiados errores, salimos */
        }
        else
        {   
            size = tosend.scdata.len > MAX_IRC_MSG ? MAX_IRC_MSG : tosend.scdata.len;
            strncpy(message, tosend.scdata.data, size);
		    if(send_message(tosend.scdata.fd, message, size) < 0)
		    {
		    	/* Política para reintentar? */
		    	syslog(LOG_WARNING, "Error enviando mensaje: %s", strerror(errno));
		    }
        }
    }

    syslog(LOG_NOTICE, "Hilo de envío saliendo.");

    pthread_cleanup_pop(0);

    return NULL;
}


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
        return -ERR;
    }
    else
    {
        syslog(LOG_INFO, "Hilo sender de mensajes creado");
    }
    return OK;
}

void *thread_send(void *st)
{
    int retval, errnum = 0;
    struct sender_thdata *recv_data = (struct sender_thdata *) st;
    struct sockcomm_data tosend;
    char *message = (char *) calloc(MAX_IRC_MSG, sizeof(char));

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
            strncpy(message, tosend.data, tosend.len);
		    if(send_message(tosend.fd, message, tosend.len) < 0)
		    {
		    	/* Política para reintentar? */
		    	syslog(LOG_WARNING, "Error enviando mensaje: %s", strerror(errno));
		    }
        }
    }
    free(message);
    free(recv_data);

    syslog(LOG_NOTICE, "Hilo de envío saliendo.");

    return NULL;
}


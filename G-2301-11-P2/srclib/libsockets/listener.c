#include "listener.h"
#include "errors.h"
#include "messager.h"
#include "listener.h"
#include "log.h"
#include "sysutils.h"
#include "sockutils.h"
#include "sockssl.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/resource.h>

int spawn_listener_thread(pthread_t *pth, int port, int ssl_port, int commsocket)
{
    struct listener_thdata *thdata = malloc(sizeof(struct listener_thdata));
    thdata->commsocket = commsocket;
    thdata->port = port;
    thdata->ssl_port = ssl_port;

    if (pthread_create(pth, NULL, thread_listener, thdata))
    {
        slog(LOG_CRIT, "Error creando thread de escucha: %s", strerror(errno));
        return ERR;
    }
    else
    {
        slog(LOG_INFO, "Thread de escucha creado.");
        return OK;
    }
}

void listener_cleanup(void *data)
{
    struct listener_thdata *thdata = (struct listener_thdata *) data;

    server_close_communication(thdata->listen_sock);

    if (thdata->listen_sock_ssl != -1)
        server_close_communication(thdata->listen_sock_ssl);

    slog(LOG_NOTICE, "listener: limpiando.");

    free(thdata);
}

void *thread_listener(void *data)
{
    struct listener_thdata *thdata = (struct listener_thdata *) data;
    int listen_sock, listen_sock_ssl = -1;
    struct pollfd fds[3];
    int numfds = 2;
    char *commbuf;

    slog(LOG_NOTICE, "Hilo de escucha creado.");

    listen_sock = server_open_socket(thdata->port, DEFAULT_MAX_QUEUE, 0);

    if (thdata->ssl_port != 0)
        listen_sock_ssl = server_open_socket(thdata->ssl_port, DEFAULT_MAX_QUEUE, 1);

    thdata->listen_sock = listen_sock;
    thdata->listen_sock_ssl = listen_sock_ssl;

    pthread_cleanup_push(listener_cleanup, thdata);

    if (listen_sock < 0)
    {
        slog(LOG_CRIT, "Abortando: No se puede abrir un socket de escucha en %d con long. cola %d: %s.", thdata->port, DEFAULT_MAX_QUEUE, strerror(errno));
        thdata->listen_sock = -1;
        kill(getpid(), SIGTERM); /* Salimos */
        pthread_exit(NULL);
    }
    else
    {
        slog(LOG_NOTICE, "Puerto de escucha abierto en %d", thdata->port);
    }

    if (listen_sock_ssl < 0 && thdata->ssl_port != 0)
    {
        slog(LOG_CRIT, "Abortando: No se puede abrir un socket de escucha SSL en %d con long. cola %d: %s.", thdata->ssl_port, DEFAULT_MAX_QUEUE, strerror(errno));
        thdata->listen_sock_ssl = -1;
        raise(SIGTERM); /* Salimos */
        pthread_exit(NULL);
    }
    else if(listen_sock_ssl > 0)
    {
        slog(LOG_NOTICE, "Puerto de escucha SSL abierto en %d", thdata->ssl_port);
    }

    fds[0].events = POLLIN;
    fds[0].fd = listen_sock;

    fds[1].events = POLLIN;
    fds[1].fd = thdata->commsocket;

    if (listen_sock_ssl != -1)
    {
        fds[2].events = POLLIN;
        fds[2].fd = listen_sock_ssl;
        numfds++;
    }

    while (1)
    {
        /* Esperamos indefinidamente hasta que haya datos en algún socket */
        if (poll(fds, numfds, -1) < 0)
        {
            slog(LOG_NOTICE, "Error al llamar a poll: %s", strerror(errno));
        }
        else
        {
            if (fds[0].revents & POLLIN)
            {
                if (create_new_connection_thread(listen_sock, thdata->commsocket) != OK)
                    slog(LOG_ERR, "Fallo al aceptar una conexión. Error %s", strerror(errno));
            }

            if (listen_sock_ssl != -1 && (fds[2].revents & POLLIN))
            {
                slog(LOG_DEBUG, "Conexión pendiente en socket SSL.");
                if (create_new_connection_thread(listen_sock_ssl, thdata->commsocket) != OK)
                {
                    slog(LOG_ERR, "Fallo al aceptar una conexión en socket SSL. Error %s", strerror(errno));
                    slog_sslerr();
                }
            }

            if (fds[1].revents & POLLIN)
            {
                if (rcv_message(thdata->commsocket, (void **)&commbuf) > 0)
                {
                    if (commbuf[0] == COMM_STOP)
                        break;
                    else
                        slog(LOG_NOTICE, "Mensaje no reconocido en commsocket: %d", commbuf[0]);
                }

                free(commbuf);
            }
        }
    }

    slog(LOG_INFO, "Hilo listener saliendo.");

    pthread_cleanup_pop(0);

    pthread_exit(0);

}

int create_new_connection_thread(int listen_sock, int commsocket)
{
    int connsock;

    slog(LOG_NOTICE, "Nueva conexión recibida.");
    connsock = server_listen_connect(listen_sock);

    if (connsock < 0)
    {
        slog(LOG_WARNING, "No se ha podido aceptar la conexión: %s", strerror(errno));
        return ERR;
    }

    if (send_message(commsocket, &connsock, sizeof(int)) < 0)
    {
        slog(LOG_ERR, "No se ha podido transmitir la información al proceso principal (%s). Abortando...", strerror(errno));
        server_close_communication(connsock);
        return ERR;
    }

    slog(LOG_NOTICE, "Nueva conexión creada con éxito en socket %d.", connsock);
    return OK;
}

#include "listener.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "errors.h"
#include "listener.h"
#include <syslog.h>
#include "messager.h"
#include <fcntl.h>
#include <poll.h>

static int _server_close_socket(int handler)
{
    if (close(handler) < 0)
    {
        syslog(LOG_ERR, "Error cerrando el socket, %d", errno);
        return -ERR_SOCK;
    }
    return OK;
}

static int _server_open_socket()
{
    int handler = socket(AF_INET, SOCK_STREAM, TCP);
    if (handler == -1)
    {
        syslog(LOG_ERR, "Error en la creación de socket");
        return -ERR_SOCK;
    }
    return handler;
}

static int _link_socket_port(int port, int handler)
{
    struct sockaddr_in serv_addr;

    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;

    bzero( (void *) serv_addr.sin_zero, sizeof(serv_addr.sin_zero));

    if (bind(handler, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in) ) == -1)
    {
        syslog(LOG_ERR, "Error asociando puerto con socket: %s", strerror(errno));
        return -ERR_SOCK;
    }

    return OK;
}

static int _set_queue_socket(int handler, int long_max)
{

    if (listen(handler, long_max) == -1)
    {
        syslog(LOG_ERR, "Error al poner a escuchar: errno=%d", errno);
        return -ERR_SOCK;
    }
    return OK;
}


int server_open_socket(int port, int max_long)
{

    int handler = _server_open_socket();
    if (handler == -ERR_SOCK)
    {
        syslog(LOG_ERR, "Error al abrir el socket");
        return -ERR_SOCK;
    }

    _link_socket_port(port, handler);

    if ( _set_queue_socket(handler, max_long) != OK)
        return -ERR_SOCK;

    fcntl(handler, F_SETFL, O_NONBLOCK);
    /*
    Código de error: Valor negativo -> ERROR; sino, devuelve el handler del socket creado.
    */
    return handler;
}

int server_listen_connect(int handler)
{
    struct sockaddr peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    int handler_accepted;

    handler_accepted = accept(handler, &peer_addr, &peer_len);

    if ( handler_accepted == -1)
    {
        syslog(LOG_ERR, "Error aceptando conexiones, %d", errno);
        return -ERR_SOCK;
    }

    /*
    - Qué hacemos con handler_accepted¿? Es el socket de la nueva conexión. El enunciado dice devolver código de error
    - ¿Cómo gestionar más d euna conexión?
    */

    return handler_accepted;

}

int server_close_communication(int handler)
{
    return _server_close_socket(handler);
}

int spawn_listener_thread(pthread_t *pth, int port, int commsocket)
{
    struct listener_thdata* thdata = malloc(sizeof(struct listener_thdata));
    thdata->commsocket = commsocket;
    thdata->port = port;

    if (pthread_create(pth, NULL, thread_listener, thdata))
    {
        syslog(LOG_CRIT, "Error creando thread de escucha: %s", strerror(errno));
        return -ERR;
    }
    else
    {
        syslog(LOG_INFO, "Thread de escucha creado.");
        return OK;
    }
}

void listener_cleanup(void* data)
{
    struct listener_thdata *thdata = (struct listener_thdata *) data;
    
    syslog(LOG_NOTICE, "listener: limpiando.");
    server_close_communication(thdata->listen_sock);

    free(thdata);
}

void *thread_listener(void *data)
{
    struct listener_thdata *thdata = (struct listener_thdata *) data;
    int listen_sock;
    struct pollfd fds[2];
    char *commbuf;

    syslog(LOG_NOTICE, "Hilo de escucha creado.");
    listen_sock = server_open_socket(thdata->port, DEFAULT_MAX_QUEUE);
    thdata->listen_sock = listen_sock;
    pthread_cleanup_push(listener_cleanup, thdata);

    if (listen_sock < 0)
    {
        syslog(LOG_CRIT, "No se puede abrir un socket de escucha en %d con long. cola %d: %s", thdata->port, DEFAULT_MAX_QUEUE, strerror(errno));
        pthread_exit(NULL);
    }
    else
    {
        syslog(LOG_NOTICE, "Puerto de escucha abierto en %d", thdata->port);
    }

    fds[0].events = POLLIN;
    fds[0].fd = listen_sock;

	fds[1].events = POLLIN;
    fds[1].fd = thdata->commsocket;
    
    while (1)
    {
        /* Esperamos indefinidamente hasta que haya datos en algún socket */
        if (poll(fds, 2, -1) < 0)
        {
            syslog(LOG_NOTICE, "Error al llamar a poll: %s", strerror(errno));
        }
        else
        {
            if (fds[0].revents & POLLIN)
            {
                if (create_new_connection_thread(listen_sock, thdata->commsocket) != OK)
                    syslog(LOG_ERR, "fallo al aceptar una conexión. Error %s", strerror(errno));
            }

            if (fds[1].revents & POLLIN)
            {
                if (rcv_message(thdata->commsocket, (void **)&commbuf) > 0)
                {
                    if (commbuf[0] == COMM_STOP)
                        break;
                    else
                        syslog(LOG_NOTICE, "Mensaje no reconocido en commsocket: %d", commbuf[0]);
                }

                free(commbuf);
            }
        }
    }

    syslog(LOG_INFO, "Hilo listener saliendo.");

    pthread_cleanup_pop(0);

    pthread_exit(0);
}

int create_new_connection_thread(int listen_sock, int commsocket)
{
    int connsock;

    syslog(LOG_NOTICE, "Nueva conexión recibida.");
    connsock = server_listen_connect(listen_sock);

    if (connsock < 0)
    {
        syslog(LOG_WARNING, "No se ha podido aceptar la conexión: %s", strerror(errno));
        return -ERR;
    }

    if (send_message(commsocket, &connsock, sizeof(int)) < 0)
    {
        syslog(LOG_ERR, "No se ha podido transmitir la información al proceso principal (%s). Abortando...", strerror(errno));
        server_close_communication(connsock);
        return -ERR;
    }

    syslog(LOG_NOTICE, "Nueva conexión creada con éxito.");
    return OK;
}

#include <sys/socket.h>
#include "log.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "receiver.h"
#include "types.h"
#include "messager.h"
#include "listener.h"
#include "sysutils.h"

static void receive_cleanup(void *data)
{
    slog(LOG_DEBUG, "receiver: limpiando.");

    free(data);
}

void *thread_receive(void *st)
{
    struct pollfds *pfds;
    int msglen;
    int ready_fds, i, fds_len;
    int errnum = 0;
    int sk_new_connection;
    char *buffer;
    char *message;
    int sock_lim_sc = -1, sock_lim_rlimit = -1, sock_lim = -1;
    struct receiver_thdata *recv_data = (struct receiver_thdata *) st;

    sock_lim_sc = sysconf(_SC_OPEN_MAX);   
    sock_lim_rlimit = get_soft_limit(RLIMIT_NOFILE);
    sock_lim = sock_lim_sc < sock_lim_rlimit ? sock_lim_sc : sock_lim_rlimit;

    if (sock_lim == -1)
        slog(LOG_WARNING, "No se ha podido obtener el límite de descriptores para el proceso: %s", strerror(errno));
    else
        slog(LOG_NOTICE, "Límite de descriptores obtenido: %d.", sock_lim);

    pfds = pollfds_init(POLLIN);
    pollfds_add(pfds, recv_data->socket);

    pthread_cleanup_push((void(*)(void *))pollfds_destroy, pfds);
    pthread_cleanup_push(receive_cleanup, recv_data);

    slog(LOG_DEBUG, "Hilo de recepción iniciado.");

    while (1)
    {
        ready_fds = pollfds_poll(pfds, -1);

        if (ready_fds == -1)
        {
            slog(LOG_ERR, "Error %d ejecutando poll: %s", errnum, strerror(errno));
            errnum++;

            if (errnum >= MAX_ERR_THRESHOLD)
                break; /* Demasiados errores, salimos */
            else
                continue;
        }

        fds_len = pfds->len; /* Evitamos iterar por posibles nuevos descriptores */

        if (ready_fds)
        {
            /* El socket de comunicación con listener es el primero por definición*/
            if (pfds->fds[0].revents & POLLERR)
            {
                /* ¿Qué hacemos en caso de error? */
                slog(LOG_ERR, "Error %d en la comunicación receiver-listener: %s", errnum, strerror(errno));
                errnum++;
                ready_fds--;
            }
            else if (pfds->fds[0].revents & POLLIN)
            {
                if (rcv_message(pfds->fds[0].fd, (void **)&buffer) < 0)
                {
                    slog(LOG_ERR, "Error recibiendo mensaje a través de commsock: %s", strerror(errno));
                }
                else
                {
                    sk_new_connection = *((int *)buffer);
                    add_new_connection(pfds, sk_new_connection);
                    free(buffer);
                }
                ready_fds--;
            }
        }

        for (i = 1; i < fds_len && ready_fds > 0; i++)
        {
            if (pfds->fds[i].revents & POLLERR) /* algo malo ha pasado. Cerramos */
            {
                slog(LOG_WARNING, "Error en conexión %d. Cerrando.", pfds->fds[i].fd);
                remove_connection(pfds, pfds->fds[i].fd); /* Cerramos conexión */
                i--; /* Reexploramos este elemento */
                fds_len--;
            }
            else if ((pfds->fds[i].revents & POLLHUP) || (pfds->fds[i].revents & POLLNVAL))
            {
                slog(LOG_NOTICE, "El socket %d ha sido cerrado.", pfds->fds[i].fd);
                remove_connection(pfds, pfds->fds[i].fd); /* Cerramos conexión */
                i--; /* Reexploramos este elemento */
                fds_len--;
            }
            else if (pfds->fds[i].revents & POLLIN) /* Datos en el socket */
            {
                ready_fds--;
                msglen = rcv_message(pfds->fds[i].fd, (void **)&message);

                if (msglen > 0)
                    send_to_main(recv_data->queue, pfds->fds[i].fd, message, msglen);
                else
                {
                    slog(LOG_WARNING, "Error en recepción de datos en socket %d: %s", pfds->fds[i].fd, strerror(errno));
                    remove_connection(pfds, pfds->fds[i].fd); /* Cerramos conexión */
                    i--; /* Reexploramos este elemento */
                    fds_len--;
                }
                if (message)
                    free(message);
            }
        }
    }

    slog(LOG_DEBUG, "Hilo de recepción saliendo.");

    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);

    return NULL;
}

int spawn_receiver_thread(pthread_t *recv_thread, int commsock, int queue)
{
    struct receiver_thdata *thdata = malloc(sizeof(struct receiver_thdata));

    thdata->queue = queue;
    thdata->socket = commsock;

    if (pthread_create(recv_thread, NULL, thread_receive, thdata))
    {
        slog(LOG_CRIT, "Error creando hilo receptor de mensajes: %s", strerror(errno));
        return ERR;
    }
    else
    {
        slog(LOG_INFO, "Hilo receptor de mensajes creado");
    }

    return OK;
}

int add_new_connection(struct pollfds *pfds, int listener_sk)
{
    return pollfds_add(pfds, listener_sk);
}

int remove_connection(struct pollfds *pfds, int fd)
{
    return pollfds_remove(pfds, fd);
}

int send_to_main(int queue, int fd, char *message, int msglen)
{
    struct msg_sockcommdata data_to_send;
    bzero(&data_to_send, sizeof(struct msg_sockcommdata));

    data_to_send.msgtype = 1;
    strncpy(data_to_send.scdata.data, message, MAX_IRC_MSG);
    data_to_send.scdata.fd = fd;
    data_to_send.scdata.len = msglen;

    if (msgsnd(queue, &data_to_send, sizeof(struct sockcomm_data), 0) == -1)
    {
        if (errno == EIDRM)
            slog(LOG_ERR, "La cola de comunicación con el hilo principal ha sido eliminada.");
        else
            slog(LOG_WARNING, "No se ha podido enviar el mensaje al hilo principal: %s", strerror(errno));

        return ERR;
    }

    return OK;
}

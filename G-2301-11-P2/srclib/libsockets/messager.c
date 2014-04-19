#include "messager.h"
#include "errors.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include "log.h"

#define INITIAL_RECEIVE_BUFFER 1024

int sock_data_available(int socket)
{
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET(socket, &set);

    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;
    /* Esperamos 1 ms para permitir al otro lado de la conexión
        enviar datos si tenía su buffer lleno. Importante para tests. */

    return select(socket + 1, &set, NULL, NULL, &timeout);
}

int send_message(int socket, const void *msg, ssize_t len)
{
    int sent = 0;
    int batch_sent;
    const char *buf = (const char *) msg;

    while (sent < len)
    {
        batch_sent = send(socket, buf + sent, len - sent, 0);

        if (batch_sent == -1)
            return ERR_SOCK;

        sent += batch_sent;
    }

    return OK;
}

/* Posible bug: ¿y si no dejamos de recibir mensajes? */
static ssize_t _rcv_message(int socket, void **buffer, int static_buf, ssize_t buflen)
{
    char *internal_buf = NULL;
    char *realloc_retval;
    size_t buf_size = static_buf ? buflen : INITIAL_RECEIVE_BUFFER;
    ssize_t read_bytes = 0;
    ssize_t batch_bytes = 0;

    if (static_buf)
        internal_buf = *buffer;
    else
        internal_buf = (char *) calloc(buf_size, sizeof(char));

    if (!internal_buf)
        return ERR_MEM;

    while (sock_data_available(socket))
    {
        batch_bytes = recv(socket, internal_buf + read_bytes, buf_size - read_bytes, 0);

        if (batch_bytes == buf_size - read_bytes) /* Hemos leído todo lo que podemos leer. */
        {
            if (static_buf)
            {
                return buflen; /* Si el buffer es estático, paramos. */
            }
            else
            {
                /* Si no, reservamos más memoria */
                buf_size *= 2;
                realloc_retval = realloc(internal_buf, buf_size * sizeof(char)); /* Liberamos memoria en caso de error con reallocf */

                if (!realloc_retval)
                {
                    free(internal_buf);
                    return ERR_MEM;
                }
                else
                {
                    internal_buf = realloc_retval;
                }
            }
        }
        else if (batch_bytes == 0)
        {
            return 0;
        }
        else if (batch_bytes < 0)
        {
            return -1;
        }

        read_bytes += batch_bytes;
    }

    if(!static_buf) 
        *buffer = internal_buf;

    return read_bytes;
}

ssize_t rcv_message(int socket, void** buffer)
{
    return _rcv_message(socket, buffer, 0, 0);
}

ssize_t rcv_message_staticbuf(int socket, void *buffer, ssize_t buflen)
{
    return _rcv_message(socket, &buffer, 1, buflen);
}

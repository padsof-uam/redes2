#include <stdlib.h>
#include <sys/socket.h>

#ifndef MESSAGER_H
#define MESSAGER_H

/**
 * Llama a select(2) para devolver si hay datos disponibles para
 * 	leer en un socket.
 * @param  socket Socket.
 * @return        0 si no hay datos, un valor mayor que cero si los hay.
 */
int sock_data_available(int socket);

/**
 * Envía un mensaje a través de un socket.
 * @param  socket Socket.
 * @param  msg    Mensaje.
 * @param  len    Longitud del mensaje.
 * @return        OK o <0 si ha habido algún error.
 */
int send_message(int socket, const void* msg, ssize_t len);

/**
 * Recibe un mensaje de un socket reservando automáticamente
 * 	el buffer necesario.
 * @param  socket Socket.
 * @param  buffer Puntero al buffer.
 * @return        Bytes recibidos.
 */
ssize_t rcv_message(int socket, void** buffer);


ssize_t rcv_message_staticbuf(int socket, void *buffer, ssize_t buflen);
#endif


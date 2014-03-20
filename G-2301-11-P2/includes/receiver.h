#ifndef RECEIVER_H
#define RECEIVER_H

#include "errors.h"
#include <pthread.h>
#include "poller.h"
#include "types.h"

#define MAX_ERR_THRESHOLD 5

/**
 * Estructura que contiene los datos necesarios para el funcionamiento d
 * 	del hilo de recepción.
 */
struct receiver_thdata
{
    int socket; /**< Socket de comunicaciones con el hilo de recepción de conexiones */
    int queue; /**< ID de la cola por donde se enviarán los mensajes */
};

/**
 * @internal
 * Punto de entrada para el hilo receptor.
 * @param    st  La información necesaria, en forma de receiver_thdata
 * @return   NULL en todo caso.
 * @see receiver_thdata
 */
void *thread_receive(void *st);

/**
* Crea un hilo que se encarga de gestionar la recepción de comunicaciones.
* @param    recv_thread     Identificador del hilo creado.
* @param    commsock        Socket de comunicaciones con el hilo de recepción de
*                               conexiones.
* @param    queue           ID de la cola de comunicaciones con el hilo de procesado
*                           	de mensajes. Los mensajes enviados a través de esta cola
*                           	serán del tipo msg_sockcommdata
* @see 	sockcomm_data
* @see  msg_sockcommdata
* @return                   Código de error si algo falló.
*/
int spawn_receiver_thread(pthread_t *recv_thread, int commsock, int queue);

/**
* Añade una nueva conexión a la estructura pollfds dinámica.
* @param    pfds        La estructura a la que añadir la conexión.
* @param    listener_sk El socket listo para ser leido con la información de la nueva conexión.
* @return               Código de error.
*/
int add_new_connection(struct pollfds *pfds, int listener_sk);

/**
* Elimina una conexión de la estructura pollfds.
* @param    pfds    La estructura a la que eliminar la conexión.
* @param    fd      El identificador de la conexión para ser cerrada.
* @return           Código de error.
*/
int remove_connection(struct pollfds *pfds, int fd);

/**
* Recibe y lee los mensajes de una conexión.
* @param    fd          El socket listo para ser leido.
* @param    message     Cadena a rellenar con la información leída (puntero no inicializado).
* @return               Código de error.
*/
int receive_parse_message(int fd, char *message);

/**
 * Envía un paquete recibido al hilo de procesado.
 * @param  queue   Cola por donde se enviará el mensaje.
 * @param  fd      Socket que ha generado el mensaje.
 * @param  message Cadena con el mensaje.
 * @param  msglen  Longitud del mensaje
 * @return         Código de error.
 */
int send_to_main(int queue, int fd, char *message, int msglen);

#endif

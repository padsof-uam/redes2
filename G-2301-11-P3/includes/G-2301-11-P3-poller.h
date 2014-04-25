#ifndef POLLER_H
#define POLLER_H

#include <poll.h>
#include "G-2301-11-P3-errors.h"

#define DEFAULT_PFDS_LEN 300 /**< Longitud inicial del array de descriptores. */

/**
 * Estructura que contiene un conjunto dinámico de descriptores 
 * 	de fichero sobre los que hacer poll.
 * 	@see poll(2)
 */
struct pollfds {
	struct pollfd* fds; /**< Lista de pollfds. */
	int len; /**< Número de descriptores de fichero almacenados */
	int capacity; /**< Capacidad de la estructura */
	short flags; /**< Flags por defecto para nuevos descriptores */
};

/**
 * Inicializa la estructura pollfds.
 * @param default_flags Flags por defecto para los nuevos pollfd
 * @return Estructura pollfds creada.
 */
struct pollfds* pollfds_init(short default_flags);

/**
 * @internal
 * Aumenta la capacidad de la estructura.
 * @param  pfds     Estructura.
 * @param  capacity Nueva capacidad.
 * @return Código de error
 */
int pollfds_setcapacity(struct pollfds* pfds, int capacity);

/**
 * Añade un descriptor.
 * @param  pfds Estructura.
 * @param  fd   Descriptor de fichero.
 * @return      Código OK/ERR
 */
int pollfds_add(struct pollfds* pfds, int fd);

/**
 * Elimina un descriptor.
 * @param  pfds Estructura
 * @param  fd   Descriptor a eliminar.
 * @return      Código OK/ERR:
 */
int pollfds_remove(struct pollfds* pfds, int fd);

/**
 * Ejecuta la llamada a poll con los descriptores almacenados.
 * @see poll(2)
 * @param  pfds    Estructura
 * @param  timeout Timeout tal y como está definido para poll(2). Si es mayor que cero,
 *                 	espera ese número de milisegundos. Si es cero, vuelve sin bloquear y
 *                 	si es -1 espera indefinidamente.
 * @return         Número de descriptores listos para I/O, el mismo código devuelto por poll(2).
 */
int pollfds_poll(struct pollfds* pfds, int timeout);

/**
 * Destruye la estructura y libera los recursos.
 * @param pfds Estructura.
 */
void pollfds_destroy(struct pollfds* pfds);

#endif

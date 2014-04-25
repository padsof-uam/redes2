#ifndef IRC_FTP_H
#define IRC_FTP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"

typedef void (*ftp_callback)(ftp_status);

/**
 * Estructura para la creación de hilos manejadores de ftp.
 */
 struct th_ftpdata
 {
 	FILE * f;
 	int sock;
 	ftp_callback cb;
 	int size;
 	struct ftp_connection * data;
 };


/**
 * Espera la recepción de un archivo.
 * @param  dest        Ruta de destino.
 * @param  port 	   Puntero donde guardar el puerto de escucha.
 * @param  cb          Función que se llamará cuando la transferencia cambie de estado.
 * @param  recv_ftp_th Identificador del hilo que se creará para la recepción.
 * @return             OK o ERR.
 */
int ftp_wait_file(const char* dest, int* port, ftp_callback cb, pthread_t * recv_ftp_th);

/**
 * Envía un fichero.
 * @param  file 		Fichero a enviar.
 * @param  ip 			IP de destino.
 * @param  port 		Puerto destino
 * @param  cb   		Función que se llamará cuando la transferencia cambie de estado.
 * @param  snd_ftp_th	Identificador del hilo que se creará para la recepción.
 * @return      OK/ERR.
 */
int ftp_send_file(const char* file, uint32_t ip, int port, ftp_callback cb, pthread_t * snd_ftp_th);

/**
 * Carga la petición de envío de un archivo en las estructuras de datos,
 * 	devolviendo OK si se puede realizar.
 * @param  conn Datos.
 * @param  file Archivo.
 * @return      OK/ERR/ERR_NOTFOUND si no se ha encontrado el archivo.
 */
int irc_ftp_reqsend(struct ftp_connection* conn, const char* file);

/**
 * Acepta el envío de un fichero, devolviendo si se puede aceptar.
 * @param  conn Datos.
 * @param  file Archivo donde se guardará. Si es nulo, se usará el nombre que envió el usuario remoto.
 * @param  port Dirección donde guardar el puerto de escucha.
 * @param  cb   Callback que se llamará cuando cambie el estado de la conexión.
 * @return      OK/ERR/ERR_NOTFOUND.
 */
int irc_ftp_accept(struct ftp_connection* conn, const char* file, int* port, ftp_callback cb);

/**
 * Recibe un accept del usuario remoto e inicia la transferencia, si se puede.
 * @param  conn Datos de conexión.
 * @param  ip   IP
 * @param  port Puerto
 * @param  cb   Callback
 * @return      OK/ERR.
 */
int irc_ftp_recvaccept(struct ftp_connection* conn, const char* ip, const char* port, ftp_callback cb);

/**
 * Guarda la recepción de un send desde un cliente remoto y
 * 	devuelve si es válido.
 * @param  conn Estructura de datos FTP.
 * @param  file Nombre del archivo.
 * @return      OK si todo ha ido bien, ERR si no se puede aceptar la petición.
 */
int irc_ftp_recvsend(struct ftp_connection* conn, const char* file);

/**
 * Cancela una petición de envío o recepción en curso.
 * @param  conn Datos FTP.
 * @return      OK/ERR.s
 */
int irc_ftp_cancel(struct ftp_connection* conn);

#endif

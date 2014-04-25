#ifndef IRC_FTP_H
#define IRC_FTP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
/**
* Longitud máxima de los tamaños ftp.
*/
#define MAX_LEN_FTP 10240
#define FTP_TIMEOUT_MS 700

typedef void (*ftp_callback)(ftp_status);

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
int ftp_send_file(const char* file, uint32_t ip, int port, ftp_callback cb,	pthread_t * snd_ftp_th);


#endif

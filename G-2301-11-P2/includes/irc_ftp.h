#ifndef IRC_FTP_H
#define IRC_FTP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
* Longitud máxima de los tamaños ftp.
*/
#define MAX_LEN_FTP 1024
#define FTP_PORT 6668

typedef enum {
	ftp_finished, ftp_aborted, ftp_timeout, ftp_started
} ftp_status;

typedef void (*ftp_callback)(ftp_status);

struct th_ftpdata
{
	FILE * f;
	int listen_port;
	ftp_callback cb;

} ;


/**
 * Espera la recepción de un archivo.
 * @param  dest        Ruta de destino.
 * @param  listen_port Puntero donde guardar el puerto de escucha.
 * @param  cb          Función que se llamará cuando la transferencia cambie de estado.
 * @return             OK o ERR.
 */
int ftp_wait_file(const char* dest, int* listen_port, ftp_callback cb);

/**
 * Envía un fichero.
 * @param  file Fichero a enviar.
 * @param  ip 	IP de destino.
 * @param  port Puerto destino
 * @param  cb   Función que se llamará cuando la transferencia cambie de estado.
 * @return      OK/ERR.
 */
int ftp_send_file(const char* file, uint32_t ip, int port, ftp_callback cb);


#endif

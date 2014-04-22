#include "irc_ftp.h"
#include "log.h"
#include "messager.h"
#include "sockutils.h"
#include "sysutils.h"

#include <errno.h>
#include <errors.h>
#include <string.h>



/**
- Tiene que abrir un puerto de escucha 
e iniciar un hilo dispuesto a escuchar y recibir el fichero.

- Controlar timeout de tiempo de espera.

- 
*/

void * thread_wait_file(void * ftpdata){
	struct th_ftpdata * data = ftpdata;
	char * buffer;
	int recv_bytes;


	do{
		recv_bytes = rcv_message(data->listen_port, (void **) &buffer);
		if (recv_bytes < 0){
			slog(LOG_ERR, "Error recibiendo mensaje");
			data->cb(ftp_aborted);
			break;
		}

		fputs(buffer, data->f);

	/* Controlar timeout*/
	} while(recv_bytes != 0 );

/* Recepción completada*/
	data->cb(ftp_finished);

	slog(LOG_DEBUG, "Hilo de recepción de fichero saliendo");

    pthread_exit(0);

}


int ftp_send_file(const char* file, uint32_t ip, int port, ftp_callback cb)
{
	return OK;
}


int ftp_wait_file(const char* dest, int* listen_port, ftp_callback cb)
{
	FILE * f = fopen(dest,"w");
	struct th_ftpdata * th_data = malloc(sizeof(struct th_ftpdata));
	pthread_t recv_ftp_th = 0;

	if (f == NULL)
	{
		slog(LOG_ERR, "Error abriendo el fichero: %s (para la recepción de datos)",dest);
		return ERR_SYS;
	}
	slog(LOG_INFO, "Fichero %s abierto para escribir",dest);

	th_data->f = f;
	th_data->cb = cb;

	/**Abrimos el puerto y aceptamos la conexión en el puerto correspondiente
	y rellenamos listen_port con e*/

	th_data->listen_port = server_listen_connect(server_open_socket(FTP_PORT,MAX_LEN_FTP));

	*listen_port = th_data->listen_port;


    if (pthread_create(&recv_ftp_th, NULL, thread_wait_file, th_data))
    {
        slog(LOG_CRIT, "Error creando thread de recepción ftp en el puerto %d: %s", th_data->listen_port, strerror(errno));
        return ERR;
    }
    else
    {
        slog(LOG_INFO, "Thread de recepción ftp creado.");
        cb(ftp_started);
        return OK;
    }
}

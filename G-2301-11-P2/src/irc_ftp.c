#include "irc_ftp.h"
#include "log.h"
#include "messager.h"
#include "sockutils.h"
#include "sysutils.h"

#include <errno.h>
#include <errors.h>
#include <string.h>
#include <netinet/in.h>



/** - Tiene que abrir un puerto de escucha 
e iniciar un hilo dispuesto a escuchar y recibir el fichero.*/

void * thread_wait_file(void * ftpdata){
	struct th_ftpdata * data = ftpdata;
	char * buffer = NULL;
	int recv_bytes = 1;
	unsigned long counter_rcv_bytes = 0;
	unsigned long * size = NULL;

	pthread_cleanup_push(free, buffer);
	pthread_cleanup_push(free, data);

	if(rcv_message_staticbuf(data->sock, (void ** )&size,sizeof(unsigned long)) < 0)
	{
		slog(LOG_ERR, "Error recibiendo el primer mensaje de recepción ftp.");
		return NULL;
	}

	if (*size <= 0)
	{
		slog(LOG_ERR, "Error en el tamaño del mensaje recibido");
		return NULL;
	}
	
	data->cb(ftp_started);

	while(recv_bytes > 0 && counter_rcv_bytes <= *size){
		recv_bytes = rcv_message(data->sock, (void **) &buffer);
		if (recv_bytes < 0){
			slog(LOG_ERR, "Error recibiendo mensaje");
			data->cb(ftp_aborted);
			break;
		}

		if (fwrite(buffer, 1, MAX_LEN_FTP, data->f)<= 0 ){
			slog(LOG_ERR, "Error escribiendo en el fichero");
		}
		counter_rcv_bytes+=recv_bytes;

	/* Controlar timeout*/
		/*if (timeout)
			data->cb(ftp_timeout);*/
	} 

	/* Recepción completada*/
	if (recv_bytes >= 0 /*&& !timeout*/ )
		data->cb(ftp_finished);

	slog(LOG_DEBUG, "Hilo de recepción de fichero saliendo");

    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    pthread_exit(0);
}


void * thread_send_file(void * ftpdata){
	struct th_ftpdata * data = ftpdata;
	char buffer;
	int snd_bytes;

	snd_bytes = fread(&buffer, 1, MAX_LEN_FTP - 1, data->f);
	if (snd_bytes < 0)
	{

	}


    pthread_exit(0);

}


int ftp_send_file(const char* file, uint32_t ip, int port, ftp_callback cb,	pthread_t * snd_ftp_th)
{
	struct th_ftpdata * th_data = malloc(sizeof(struct th_ftpdata));
	struct sockaddr_in dst_addr;
    int sock;
    unsigned long size;
	
	FILE * f = fopen(file, "r");
   
	if (f == NULL)
	{
		slog(LOG_ERR, "Error abriendo el fichero: %s (para en envío de datos ftp)",file);
		return ERR_SYS;
	}
	slog(LOG_INFO, "Fichero %s abierto para leer y enviar",file);



    bzero(&dst_addr, sizeof dst_addr);
    dst_addr.sin_addr.s_addr = ip;
    dst_addr.sin_port = port;
    dst_addr.sin_family = PF_INET;

    sock = open_listen_tcp_socket();
    if (sock == ERR_SOCK)
    {
        slog(LOG_CRIT, "Error creando socket de envío: %s", strerror(errno));
        return ERR_SOCK;
    }

    if (connect(sock, (struct sockaddr *)&dst_addr, sizeof dst_addr) == -1)
    {
        slog(LOG_CRIT, "Error asociando socket al otro extremo de la conexión: %s", strerror(errno));
        return ERR_SOCK;
    }

    /*Envío un primer mensaje con el tamaño.*/
	fseek(f, 0L, SEEK_END);
	size = ftell(f);
	fseek(f, 0L, SEEK_SET);
    th_data->size = size;

	th_data->f = f;
	th_data->cb = cb;
    th_data->sock = sock;

    if (send_message(sock, &size, sizeof(unsigned long)) < 0){
    	slog(LOG_ERR, "Error enviando el tamaño del fichero ftp: %s",strerror(errno));
    	return ERR;
    }

    if (pthread_create(snd_ftp_th, NULL, thread_send_file, th_data))
    {
        slog(LOG_CRIT, "Error creando thread de envío ftp en el puerto %d: %s", th_data->sock, strerror(errno));
        return ERR_SYS;
    }
    else
    {
        slog(LOG_INFO, "Thread de envío ftp creado.");
        cb(ftp_started);
        return OK;
    }

	return OK;
}


int ftp_wait_file(const char* dest, int* port, ftp_callback cb, pthread_t * recv_ftp_th)
{
	FILE * f = fopen(dest,"w");
	struct th_ftpdata * th_data = malloc(sizeof(struct th_ftpdata));

	if (f == NULL)
	{
		slog(LOG_ERR, "Error abriendo el fichero: %s (para la recepción de datos)",dest);
		return ERR_SYS;
	}
	slog(LOG_INFO, "Fichero %s abierto para escribir",dest);

	th_data->f = f;
	th_data->cb = cb;

	th_data->sock = server_open_socket(FTP_RCV_PORT,MAX_LEN_FTP);
	server_listen_connect(th_data->sock);

	get_socket_port(th_data->sock,port);

    if (pthread_create(recv_ftp_th, NULL, thread_wait_file, th_data))
    {
        slog(LOG_CRIT, "Error creando thread de recepción ftp en el puerto %d: %s", th_data->sock, strerror(errno));
        return ERR;
    }
    else
    {
        slog(LOG_INFO, "Thread de recepción ftp creado.");
        return OK;
    }
}

#include "irc_ftp.h"
#include "log.h"
#include "messager.h"
#include "sockutils.h"
#include "sysutils.h"
#include "types.h"
#include "ssltrans.h"

#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>


void * thread_wait_file(void * ftpdata){
	struct th_ftpdata * data = ftpdata;
	char * buffer = NULL;
	int recv_bytes = 1;
	unsigned long counter_rcv_bytes = 0;
	long * size = malloc(sizeof(unsigned long));

	pthread_cleanup_push(free, buffer);
	pthread_cleanup_push(free, data);

	int sock = server_listen_connect(data->sock);
	
	if(sock < 0)
	{
		slog(LOG_ERR, "Error aceptando conexión.");
		data->cb(ftp_aborted);
		return NULL;
	}

	sock_set_block(sock, 1);

	slog(LOG_DEBUG, "Conexión ftp aceptada");
	
	data->cb(ftp_started);	

	if (!sock_wait_data(sock, FTP_TIMEOUT_MS))
	{
		data->cb(ftp_timeout);
		return NULL;
	}

	if(rcv_message_staticbuf(sock, size,sizeof(long)) < 0)
	{
		slog(LOG_ERR, "Error recibiendo el primer mensaje de recepción ftp.");
		return NULL;
	}

	if (*size < 0)
	{
		slog(LOG_ERR, "Error en el tamaño del mensaje recibido");
		return NULL;
	}
	
	while(recv_bytes > 0 && counter_rcv_bytes < *size){
		if (!sock_wait_data(sock,FTP_TIMEOUT_MS))
		{
			data->cb(ftp_timeout);
			return NULL;
		}
		recv_bytes = rcv_message(sock, (void **) &buffer);
		if (recv_bytes < 0){
			slog(LOG_ERR, "Error recibiendo mensaje");
			break;
		}

		if (recv_bytes > 0 && fwrite(buffer, sizeof(char), recv_bytes, data->f) == 0 ){
			slog(LOG_ERR, "Error escribiendo en el fichero");
		}
		counter_rcv_bytes+=recv_bytes;
		free(buffer);
	} 


	/* Recepción completada*/
	
	if (counter_rcv_bytes == *size )
		data->cb(ftp_finished);
	else
		data->cb(ftp_aborted);

	slog(LOG_DEBUG, "Hilo de recepción de fichero saliendo");

	fclose(data->f);
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	pthread_exit(0);
}


int ftp_wait_file(const char* dest, int* port, ftp_callback cb, pthread_t * recv_ftp_th)
{
	FILE * f = fopen(dest,"w");
	int sock;
	struct th_ftpdata * th_data = malloc(sizeof(struct th_ftpdata));

	if (f == NULL)
	{
		slog(LOG_ERR, "Error abriendo el fichero: %s (para la recepción de datos)",dest);
		return ERR_SYS;
	}
	slog(LOG_DEBUG, "Fichero %s abierto para escribir",dest);

	th_data->f = f;
	th_data->cb = cb;

	sock = server_open_socket(0, MAX_LEN_FTP, 1);

	if (sock < 0)
	{
		slog(LOG_ERR,"Error abriendo socket tcp para ftp");
		return ERR_SOCK;
	}

	sock_set_block(sock, 1);

	th_data->sock = sock;

	if(get_socket_port(sock,port) != OK){
		slog(LOG_ERR,"Error obteniendo puerto del socket tcp");
		return ERR_SOCK;
	}

	if (pthread_create(recv_ftp_th, NULL, thread_wait_file, th_data))
	{
		slog(LOG_CRIT, "Error creando thread de recepción ftp en el puerto %d: %s", th_data->sock, strerror(errno));
		return ERR;
	}
	else
	{
		slog(LOG_DEBUG, "Thread de recepción ftp creado.");
		return OK;
	}
}


void * thread_send_file(void * ftpdata){
	struct th_ftpdata * data = ftpdata;
	char buffer[MAX_LEN_FTP];
	int snd_bytes = 1;
	int counter_snd_bytes = 0;
	data->cb(ftp_started);

	pthread_cleanup_push(free, data);

	while (snd_bytes > 0 && counter_snd_bytes < data->size){
		snd_bytes = fread(buffer, sizeof(char), MAX_LEN_FTP, data->f);
		if (snd_bytes > 0)
		{
			if (send_message(data->sock, buffer, snd_bytes) < 0)
			{
				slog(LOG_ERR, "Error enviando un mensaje de transmisión ftp");
				data->cb(ftp_aborted);
				break;
			}

		}
		counter_snd_bytes += snd_bytes;
	}

	if (counter_snd_bytes == data->size)
	{
		data->cb(ftp_finished);
		slog(LOG_DEBUG, "Transmisión finalizada con éxito");
	}

	slog(LOG_DEBUG, "Hilo envío ftp saliendo");
	pthread_cleanup_pop(0);
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
	slog(LOG_DEBUG, "Fichero %s abierto para leer y enviar",file);

	bzero(&dst_addr, sizeof dst_addr);
	dst_addr.sin_addr.s_addr = ip;
	dst_addr.sin_port = port;
	dst_addr.sin_family = PF_INET;

	sock = dsocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, 1);

	if (sock == ERR_SOCK)
	{
		slog(LOG_CRIT, "Error creando socket de envío: %s", strerror(errno));
		return ERR_SOCK;
	}

	if (dconnect(sock, (struct sockaddr *)&dst_addr, sizeof dst_addr) == -1)
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
		slog(LOG_DEBUG, "Thread de envío ftp creado.");
		return OK;
	}

	return OK;
}

static int _can_open_file(const char* file, const char* mode)
{
	FILE* f = fopen(file, mode);

	if(!f) return 0;
	fclose(f);
	return 1;
}

int irc_ftp_reqsend(struct ftp_connection* conn, const char* file)
{
	if(conn->status != ftp_finished)
		return ERR;

	if(!_can_open_file(file, "r"))
		return ERR_NOTFOUND;

	strncpy(conn->file_to_send, file, MAX_FTP_PATH);
	conn->status = ftp_requested;

	return OK;
}

int irc_ftp_accept(struct ftp_connection* conn, const char* file, int* port, ftp_callback cb)
{
	if(conn->status != ftp_recv_req)
		return ERR;

	if(!file)
		file = conn->file_to_send;

	if(!_can_open_file(file, "w"))
		return ERR_NOTFOUND;

	if(ftp_wait_file(file, port, cb, &conn->th_manager) != OK)
		return ERR;

	return OK;
}

int irc_ftp_recvaccept(struct ftp_connection* conn, const char* ip, const char* port, ftp_callback cb)
{
	int port_n;
	uint32_t ip_n;

	if(conn->status != ftp_requested)
		return ERR;

	ip_n = strtol(ip, NULL, 10);

    if (ip_n == 0)
    {
        slog(LOG_ERR, "IP inválido: %s", ip);
        return ERR;
    }

    port_n = strtol(port, NULL, 10);

    if (port_n == 0)
    {
        slog(LOG_ERR, "Puerto inválido: %s", port);
        return ERR;
    }	

    if(ftp_send_file(conn->file_to_send, ip_n, port_n, cb, &conn->th_manager) != OK)
    	return ERR;

    return OK;
}

int irc_ftp_recvsend(struct ftp_connection* conn, const char* file)
{
	if(conn->status != ftp_finished)
		return ERR;

	strncpy(conn->file_to_send, file, MAX_FTP_PATH);
	conn->status = ftp_recv_req;

	return OK;
}

int irc_ftp_cancel(struct ftp_connection* conn)
{
	if(conn->status == ftp_finished)
		return ERR;

	conn->status = ftp_finished;

	pthread_cancel(conn->th_manager);
	return OK;
}

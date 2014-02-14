#include "listener.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include "errors.h"
#include "listener.h"
#include <syslog.h>

int Server_open_socket(int port, int max_long){

	int handler = _server_open_socket();
	if (handler == -ERR_SOCK)	
	{
		syslog(LOG_ERR, "Error al abrir el socket");
		return -ERR_SOCK;
	}
	_link_socket_port(port, handler);
	
	if( _set_queue_socket(handler, max_long)!=OK)
		return -ERR_SOCK;

	/*
	Código de error: Valor negativo -> ERROR
	*/
	return handler;
}

int Server_listen_connect(int handler){
	struct sockaddr peer_addr;
	socklen_t peer_len=sizeof(peer_addr);
	int handler_accepted;


	handler_accepted=accept(handler,&peer_addr, &peer_len);
	
	if( handler_accepted == -1){
		syslog(LOG_ERR, "Error aceptando conexiones, %d",errno);
		return -ERR_SOCK;
	}

	/*
	- Qué hacemos con handler_accepted¿? Es el socket de la nueva conexión. El enunciado dice devolver código de error
	- ¿Cómo gestionar más d euna conexión?
	*/

	return handler_accepted;

}

int Server_close_communication(int handler){
	return _server_close_socket(handler);
}


static int _server_close_socket(int handler){
	if(shutdown(handler, 2) == -1){
		syslog(LOG_ERR, "Error cerrando el socket, %d",errno);
		return -ERR_SOCK;
	}
	return OK;
}





static int _server_open_socket()
{
	int handler = socket(AF_INET, SOCK_STREAM, TCP);
	if (handler==-1)
	{
		syslog(LOG_ERR, "Error en la creación de socket");
		return -ERR_SOCK;
	}
	return handler;
}

static int _link_socket_port(int port, int handler){
	struct sockaddr_in serv_addr;

	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_family = AF_INET;

	bzero( (void *) serv_addr.sin_zero, sizeof(serv_addr.sin_zero));

	if(bind(handler, (struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in) ) == -1)
	{
		syslog(LOG_ERR, "Error asociando puerto con socket.");
		return -ERR_SOCK;
	}

	return OK;
}

static int _set_queue_socket(int handler,int long_max){

	if (listen(handler, long_max) == -1){
		syslog(LOG_ERR, "Error al poner a escuchar: errno=%d", errno);
		return -ERR_SOCK;
	}
	return OK;
}





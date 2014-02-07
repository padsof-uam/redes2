#include "listener.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include "errors.h"
#include "listener.h"
#include <syslog.h>

int ServerOpenSocket(int port, int max_long);
int ServerListenConnect();
int ServerClose();


static int _server_open_socket()
{
	int handler = socket(AF_INET, SOCK_STREAM, TCP);
	if (handler==-1)
	{
		syslog(LOG_ERR, "Error en la creación de socket");
		return SYS_ERR;
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
		return SYS_ERR;
	}

	return OK;
}


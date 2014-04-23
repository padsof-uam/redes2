#include "sockutils.h"
#include "listener.h"
#include "errors.h"
#include "messager.h"
#include "log.h"
#include "sysutils.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/resource.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

static int _server_close_socket(int handler)
{
    if (handler > 0 && shutdown(handler,SHUT_RDWR) < 0 && errno != ENOTCONN)
    {
        slog(LOG_ERR, "Error cerrando el socket: %s", strerror(errno));
        return ERR_SOCK;
    }

    return OK;
}

static int _server_open_socket()
{
    int handler = socket(AF_INET, SOCK_STREAM, TCP);
    if (handler == -1)
    {
        slog(LOG_ERR, "Error en la creación de socket");
        return ERR_SOCK;
    }
    return handler;
}

static int _link_socket_port(int port, int handler)
{
    struct sockaddr_in serv_addr;

    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;

    bzero( (void *) serv_addr.sin_zero, sizeof(serv_addr.sin_zero));

    if (bind(handler, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in) ) == -1)
    {
        slog(LOG_ERR, "Error asociando puerto con socket: %s", strerror(errno));
        return ERR_SOCK;
    }

    return OK;
}

static int _set_queue_socket(int handler, int long_max)
{
    if (listen(handler, long_max) == -1)
    {
        slog(LOG_ERR, "Error al poner a escuchar: %s", strerror(errno));
        return ERR_SOCK;
    }
    return OK;
}


int server_open_socket(int port, int max_long)
{
    int handler = _server_open_socket();
    if (handler == ERR_SOCK)
    {
        slog(LOG_ERR, "Error al abrir el socket");
        return ERR_SOCK;
    }

    if(_link_socket_port(port, handler) != OK)
        return ERR_SOCK;

    if ( _set_queue_socket(handler, max_long) != OK)
        return ERR_SOCK;

    fcntl(handler, F_SETFL, O_NONBLOCK);

    return handler;
}

int server_open_tcp_socket_b(int port, int max_long)
{
    int handler = _server_open_socket();

    if (handler == ERR_SOCK)
    {
        slog(LOG_ERR, "Error al abrir el socket");
        return ERR_SOCK;
    }

    if(_link_socket_port(port, handler) != OK)
        return ERR_SOCK;

    if ( _set_queue_socket(handler, max_long) != OK)
        return ERR_SOCK;

    if (listen(handler, 1) != OK)
        return ERR_SOCK;

    return handler;
}


int server_listen_connect(int handler)
{
    struct sockaddr peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    int handler_accepted;

    handler_accepted = accept(handler, &peer_addr, &peer_len);

    if (handler_accepted == -1)
    {
        slog(LOG_ERR, "Error aceptando conexiones : %s", strerror(errno));
        return ERR_SOCK;
    }

    if(fcntl(handler_accepted, F_SETFL, O_NONBLOCK) == -1)
    {
        slog(LOG_WARNING, "Error al marcar el socket %d como O_NONBLOCK: %s", handler_accepted, strerror(errno));
    }

    return handler_accepted;

}

int server_close_communication(int handler)
{
    return _server_close_socket(handler);
}

int resolve_ip4(const char* host, uint32_t* ip)
{
    struct addrinfo *info;
    struct addrinfo hints;
    int retval;

    bzero(&hints, sizeof(struct addrinfo));

    hints.ai_flags = AI_ALL;
    hints.ai_family = PF_INET;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;

    retval = getaddrinfo(host, NULL, &hints, &info);

    if (retval == EAI_SYSTEM)
        return ERR_SYS;
    else if (retval != 0)
        return ERR_AIR;

    *ip = ((struct sockaddr_in*)info->ai_addr)->sin_addr.s_addr; 
    
    freeaddrinfo(info);

    return OK;
}

int client_connect_to(const char* host, const char* port, char* resolved_addr, size_t resadr_len)
{
    int sock;
    struct addrinfo *info, *info_orig;
    struct addrinfo hints;
    int retval;
    short connected = 0;
    int port_num;
    char addr_buffer[INET6_ADDRSTRLEN+10];

    bzero(&hints, sizeof(struct addrinfo));

    hints.ai_flags = AI_ALL;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_protocol = IPPROTO_TCP;

    retval = getaddrinfo(host, port, &hints, &info);

    if (retval == EAI_SYSTEM)
        return ERR_SYS;
    else if (retval != 0)
        return ERR_AIR;

    info_orig = info;

    while (info != NULL && !connected)
    {
        sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

        if (sock == -1 || connect(sock, info->ai_addr, info->ai_addrlen) == -1)
        {
            close(sock);
            info = info->ai_next;
        }
        else
        {
            connected = 1;
        }
    }

    if(resolved_addr && info != NULL)
    {
        if(info->ai_family == PF_INET)
            port_num = ntohs(((struct sockaddr_in*)info->ai_addr)->sin_port);
        else if (info->ai_family == PF_INET6)
            port_num = ntohs(((struct sockaddr_in6*)info->ai_addr)->sin6_port);
        else
            port_num = -1;

        inet_ntop(info->ai_family, &(((struct sockaddr_in*)info->ai_addr)->sin_addr), addr_buffer, info->ai_addrlen);
        snprintf(resolved_addr, resadr_len, "%s:%d", addr_buffer, port_num);
    }
    
    freeaddrinfo(info_orig);

    if (!connected)
        return ERR_NOTFOUND;

    return sock;
}

int get_socket_port(int sock, int *port)
{
    struct sockaddr_in addr;
    socklen_t sa_size = sizeof(struct sockaddr_in);

    if (getsockname(sock, (struct sockaddr *)&addr, &sa_size) == -1)
        return ERR_SOCK;

    *port = addr.sin_port;

    return OK;
}

/*int open_listen_tcp_socket(int port)
{
    int sock;
    struct sockaddr_in addr;

    bzero(&addr, sizeof(struct sockaddr_in));

    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == -1)
        return ERR_SOCK;

    if (bind(sock, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in) ) == -1)
    {
        close(sock);
        return ERR_SOCK;
    }

    return sock;
}
*/
int open_listen_udp_socket()
{
    int sock;
    struct sockaddr_in addr;

    bzero(&addr, sizeof(struct sockaddr_in));

    addr.sin_family = PF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = INADDR_ANY;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock == -1)
        return ERR_SOCK;

    if (bind(sock, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in) ) == -1)
    {
        close(sock);
        return ERR_SOCK;
    }

    return sock;
}


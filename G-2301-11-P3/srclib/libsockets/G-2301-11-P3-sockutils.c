#include "G-2301-11-P3-sockutils.h"
#include "G-2301-11-P3-listener.h"
#include "G-2301-11-P3-errors.h"
#include "G-2301-11-P3-messager.h"
#include "G-2301-11-P3-log.h"
#include "G-2301-11-P3-sysutils.h"
#include "G-2301-11-P3-ssltrans.h"

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
#include <pthread.h>

static int _server_close_socket(int handler)
{
    if (handler > 0 && dshutdown(handler, SHUT_RDWR) < 0 && errno != ENOTCONN)
    {
        slog(LOG_ERR, "Error cerrando el socket: %s", strerror(errno));
        return ERR_SOCK;
    }

    return OK;
}

static int _server_open_socket(short use_ssl)
{
    int handler = dsocket(AF_INET, SOCK_STREAM, TCP, use_ssl);

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
    serv_addr.sin_family = PF_INET;

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


int server_open_socket(int port, int max_long, short use_ssl)
{
    int handler = _server_open_socket(use_ssl);

    if (handler == ERR_SOCK)
    {
        slog(LOG_ERR, "Error al abrir el socket");
        return ERR_SOCK;
    }

    if (_link_socket_port(port, handler) != OK)
        return ERR_SOCK;

    if (_set_queue_socket(handler, max_long) != OK)
        return ERR_SOCK;

    fcntl(handler, F_SETFL, O_NONBLOCK);

    return handler;
}


int server_listen_connect(int handler)
{
    struct sockaddr peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    int handler_accepted;

    handler_accepted = daccept(handler, &peer_addr, &peer_len);

    if (handler_accepted == -1)
    {
        slog(LOG_ERR, "Error aceptando conexión en socket %d: %s", handler, strerror(errno));
        return ERR_SOCK;
    }

    if (fcntl(handler_accepted, F_SETFL, O_NONBLOCK) == -1)
        slog(LOG_WARNING, "Error al marcar el socket %d como no bloqueante: %s", handler_accepted, strerror(errno));

    return handler_accepted;

}

int server_close_communication(int handler)
{
    return _server_close_socket(handler);
}

int resolve_ip4(const char *host, uint32_t *ip)
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

    *ip = ((struct sockaddr_in *)info->ai_addr)->sin_addr.s_addr;

    freeaddrinfo(info);

    return OK;
}

int client_connect_to(const char *host, const char *port, char *resolved_addr, size_t resadr_len, short use_ssl)
{
    int sock;
    struct addrinfo *info, *info_orig;
    struct addrinfo hints;
    int retval;
    short connected = 0;
    int port_num;
    char addr_buffer[INET6_ADDRSTRLEN + 10];

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
        sock = dsocket(info->ai_family, info->ai_socktype, info->ai_protocol, use_ssl);
        
        if (sock == -1 || dconnect(sock, info->ai_addr, info->ai_addrlen) != 0)
        {
            dclose(sock);
            info = info->ai_next;
        }
        else
        {
            connected = 1;
        }
    }

    if (resolved_addr && info != NULL)
    {
        if (info->ai_family == PF_INET)
            port_num = ntohs(((struct sockaddr_in *)info->ai_addr)->sin_port);
        else if (info->ai_family == PF_INET6)
            port_num = ntohs(((struct sockaddr_in6 *)info->ai_addr)->sin6_port);
        else
            port_num = -1;

        inet_ntop(info->ai_family, &(((struct sockaddr_in *)info->ai_addr)->sin_addr), addr_buffer, info->ai_addrlen);
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

struct _ssl_sp_data {
    int sock;
    struct sockaddr* addr;
    socklen_t addr_len;
    int retval;
};

static void* _sp_connect(void* data)
{
    struct _ssl_sp_data* thdata = data;

    thdata->retval = dconnect(thdata->sock, thdata->addr, thdata->addr_len);

    return NULL;
}

int ssl_socketpair(int domain, int type, int protocol, int sockets[2])
{
    int sock_snd = -1, sock_rcv = -1, sock_lst = -1;
    struct sockaddr addr, caddr;
    socklen_t addr_len = sizeof addr, caddr_len = sizeof caddr;
    struct _ssl_sp_data thdata;
    pthread_t th;

    sock_snd = dsocket(domain, type, protocol, 1);
    sock_lst = dsocket(domain, type, protocol, 1);

    if (type == SOCK_STREAM || type == SOCK_SEQPACKET)
    {
        if (_link_socket_port(0, sock_lst) == -1)
            goto error;

        if (_set_queue_socket(sock_lst, 1) == -1)
            goto error;
    }
    else
    {
        if (getsockname(sock_snd, &addr, &addr_len) == -1)
            goto error;

        if (dconnect(sock_lst, &addr, addr_len) == -1)
            goto error;
    }

    addr_len = sizeof addr;

    if (getsockname(sock_lst, &addr, &addr_len) == -1)
        goto error;

    thdata.sock = sock_snd;
    thdata.addr = &addr;
    thdata.addr_len = addr_len;

    if(pthread_create(&th, NULL, _sp_connect, &thdata) == -1)
        goto error;

    sock_rcv = daccept(sock_lst, &caddr, &caddr_len);

    if(sock_rcv == -1)
    {
        pthread_cancel(th);
        pthread_join(th, NULL);
        goto error;
    }

    if(pthread_join(th, NULL) != 0)
        goto error;

    if(thdata.retval == -1)
        goto error;

    sockets[0] = sock_snd;
    sockets[1] = sock_rcv;

    close(sock_lst);

    return 0;
error:
    if (sock_snd != -1)
        dclose(sock_snd);
    if (sock_lst != -1)
        dclose(sock_lst);
    if (sock_rcv != -1)
        dclose(sock_rcv);

    return -1;
}

int sock_set_block(int socket, short block)
{
    int opts = fcntl(socket, F_GETFL);

    if(opts < 0)
        return ERR_SOCK;

    if(block)
        opts = opts & (~O_NONBLOCK);
    else
        opts = opts | O_NONBLOCK;

    if(fcntl(socket, F_SETFL, opts) < 0)
        return ERR_SOCK;

    return OK;
}

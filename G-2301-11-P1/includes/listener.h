#ifndef LISTENER_H
#define LISTENER_H value


#define TCP 6                /* Protocolo TCP                            */
#define UDP 17               /* Protocolo UDP                            */


int Server_open_socket(int port, int max_long);
int Server_listen_connect();
int Server_close();

static int _server_open_socket();

static int _link_socket_port(int port, int handler);

static int _set_queue_socket(int handler,int long_max);

#endif
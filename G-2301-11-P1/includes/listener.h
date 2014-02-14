#ifndef LISTENER_H
#define LISTENER_H value
#include "errors.h"
#define TCP 6                /* Protocolo TCP                            */
#define UDP 17               /* Protocolo UDP                            */


int Server_open_socket(int port, int max_long);
int Server_listen_connect(int handler);
int Server_close_communication(int handler);

#endif

#include <stdlib.h>
#include <sys/socket.h>

#ifndef MESSAGER_H
#define MESSAGER_H

int sock_data_available(int socket);
int send_message(int socket, const void* msg, ssize_t len);
ssize_t rcv_message(int socket, void** buffer);
#endif


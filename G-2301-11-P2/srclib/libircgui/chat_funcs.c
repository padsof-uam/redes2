#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "chat.h"
#include "messager.h"

extern int rcv_sockcomm;

void connectClient(void)
{
    int sock;
    char *server = getServidor();
    int port = getPuerto();
    char port_str[10];
    struct addrinfo *info;
    const char *err;
    char addr_str[100];
    int retval;
    short connected = 0;

    snprintf(port_str, 10, "%d", port);

    if (!server || strlen(server) == 0)
    {
        errorText("Error: servidor inválido.");
        return;
    }

    if (port <= 0)
    {
        errorText("Error: puerto inválido.");
        return;
    }


    messageText("Conectando con %s...", server);

    retval = getaddrinfo(server, port_str, NULL, &info);

    if (retval == EAI_SYSTEM)
        err = strerror(errno);
    else if (retval != 0)
        err = gai_strerror(retval);

    if (retval != 0)
    {
        errorText("Error resolviendo la dirección %s: %s", server, err);
        return;
    }

    while (info != NULL && !connected)
    {
        inet_ntop(info->ai_family, info->ai_addr, addr_str, info->ai_addrlen);
        sock = socket(info->ai_family, info->ai_socktype, 17);

        messageText("Resolviendo %s: tratando de conectar con %s...", server, addr_str);

        if (sock == -1 || connect(sock, info->ai_addr, info->ai_addrlen) == -1)
        {
            errorText("Error conectando a %s: %s", server, strerror(errno));
            close(sock);
            info = info->ai_next;
        }
        else
        {
            connected = 1;
        }
    }

    if (!connected)
    {
        errorText("No se ha podido connectar con el servidor %s", server);
        return;
    }

    if (send_message(rcv_sockcomm, &sock, sizeof(int)) == -1)
    {
        errorText("Error al configurar la nueva conexión: %s", strerror(errno));
        return;
    }

    messageText("Conectado a %s:%d", server, port);
}

void disconnectClient(void)
{
    errorWindow("Función no implementada");
    errorText("Función no implementada");
}

void topicProtect(gboolean state)
{
    errorWindow("Función no implementada");
    errorText("Función no implementada");
}

void externMsg(gboolean state)
{
    errorWindow("Función no implementada");
    errorText("Función no implementada");
}

void secret(gboolean state)
{
    errorWindow("Función no implementada");
    errorText("Función no implementada");
}

void guests(gboolean state)
{
    errorWindow("Función no implementada");
    errorText("Función no implementada");
}

void privated(gboolean state)
{
    errorWindow("Función no implementada");
    errorText("Función no implementada");
}

void moderated(gboolean state)
{
    errorWindow("Función no implementada");
    errorText("Función no implementada");
}

void newText (const char *msg)
{
    errorText("Función no implementada");
}



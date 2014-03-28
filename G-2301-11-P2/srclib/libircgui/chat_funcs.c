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

#include "gui_client.h"
#include "messager.h"
#include "sockutils.h"
#include "errors.h"

extern int rcv_sockcomm;

void connectClient(void)
{
    int sock;
    char *server = getServidor();
    char* port = getPuerto();
    const char *err;
    char addr_str[100];
    int retval;

    if (!server || strlen(server) == 0)
    {
        errorText("Error: servidor inválido.");
        return;
    }

    if (!port || strlen(port) == 0)
    {
        errorText("Error: puerto inválido.");
        return;
    }

    messageText("Conectando con %s...", server);

 	retval = client_connect_to(server, port, addr_str, 100);

    if (retval == ERR_SYS)
        err = strerror(errno);
    else if (retval == ERR_AIR)
        err = "no se ha podido resolver la dirección";
    else if (retval == ERR_NOTFOUND)
    	err = "no se ha podido conectar.";
    else
    	err = "error desconocido.";

    if (retval <= 0)
    {
        errorText("Error resolviendo %s: %s", server, err);
        return;
    }

    sock = retval;

    if (send_message(rcv_sockcomm, &sock, sizeof(int)) == ERR_SOCK)
    {
        errorText("Error al configurar la nueva conexión: %s", strerror(errno));
        close(sock);
        return;
    }

    messageText("Conectado a %s", addr_str);
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



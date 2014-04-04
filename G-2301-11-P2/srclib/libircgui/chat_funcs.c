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
#include "log.h"
#include "messager.h"
#include "sockutils.h"
#include "errors.h"
#include "irc_processor.h"
#include "list.h"

int rcv_sockcomm;
struct irc_globdata *ircdata;
int snd_qid;
int serv_sock;

void connectClient(void)
{
    int sock;
    const char *err;
    char addr_str[100];
    int retval;
    char *server = getServidor();
    char *port = getPuerto();
    char *nick = getApodo();
    char *user = getNombre();
    char *name = getNombreReal();

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

    if (!nick || !user || !name || strlen(nick) == 0 || strlen(user) == 0 || strlen(name) == 0)
    {
        errorWindow("Rellene los datos de nombre/usuario/apodo");
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

    irc_send_message(snd_qid, sock, "NICK %s", getApodo());
    irc_send_message(snd_qid, sock, "USER %s %s %s :%s", getNombre(), "0", "*", getNombreReal());

    ircdata->connected = 1;

    serv_sock = sock;

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

    if (!msg)
    {
        slog(LOG_ERR, "Se ha colado un mensaje NULL desde la interfaz");
        return;
    }

    if (*msg != '/')
    {
        if (!ircdata->connected)
        {
            errorText("No estás conectado a ningún servidor");
        }
        else if (list_count(ircdata->chan_list) == 0)
        {
            errorText("No perteneces a ningún canal al que enviar el mensaje");
        }
        else
        {
            /* Mandamos al servidor el mensaje para los usuarios del canal. */
            irc_send_message(snd_qid, serv_sock, "PRIVMSG #%s %s", (char *) list_at(ircdata->chan_list, 0), msg);
            messageText((char *) msg);
        }
    }
    else
    {
        irc_send_message(snd_qid, serv_sock, msg + 1);
    }
}




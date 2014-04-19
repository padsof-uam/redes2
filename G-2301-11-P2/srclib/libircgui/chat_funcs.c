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
#include "irc_funs_userinput.h"
#include "server_history.h"

int rcv_sockcomm;
struct irc_globdata *ircdata;
int snd_qid;
struct irc_clientdata *client;

void connectClient(void)
{
    char *server = getServidor();
    char *port = getPuerto();

    connectToServer(server, port);
}

void connectToFavServ(int favserv)
{
    struct serv_info serv;

    if(serv_get_number(favserv, &serv) != OK)
    {
        errorText("No hemos podido recuperar el servidor número %d", favserv);
        return;
    }

    setServidor(serv.servname);
    setPuerto(serv.port);

    connectToServer(serv.servname, serv.port);
}

void connectToServer(const char *server, const char* port)
{
    char *nick = getApodo();
    char *user = getNombre();
    char *name = getNombreReal();
    int sock;
    const char *err;
    char addr_str[100];
    int retval;
    struct serv_info serv;

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

    if(client->connected)
        disconnectClient(NULL);

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

    client->connected = 1;
    strncpy(client->nick, nick, MAX_NICK_LEN);
    client->serv_sock = sock;

    setUserConnectionState(TRUE);

    strncpy(serv.servname, server, SERV_NAMELEN);
    strncpy(serv.port, port, MAX_PORT_LEN);
    serv_save_connection(&serv);

    messageText("Conectado a %s", addr_str);
    saveUserSettings(nick, user, name);
}

void disconnectClient(const char* bye_msg)
{
    if(!bye_msg)
        bye_msg = "Redirc - bye!";

    irc_send_message(snd_qid, client->serv_sock, "QUIT :%s", bye_msg);
    shutdown(client->serv_sock, 2);
    messageText("Desconexión del servidor.");
    setUserConnectionState(FALSE);
}

void _send_flag(char flag, gboolean state)
{
    char mod;

    if (state)
        mod = '+';
    else
        mod = '-';

    irc_send_message(snd_qid, client->serv_sock, "MODE %s %c%c", client->chan, mod, flag);
}

void topicProtect(gboolean state)
{
    _send_flag('t', state);
}

void externMsg(gboolean state)
{
    _send_flag('n', state);
}

void secret(gboolean state)
{
    _send_flag('s', state);
}

void guests(gboolean state)
{
    _send_flag('i', state);
}

void privated(gboolean state)
{
    _send_flag('p', state);
}

void moderated(gboolean state)
{
    _send_flag('m', state);
}

static short _is_server_message(const char* msg)
{
    return strncmp("/server", msg, strlen("/server")) == 0;
}

void newText (const char *msg)
{
    char irc_msg[MAX_IRC_MSG];

    if (!msg)
    {
        slog(LOG_ERR, "Se ha colado un mensaje NULL desde la interfaz");
        return;
    }

    slog(LOG_DEBUG, "Recibido el mensaje \"%s\" desde interfaz para ser procesado", msg);

    if(!client->connected && !_is_server_message(msg))
    {
        errorText("No estás conectado a ningún servidor.");
        return;
    }

    if (*msg != '/')
    {
        if (!client->in_channel)
        {
            errorText("No perteneces a ningún canal al que enviar el mensaje");
        }
        else
        {
            /* Mandamos al servidor el mensaje para los usuarios del canal. */
            irc_send_message(snd_qid, client->serv_sock, "PRIVMSG %s :%s", client->chan, msg);
            privateText(client->nick, msg);
        }
    }
    else
    {
        snprintf(irc_msg, MAX_IRC_MSG, "%s\r\n", msg + 1);
        irc_user_msgprocess(irc_msg, client);
    }
}




#include "irc_funs_client.h"
#include "gui_client.h"
#include "irc_processor.h"
#include "log.h"
#include "gui_client.h"
#include "sockutils.h"

#include <string.h>
#include <arpa/inet.h>

const char *_irc_client_cmds[] =
{
    "332", /* RPL_TOPIC */
    "433", /* ERR_NICKNAMEINUSE */
    "436", /* ERR_NICKCOLLISION */
    "461", /* ERR_NEEDMOREPARAMS */
    "352", /* RPL_WHO */
    "315", /* RPL_ENDOFWHO */
    "376", /* RPL_ENDOFMOTD */
    "JOIN",
    "PART",
    "PRIVMSG",
    "NICK",
    "QUIT",
    "NOTICE",
    "MODE",
    "PING",
    "*"
};

cmd_action _irc_client_actions[] =
{
    irc_recv_topic,
    irc_nickcollision,
    irc_nickcollision,
    irc_needmoreparams,
    irc_recv_who,
    irc_ignore,
    irc_recv_end_motd,
    irc_recv_join,
    irc_recv_part,
    irc_recv_privmsg,
    irc_recv_nick,
    irc_recv_quit,
    irc_recv_notice,
    irc_recv_mode,
    irc_recv_ping,
    irc_default
};

void irc_client_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_clientdata *cdata)
{
    _irc_msgprocess(snd_qid, data, NULL, cdata, _irc_client_cmds, _irc_client_actions, (sizeof(_irc_client_actions) / sizeof(cmd_action)));
}

int irc_ignore(void* data)
{
    return OK;
}


int irc_recv_end_motd(void* data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    irc_send_response(msgdata, "WHO %s", msgdata->clientdata->nick);

    return OK;
}

int irc_default(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    noticeText("server: %s", msgdata->msg);
    return OK;
}

int irc_recv_topic(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    char *params[3];

    if (irc_parse_paramlist(msgdata->msg, params, 3) < 3)
    {
        slog(LOG_ERR, "Respuesta RPL_TOPIC mal formada: %s", msgdata->msg);
        return OK;
    }

    messageText("El tema del canal \"%s\" es : %s", params[1], params[2]);

    return OK;
}

int irc_recv_join(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    char user[MAX_NICK_LEN + 1];
    char *params[1];

    if (irc_get_prefix(msgdata->msg, user, MAX_NICK_LEN + 1) != OK || irc_parse_paramlist(msgdata->msg, params, 1) != 1)
    {
        slog(LOG_ERR, "Mensaje JOIN mal formado: %s", msgdata->msg);
        return OK;
    }

    if (!strncmp(user, msgdata->clientdata->nick, MAX_NICK_LEN))
    {
        if (msgdata->clientdata->in_channel)
            irc_send_response(msgdata, "PART %s", msgdata->clientdata->chan);

        msgdata->clientdata->in_channel = 1;
        enableChanMods();
        strncpy(msgdata->clientdata->chan, params[0], MAX_CHAN_LEN);
    }

    messageText("%s ha entrado en el canal \"%s\"", user, params[0]);

    return OK;
}

int irc_recv_part(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    char user[MAX_NICK_LEN + 1];
    char *params[1];

    if (irc_get_prefix(msgdata->msg, user, MAX_NICK_LEN + 1) != OK || irc_parse_paramlist(msgdata->msg, params, 1) != 1)
    {
        slog(LOG_ERR, "Mensaje PART mal formado: %s", msgdata->msg);
        return OK;
    }

    if (!strncmp(user, msgdata->clientdata->nick, MAX_NICK_LEN) && !strncmp(msgdata->clientdata->chan, params[0], MAX_CHAN_LEN))
    {
        msgdata->clientdata->in_channel = 0;
        disableChanMods();
    }

    messageText("%s ha salido del canal \"%s\"", user, params[0]);

    return OK;
}

int irc_recv_privmsg(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    char user[MAX_NICK_LEN + 1];
    char *params[2];
    char *text;

    if (irc_get_prefix(msgdata->msg, user, MAX_NICK_LEN) != OK ||
            irc_parse_paramlist(msgdata->msg, params, 2) != 2)
    {
        slog(LOG_ERR, "Mensaje PRIVMSG mal formado: %s", msgdata->msg);
        return OK;
    }

    text = params[1];

    if (!strncmp("$PCALL ", text, strlen("$PCALL ")))
        parse_pcall(msgdata->clientdata, text, user);
    else if (!strncmp("$PACCEPT ", text, strlen("$PACCEPT ")))
        parse_paccept(msgdata->clientdata, text, user);
    else if (!strncmp("$PCLOSE ", text, strlen("$PACCEPT ")))
        parse_pclose(msgdata->clientdata, text, user);
    else
        publicText(user, text);

    return OK;
}

int irc_needmoreparams(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    char *params[2];

    if (irc_parse_paramlist(msgdata->msg, params, 2) < 2)
    {
        slog(LOG_ERR, "Respuesta ERR_NEEDMOREPARAMS mal formada: %s", msgdata->msg);
        return OK;
    }

    errorText("Se necesitan más parámetros para el comando %s", params[0]);

    return OK;
}

int irc_nickcollision(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;

    errorText("El nick que has escogido (%s) está en uso. Elige otro con el comando \"/nick tunuevonick\"", msgdata->clientdata->nick);
    return OK;
}

int irc_recv_nick(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    char prefix[MAX_NICK_LEN + 1];
    char *params[1];

    irc_get_prefix(msgdata->msg, prefix, MAX_NICK_LEN + 1);

    if (irc_parse_paramlist(msgdata->msg, params, 1) != 1)
    {
        slog(LOG_ERR, "Mensaje NICK mal formado: %s", msgdata->msg);
        return OK;
    }

    messageText("%s cambió su apodo a %s", prefix, params[0]);

    if (!strncmp(prefix, msgdata->clientdata->nick, MAX_NICK_LEN))
    {
        msgdata->clientdata->connected = 1;
        strncpy(msgdata->clientdata->nick, params[0], MAX_NICK_LEN);
        setApodo(params[0]);
        saveUserSettings(params[0], getNombre(), getNombreReal());
    }


    return OK;
}

int irc_recv_quit(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    char prefix[MAX_NICK_LEN + 1];
    char *params[1];

    irc_get_prefix(msgdata->msg, prefix, MAX_NICK_LEN + 1);

    if (irc_parse_paramlist(msgdata->msg, params, 1) != 1)
    {
        slog(LOG_ERR, "Mensaje QUIT mal formado: %s", msgdata->msg);
        return OK;
    }

    messageText("%s ha salido del canal: %s", prefix, params[0]);

    if (!strncmp(prefix, msgdata->clientdata->nick, MAX_NICK_LEN))
        msgdata->clientdata->connected = 0;

    return OK;
}

int irc_recv_notice(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    char *params[2];

    if (irc_parse_paramlist(msgdata->msg, params, 2) != 2)
    {
        slog(LOG_ERR, "Mensaje NOTICE mal formado: %s", msgdata->msg);
        return OK;
    }

    noticeText("server: %s", params[1]);

    return OK;
}

int irc_recv_mode(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    char *params[3];
    int pnum;
    char *target, *mode_str, *param;
    int mode = msgdata->clientdata->chanmode;

    params[2] = NULL;

    struct ircflag chan_flags[] =
    {
        {'p', chan_priv},
        {'s', chan_secret},
        {'i', chan_invite},
        {'t', chan_topiclock},
        {'n', chan_nooutside},
        {'m', chan_moderated},
        IRCFLAGS_END
    };

    pnum = irc_parse_paramlist(msgdata->msg, params, 3);

    target = params[0];
    mode_str = params[1];
    param = params[2] ? params[2] : "";

    if (pnum >= 2 && (target[0] == '#' || target[0] == '&') &&
            strncmp(target, msgdata->clientdata->chan, MAX_CHAN_LEN) == 0)
    {
        irc_flagparse(mode_str, &mode, chan_flags);

        setTopicProtect(mode & chan_topiclock, TRUE);
        setExternMsg(mode & chan_nooutside, TRUE);
        setSecret(mode & chan_secret, TRUE);
        setGuests(mode & chan_invite, TRUE);
        setPrivate(mode & chan_priv, TRUE);
        setModerated(mode & chan_moderated, TRUE);

        messageText("Channel mode modified: %s %s", mode_str, param);

        msgdata->clientdata->chanmode = mode;
    }
    else
    {
        messageText("User %s modified its mode: %s %s", target, mode_str, param);
    }

    return OK;
}


void parse_pcall(struct irc_clientdata *cdata, char *text, char *source)
{
    char *params[2];
    int port;
    uint32_t ip;

    if (irc_parse_paramlist(text, params, 2) != 2)
    {
        slog(LOG_ERR, "Mensaje PRIVMSG/PCALL mal formado: %s", text);
        return;
    }

    ip = strtol(params[0], NULL, 10);

    if (ip == 0)
    {
        slog(LOG_ERR, "IP inválido: %s", params[0]);
        return;
    }

    port = strtol(params[1], NULL, 10);

    if (port == 0)
    {
        slog(LOG_ERR, "Puerto inválido: %s", params[1]);
        return;
    }

    if (cdata->call_status == call_none)
    {
        cdata->call_ip = ip;
        cdata->call_port = port;
        cdata->call_status = call_incoming;
        strncpy(cdata->call_user, source, MAX_NICK_LEN);

        messageText("Tienes una llamada de %s. Acéptala con /paccept.", source);
    }
    else
    {
        messageText("Has recibido una llamada de %s, pero sólo puedes tener una llamada a la vez.", source);
    }
}

void parse_paccept(struct irc_clientdata *cdata, char *text, char *source)
{
    char *params[2];
    int port;
    uint32_t ip;

    if (cdata->call_status != call_outgoing)
    {
        slog(LOG_WARNING, "Hemos recibido un mensaje PACCEPT pero no hay llamadas pendientes.");
        return;
    }

    if (irc_parse_paramlist(text, params, 2) != 2)
    {
        slog(LOG_ERR, "Mensaje PRIVMSG/PACCEPT mal formado: %s", text);
        return;
    }

    ip = strtol(params[0], NULL, 10);

    if (ip == 0)
    {
        slog(LOG_ERR, "IP inválido: %s", params[0]);
        return;
    }

    port = strtol(params[1], NULL, 10);

    if (port == 0)
    {
        slog(LOG_ERR, "Puerto inválido: %s", params[1]);
        return;
    }

    signal(SIGALRM, SIG_IGN);

    spawn_call_manager_thread(&(cdata->call_info), ip, port, cdata->call_socket);
    cdata->call_status = call_running;
    messageText("Llamada aceptada.");
}

void parse_pclose(struct irc_clientdata *cdata, char *text, char *source)
{
    if (cdata->call_status != call_none)
    {
        messageText("El usuario remoto ha terminado la llamada.");
        cdata->call_status = call_none;
        call_stop(&(cdata->call_info));
    }
}

int irc_recv_ping(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    char *payload = irc_next_param(msgdata->msg);

    if (!payload)
        payload = "";

    irc_send_response(msgdata, "PONG %s", payload);

    return OK;
}

int irc_recv_who(void *data)
{
    struct irc_msgdata *msgdata = (struct irc_msgdata *) data;
    char *msg = msgdata->msg;
    char *host;
    char *params[8];
    int pnum;
    uint32_t ip;

    pnum = irc_parse_paramlist(msg, params, 8);

    if (pnum != 8)
    {
        slog(LOG_WARNING, "Mensaje RPL_WHO inválido: %s", msg);
        return OK;
    }

    messageText("WHO %s (%s/%s): conectado desde %s a %s", params[5], params[2], params[7], params[3], params[4]);

    if (!strncmp(msgdata->clientdata->nick, params[5], MAX_NICK_LEN)) /* Nuestro usuario */
    {
        host = params[3];
        if (resolve_ip4(host, &ip) != OK)
        {
            slog(LOG_WARNING, "No se ha podido resolver %s", host);
        }
        else
        {
            msgdata->clientdata->client_ip = ip;
            slog(LOG_INFO, "Guardada IP del cliente: %s", host);
        }
    }

    return OK;
}

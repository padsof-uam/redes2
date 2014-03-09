#include "irc_processor.h"
#include "commparser.h"
#include "irc_core.h"
#include "irc_codes.h"
#include "irc_funs.h"
#include "strings.h"

#include <string.h>
#include <sys/syslog.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>

const char *_irc_cmds[] =
{
    "PRIVMSG",
    "PING",
    "NICK",
    "USER",
    "QUIT",
    "JOIN",
    "PART",
    "TOPIC",
    "NAMES",
    "LIST",
    "KICK",
    "TIME",
    "NOTICE",
    "PONG",
    "USERS"
};

cmd_action _irc_actions[] =
{
    irc_privmsg,
    irc_ping,
    irc_nick,
    irc_user,
    irc_quit,
    irc_join,
    irc_part,
    irc_topic,
    irc_names,
    irc_list,
    irc_kick,
    irc_time,
    irc_notice,
    irc_pong,
    irc_users
};

void irc_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_globdata *gdata)
{
    struct irc_msgdata ircdata;
    char *msg;
    char *msg_end;
    int i;

    ircdata.globdata = gdata;
    ircdata.msgdata = data;
    ircdata.msg_tosend = list_new();

    if (irc_user_byid(gdata, data->fd) == NULL)
        irc_register_user(gdata, data->fd);

    msg = data->data;
    msg[MAX_IRC_MSG] = '\0';
    msg = irc_remove_prefix(msg);

    while ((msg_end = irc_msgsep(msg, MAX_IRC_MSG)) != NULL)
    {
        ircdata.msg = msg;

        if (parse_exec_command(msg, _irc_cmds, _irc_actions, (sizeof(_irc_actions) / sizeof(cmd_action)), &ircdata) == -1)
            syslog(LOG_WARNING, "Error parsing command %s", data->data);

        msg = msg_end;
    }

    for (i = 0; i < list_count(ircdata.msg_tosend); i++)
        irc_enqueue_msg(list_at(ircdata.msg_tosend, i), snd_qid);

    list_destroy(ircdata.msg_tosend, free);
}

char *irc_msgsep(char *str, int len)
{
    char crlf[] = "\r\n";
    char *msgend;

    if (!str)
        return NULL;

    msgend = strnstr(str, crlf, len);

    if (msgend)
    {
        /* Quitamos el \r\n */
        *msgend = '\0';
        msgend++;
        *msgend = '\0';
        msgend++;
    }

    return msgend;
}

int irc_parse_paramlist(char *msg, char **params, size_t max_params)
{
    char *colon;
    char *arg;
    int param_count = 0;

    msg = irc_remove_prefix(msg);
    msg = strchr(msg, ' ');

    if(!msg) 
        return 0; /* No hay un espacio detrás del comando -> no hay argumentos. */

    msg++; /* Marcamos al inicio de la lista de parámetros */
    colon = strchr(msg, ':');

    if (colon != NULL)
    {
        *colon = '\0';
        colon++;
    }

    while ((arg = strsep(&msg, " ")) != NULL && param_count < max_params)
    {
        strip(&arg);

        if(strlen(arg) == 0)
            continue;

        params[param_count] = arg;
        param_count++;
    }

    if (param_count < max_params && colon != NULL)
    {
        params[param_count] = colon;
        param_count++;
    }

    return param_count;
}

int irc_enqueue_msg(struct sockcomm_data *msg, int snd_qid)
{
    struct msg_sockcommdata qdata;

    memcpy(&(qdata.scdata), msg, sizeof(struct sockcomm_data));
    qdata.msgtype = 1;

    if (msgsnd(snd_qid, &qdata, sizeof(struct msg_sockcommdata), 0) == -1)
    {
        syslog(LOG_ERR, "error enviando en cola de mensajes de envío: %s", strerror(errno));
        return ERR;
    }

    return OK;
}


char *irc_remove_prefix(char *msg)
{
    char *space;

    if (!msg)
        return NULL;

    if (msg[0] != ':')
        return msg;

    space = strchr(msg, ' ');

    if (!space)
        return NULL;
    else
        return space + 1;
}

static struct sockcomm_data* _irc_quit_message(const char* name, const char* message, int fd)
{
    struct sockcomm_data* smsg = malloc(sizeof(struct sockcomm_data*));

    smsg->fd = fd;
    smsg->len = snprintf(smsg->data, MAX_IRC_MSG, ":%s QUIT :%s", name, message);

    return smsg;
}

int irc_create_quit_messages(struct ircuser* user, list* msgqueue, const char* message)
{
    int i, j;
    int chan_count, user_count;
    struct ircchan* chan;
    list* user_list;
    struct ircuser* target;

    chan_count = list_count(user->channels);

    for(i = 0; i < chan_count; i++)
    {
        chan = list_at(user->channels, i);
        user_list = chan->users;
        user_count = list_count(user_list);

        for(j = 0; j < user_count; j++)
        {
            target = list_at(user_list, j);
            if(irc_compare_user(target, user) != 0) /* No enviamos mensaje al mismo usuario */
                list_add(msgqueue, _irc_quit_message(user->nick, message, user->fd));
        }
    }

    return OK;
}

struct sockcomm_data *irc_build_errmsg(struct irc_msgdata *irc, int errcode, const char *additional_params)
{
    return irc_build_errmsg_withtext(irc, errcode, additional_params, irc_errstr(errcode));
}

struct sockcomm_data *irc_build_errmsg_withtext(struct irc_msgdata *irc, int errcode, const char *additional_params, const char* text)
{
    struct ircuser* user = irc_user_byid(irc->globdata, irc->msgdata->fd);
    char* nick;
    char space[2] = "";

    space[0] = '\0';
    space[1] = '\0';

    if(user)
        nick = user->nick;
    else
        nick = "?";

    if(!additional_params)
        additional_params = "";
    else
        space[0] = ' ';

    return irc_response_create(irc->msgdata->fd, ":%s %d %s%s%s :%s", irc->globdata->servername, errcode, nick, space, additional_params, text);    
}

struct sockcomm_data* irc_response_create(int fd, const char* fmt_string, ...)
{
    va_list ap;
    struct sockcomm_data* msg = malloc(sizeof(struct sockcomm_data));

    msg->fd = fd;
    va_start(ap, fmt_string);
    msg->len = vsnprintf(msg->data, MAX_IRC_MSG, fmt_string, ap);
    va_end(ap);

    return msg;
}

char *irc_errstr(int errcode)
{

    switch (errcode)
    {
    case OK:
        return " ";
    case ERR_NOSUCHNICK:

        break;
    case ERR_NOSUCHSERVER:

        break;
    case ERR_NOSUCHCHANNEL:

        break;
    case ERR_CANNOTSENDTOCHAN:

        break;
    case ERR_TOOMANYCHANNELS:

        break;
    case ERR_WASNOSUCHNICK:

        break;
    case ERR_TOOMANYTARGETS:

        break;
    case ERR_NOORIGIN:

        break;
    case ERR_NORECIPIENT:

        break;
    case ERR_NOTEXTTOSEND:

        break;
    case ERR_NOTOPLEVEL:

        break;
    case ERR_WILDTOPLEVEL:

        break;
    case ERR_UNKNOWNCOMMAND:

        break;
    case ERR_NOMOTD:

        break;
    case ERR_NOADMININFO:

        break;
    case ERR_FILEERROR:

        break;
    case ERR_NONICKNAMEGIVEN:

        break;
    case ERR_ERRONEUSNICKNAME:

        break;
    case ERR_NICKNAMEINUSE:

        break;
    case ERR_NICKCOLLISION:

        break;
    case ERR_USERNOTINCHANNEL:

        break;
    case ERR_NOTONCHANNEL:

        break;
    case ERR_USERONCHANNEL:

        break;
    case ERR_NOLOGIN:

        break;
    case ERR_SUMMONDISABLED:

        break;
    case ERR_USERSDISABLED:

        break;
    case ERR_NOTREGISTERED:

        break;
    case ERR_NEEDMOREPARAMS:

        break;
    case ERR_ALREADYREGISTRED:

        break;
    case ERR_NOPERMFORHOST:

        break;
    case ERR_PASSWDMISMATCH:

        break;
    case ERR_YOUREBANNEDCREEP:

        break;
    case ERR_KEYSET:

        break;
    case ERR_CHANNELISFULL:

        break;
    case ERR_UNKNOWNMODE:

        break;
    case ERR_INVITEONLYCHAN:

        break;
    case ERR_BANNEDFROMCHAN:

        break;
    case ERR_BADCHANNELKEY:

        break;
    case ERR_NOPRIVILEGES:

        break;
    case ERR_CHANOPRIVSNEEDED:

        break;
    case ERR_CANTKILLSERVER:

        break;
    case ERR_NOOPERHOST:

        break;
    case ERR_UMODEUNKNOWNFLAG:

        break;
    case ERR_USERSDONTMATCH:
        break;
    default:
        return "Rellena este error";
    }

    return "Default error";
}

#include "irc_processor.h"
#include "commparser.h"
#include "irc_core.h"
#include "irc_codes.h"
#include "irc_funs_server.h"
#include "irc_funs_client.h"
#include "strings.h"
#include "log.h"

#include <string.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/socket.h>

const char *_irc_server_cmds[] =
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
    "USERS",
    "OPER",
    "MODE",
    "INVITE",
    "VERSION",
    "KILL",
    "AWAY",
    "WHO",
    "ISON"
};

cmd_action _irc_server_actions[] =
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
    irc_users,
    irc_oper,
    irc_mode,
    irc_invite,
    irc_version,
    irc_kill,
    irc_away,
    irc_who,
    irc_ison
};

const char * _irc_client_cmds[] = 
{
    "332", /* RPL_TOPIC */
    "JOIN",
    "PART",
    "PRIVMSG",
    "*"
};

cmd_action _irc_client_actions[] =
{
    irc_recv_topic,
    irc_recv_join,
    irc_recv_part,
    irc_recv_privmsg,
    irc_default
};

void irc_server_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_globdata *gdata)
{
    _irc_msgprocess(snd_qid, data, gdata, _irc_server_cmds, _irc_server_actions, (sizeof(_irc_server_actions)/sizeof(cmd_action)));
}

void irc_client_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_globdata *gdata)
{
    _irc_msgprocess(snd_qid, data, gdata,_irc_client_cmds, _irc_client_actions, (sizeof(_irc_client_actions)/sizeof(cmd_action)));
}


void _irc_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_globdata *gdata, const char ** cmds, cmd_action * actions,int len)
{
    struct irc_msgdata ircdata;
    char *msg;
    char *msg_end;
    int i;
    struct ircuser *user;

    ircdata.globdata = gdata;
    ircdata.msgdata = data;
    ircdata.msg_tosend = list_new();
    ircdata.connection_to_terminate = 0;

    if (data->fd < 0) /* Usuario eliminado */
    {
        user = irc_user_byid(gdata, - data->fd);

        if (user)
            irc_delete_user(gdata, user);

        return;
    }

    if (irc_user_byid(gdata, data->fd) == NULL)
        irc_register_user(gdata, data->fd);

    msg = data->data;
    msg[MAX_IRC_MSG] = '\0';

    while ((msg_end = irc_msgsep(msg, MAX_IRC_MSG)) != NULL)
    {
        ircdata.msg = msg;

        slog_debug("Recibido mensaje \"%s\" en %d", msg, data->fd);

        if (parse_exec_command(irc_remove_prefix(msg), cmds, actions, len, &ircdata) == -1)
        {
            slog(LOG_WARNING, "Error parsing command %s", msg);
            irc_send_numericreply(&ircdata, ERR_UNKNOWNCOMMAND, msg);
        }

        msg = msg_end;
    }

    for (i = 0; i < list_count(ircdata.msg_tosend); i++)
        irc_enqueue_msg(list_at(ircdata.msg_tosend, i), snd_qid);

    list_destroy(ircdata.msg_tosend, free);

    if (ircdata.connection_to_terminate)
        shutdown(ircdata.connection_to_terminate, SHUT_RD);
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

int irc_get_prefix(const char* msg, char* prefix_buf, size_t max_prefix_len)
{
    char* space_pos;
    int prefix_len;

    if(msg[0] != ':')
        return ERR_PARSE;
    msg++;
    space_pos = strchr(msg, ' ');

    if(!space_pos)
        return ERR_PARSE;

    prefix_len = space_pos - msg;

    if(prefix_len + 1 > max_prefix_len)
        prefix_len = max_prefix_len - 1;

    strncpy(prefix_buf, msg, prefix_len);
    prefix_buf[prefix_len] = '\0';

    return OK;
}

int irc_parse_paramlist(char *msg, char **params, size_t max_params)
{
    char *colon;
    char *arg;
    int param_count = 0;

    msg = irc_remove_prefix(msg);
    msg = strchr(msg, ' ');

    if (!msg)
        return 0; /* No hay un espacio detrás del comando -> no hay argumentos. */

    msg++; /* Marcamos al inicio de la lista de parámetros */
    if (*msg=='\0')
    {
        return 0; /* Hay un espacion detrás del comando pero no hay argumentos. */
    }
    colon = strchr(msg, ':');

    if (colon != NULL)
    {
        *colon = '\0';
        colon++;
    }

    while ((arg = strsep(&msg, " ")) != NULL && param_count < max_params)
    {
        strip(&arg);

        if (strlen(arg) == 0)
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

int irc_send_message(int snd_qid, int fd, const char* format, ...)
{
    struct sockcomm_data* msg;
    int retval;
    va_list ap;
    va_start(ap, format);

    msg = irc_response_vcreate(fd, format, ap);

    if(msg == NULL)
        return ERR;

    retval =  irc_enqueue_msg(msg, snd_qid);
    free(msg);
    return retval;
}

int irc_enqueue_msg(struct sockcomm_data *msg, int snd_qid)
{
    struct msg_sockcommdata qdata;

    memcpy(&(qdata.scdata), msg, sizeof(struct sockcomm_data));
    qdata.msgtype = 1;

    if (msgsnd(snd_qid, &qdata, sizeof(struct sockcomm_data), 0) == -1)
    {
        slog(LOG_ERR, "error enviando en cola de mensajes de envío: %s", strerror(errno));
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

static int _irc_create_quitkill_messages(struct ircuser *user, list *msgqueue, const char *message, const char *cmd)
{
    int i;
    int chan_count;
    struct ircchan *chan;

    chan_count = list_count(user->channels);

    for (i = 0; i < chan_count; i++)
    {
        chan = list_at(user->channels, i);
        irc_channel_broadcast(chan, msgqueue, NULL, ":%s %s :%s", user->nick, cmd, message);
    }

    return OK;
}

int irc_create_quit_messages(struct ircuser *user, list *msgqueue, const char *message)
{
    return _irc_create_quitkill_messages(user, msgqueue, message, "QUIT");
}

int irc_create_kill_messages(struct ircuser *user, list *msgqueue, const char *tokill_name, const char *message)
{
    char cmd[20];
    snprintf(cmd, 20, "KILL %s", tokill_name);
    return _irc_create_quitkill_messages(user, msgqueue, message, cmd);
}

struct sockcomm_data *irc_build_numericreply(struct irc_msgdata *irc, int errcode, const char *additional_params)
{
    return irc_build_numericreply_withtext(irc, errcode, additional_params, irc_errstr(errcode));
}

struct sockcomm_data *irc_build_numericreply_withtext(struct irc_msgdata *irc, int errcode, const char *additional_params, const char *text)
{
    struct ircuser *user = irc_user_byid(irc->globdata, irc->msgdata->fd);
    char *nick;
    char space[2] = "";

    space[0] = '\0';
    space[1] = '\0';

    if (user)
        nick = user->nick;
    else
        nick = "?";

    if (!additional_params)
        additional_params = "";
    else
        space[0] = ' ';

    return irc_response_create(irc->msgdata->fd, ":%s %d %s%s%s :%s", irc->globdata->servername, errcode, nick, space, additional_params, text);
}

struct sockcomm_data *irc_response_vcreate(int fd, const char *fmt_string, va_list ap)
{
    struct sockcomm_data *msg = malloc(sizeof(struct sockcomm_data));

    msg->fd = fd;
    msg->len = vsnprintf(msg->data, MAX_IRC_MSG - 2, fmt_string, ap);
    msg->data[msg->len] = '\r';
    msg->data[msg->len + 1] = '\n';
    msg->len += 2;

    return msg;
}

struct sockcomm_data *irc_response_create(int fd, const char *fmt_string, ...)
{
    va_list ap;
    struct sockcomm_data *msg;

    va_start(ap, fmt_string);
    msg = irc_response_vcreate(fd, fmt_string, ap);
    va_end(ap);

    return msg;
}

int irc_send_numericreply(struct irc_msgdata *irc, int errcode, const char *additional_params)
{
    return irc_send_numericreply_withtext(irc, errcode, additional_params, irc_errstr(errcode));
}

int irc_send_numericreply_withtext(struct irc_msgdata *irc, int errcode, const char *additional_params, const char *text)
{
    struct sockcomm_data *msg;

    msg = irc_build_numericreply_withtext(irc, errcode, additional_params, text);

    if (msg == NULL)
    {
        slog(LOG_WARNING, "Error construyendo mensaje de respuesta %d para el socket %d", errcode, irc->msgdata->fd);
        return ERR;
    }

    list_add(irc->msg_tosend, msg);
    return OK;
}


int irc_channel_broadcast(struct ircchan *channel, list *msg_tosend, struct ircuser *sender, const char *message, ...)
{
    va_list ap;
    int i;
    struct ircuser *user;
    char msg[MAX_IRC_MSG];
    msg[MAX_IRC_MSG - 1] = '\0';

    if (!channel || !msg_tosend)
        return ERR;

    va_start(ap, message);
    vsnprintf(msg, MAX_IRC_MSG - 2, message, ap);
    va_end(ap);

    for (i = 0; i < list_count(channel->users); i++)
    {
        user = list_at(channel->users, i);

        /* Parece que no podemos hacer eso, pero sí podemos. Los punteros son siempre
         *  constantes, un usuario siempre está en la misma estructura así que si el
         *  puntero es igual, los usuarios son iguales. */
        if (sender == NULL || sender != user)
            list_add(msg_tosend, irc_response_create(user->fd, msg));
    }

    return OK;
}

int irc_flagparse(const char *flags, int *flagval, const struct ircflag *flagdic)
{
    int i;
    short adding;

    adding = flags[0] == '+';
    flags++;

    while (*flags != ' ' && *flags != '\0')
    {
        for (i = 0; !IS_IRCFLAGS_END(flagdic[i]); i++)
        {
            if (*flags == flagdic[i].code)
            {
                if (adding)
                    *flagval |= flagdic[i].value;
                else
                    *flagval &= ~(flagdic[i].value);
            }
        }

        flags++;
    }

    return OK;
}

int irc_strflag(int flags, char *str, size_t len, const struct ircflag *flagdic)
{
    int sp, i;
    str[0] = '+';
    sp = 1;

    for (i = 0; !IS_IRCFLAGS_END(flagdic[i]) && sp < len - 1; i++)
    {
        if (flags & flagdic[i].value)
        {
            str[sp] = flagdic[i].code;
            sp++;
        }
    }

    str[sp] = '\0';

    return OK;
}


int irc_send_names_messages(struct ircchan *channel, struct irc_msgdata *ircdata)
{
    struct ircuser *user;
    list *users;
    int j;

    users = channel->users;
    for (j = 0; j < list_count(users); j++)
    {
        user = list_at(users, j);
        irc_send_numericreply_withtext(ircdata, RPL_NAMREPLY, channel->name, user->nick);
    }

    return OK;
}


int irc_send_response(struct irc_msgdata* irc, const char* msg_format,...)
{
    struct sockcomm_data* msg;
    va_list ap;
    va_start(ap, msg_format);
    msg = irc_response_vcreate(irc->msgdata->fd, msg_format, ap);
    va_end(ap);

    if(!msg)
        return ERR;

    return list_add(irc->msg_tosend, msg);
}

int irc_users_have_common_chans(struct ircuser* a, struct ircuser* b)
{
    int i;
    struct ircchan* chan;

    for(i = 0; i < list_count(a->channels); i++)
    {
        chan = list_at(a->channels, i);
        if(irc_user_inchannel(chan, b) == OK)
            return 1;
    }

    return 0;
}

char *irc_errstr(int errcode)
{
    switch (errcode)
    {
    case OK:
        return " ";
    case ERR_NOSUCHNICK:
        return "No suck nick/channel";
    case ERR_NOSUCHSERVER:
        return "No such server";
    case ERR_NOSUCHCHANNEL:
        return "No such channel";
    case ERR_CANNOTSENDTOCHAN:
        return "Cannot send to channel";
    case ERR_TOOMANYCHANNELS:
        return "You have joined too many channels";
    case ERR_WASNOSUCHNICK:
        return "There was no such nickname";
    case ERR_TOOMANYTARGETS:
        return "Duplicate recipients. No message delivered";
    case ERR_NOORIGIN:
        return "No origin specificed";
    case ERR_NORECIPIENT:
        return "No recipient given";
    case ERR_NOTEXTTOSEND:
        return "No text to send";
    case ERR_NOTOPLEVEL:
        return "No toplevel domain specified";
    case ERR_WILDTOPLEVEL:
        return "Wildcard in toplevel domain";
    case ERR_UNKNOWNCOMMAND:
        return "Unknown command";
    case ERR_NOMOTD:
        return "MOTD File is missing";
    case ERR_NOADMININFO:
        return "No administrative info available";
    case ERR_FILEERROR:
        return "File error doing %s on %s";
    case ERR_NONICKNAMEGIVEN:
        return "No nickname given";
    case ERR_ERRONEUSNICKNAME:
        return "Erroneus nickname";
    case ERR_NICKNAMEINUSE:
        return "Nickname is already in use";
    case ERR_NICKCOLLISION:
        return "Nickname collision KILL";
    case ERR_USERNOTINCHANNEL:
        return "They aren't on that channel";
    case ERR_NOTONCHANNEL:
        return "You're not on that channel";
    case ERR_USERONCHANNEL:
        return "is already in channel";
    case ERR_NOLOGIN:
        return "User not logged in";
    case ERR_SUMMONDISABLED:
        return "SUMMON has been disabled";
    case ERR_USERSDISABLED:
        return "USERS has been disabled";
    case ERR_NOTREGISTERED:
        return "You have not registered";
    case ERR_NEEDMOREPARAMS:
        return "Not enough parameters";
    case ERR_ALREADYREGISTRED:
        return "You may not reregister";
    case ERR_NOPERMFORHOST:
        return "Your host is not among the privileged";
    case ERR_PASSWDMISMATCH:
        return "Password incorrect";
    case ERR_YOUREBANNEDCREEP:
        return "You''re banned from this server";
    case ERR_KEYSET:
        return "Channel key already set";
    case ERR_CHANNELISFULL:
        return "Cannot join channel (+l)";
    case ERR_UNKNOWNMODE:
        return "is unknown mode char to me";
    case ERR_INVITEONLYCHAN:
        return "Cannot join channel (+i)";
    case ERR_BANNEDFROMCHAN:
        return "Cannot join channel (+b)";
    case ERR_BADCHANNELKEY:
        return "Cannot join channel (+k)";
    case ERR_NOPRIVILEGES:
        return "Permission Denied- You're not an IRC operator";
    case ERR_CHANOPRIVSNEEDED:
        return "You're not channel operator";
    case ERR_CANTKILLSERVER:
        return "You can't kill a server";
    case ERR_NOOPERHOST:
        return "No O-lines for your host";
    case ERR_UMODEUNKNOWNFLAG:
        return "Unknown MODE flag";
    case ERR_USERSDONTMATCH:
        return "Can''t change mode for other users";
    case RPL_NOTOPIC:
        return "No topic is set";
    case RPL_ENDOFNAMES:
        return "End of /NAMES list";
    case RPL_YOUREOPER:
        return "You're now an IRC operator";
    case RPL_NOWAWAY:
        return "You have been marked as being away";
    case RPL_UNAWAY:
        return "You are no longer marked as being away";
    case RPL_LISTSTART:
        return "Channel users topic";
    case RPL_LISTEND:
        return "End of /LIST";
    case RPL_ENDOFWHO:
        return "End of /WHO list";
    default:
        return "I don't know that error";
    }
}

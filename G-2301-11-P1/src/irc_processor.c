#include "irc_processor.h"
#include "commparser.h"
#include "irc_core.h"
#include "irc_codes.h"
#include "irc_funs.h"

#include <string.h>
#include <sys/syslog.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

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

    colon = strchr(msg, ':');

    if (colon != NULL)
    {
        *colon = '\0';
        colon++;
    }

    while ((arg = strsep(&msg, ",")) != NULL && param_count < max_params)
    {
        params[param_count] = arg;
        param_count++;
    }

    if (param_count < max_params)
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


struct sockcomm_data *irc_build_errmsg(int errcode, struct irc_msgdata *irc , char *msg)
{

    struct sockcomm_data *msg_answer = malloc(sizeof(struct sockcomm_data));
    char retval[MAX_IRC_MSG];

    msg_answer->fd = irc->msgdata->fd;

    if (msg != NULL)
    {
        _irc_numeric_reponse(irc->globdata, OK, retval);
        snprintf(msg_answer->data, MAX_IRC_MSG, "%s %s", retval, msg);
        msg_answer->len = strlen(msg_answer->data + 1);
        return msg_answer;
    }
    else
    {
        _irc_numeric_reponse(irc->globdata, errcode, retval);
        strncpy(msg_answer->data, retval, MAX_IRC_MSG);
        msg_answer->len = strlen(_irc_errmsg(errcode));
    }

    return msg_answer;
}


void _irc_numeric_reponse(struct irc_globdata *irc, int errcode, char *retval)
{
    sprintf(retval, ":%s %d %s", irc->servername, errcode, _irc_errmsg(errcode));
}

char *_irc_errmsg(int errcode)
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
}

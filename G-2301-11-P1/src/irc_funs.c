#include "irc_funs.h"
#include "irc_core.h"
#include "irc_codes.h"
#include "irc_processor.h"
#include "log.h"
#include "strings.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#define MSG_PRIVMSG 1
#define MSG_NOTICE 2

static int _irc_send_msg_tochan(struct irc_msgdata *irc, const char *receiver, const char *text, int msgtype)
{
    struct ircchan *chan = irc_channel_byname(irc->globdata, receiver);
    struct ircuser *sender = irc_user_byid(irc->globdata, irc->msgdata->fd);
    char *cmd_name = msgtype == MSG_PRIVMSG ? "PRIVMSG" : "NOTICE";

    if (!chan)
    {
        if (msgtype == MSG_PRIVMSG)
            irc_send_numericreply(irc, ERR_NOSUCHNICK, receiver);
        return OK;
    }

    irc_channel_broadcast(chan, irc->msg_tosend, sender, ":%s %s %s :%s", sender->nick, cmd_name, receiver, text);

    return OK;
}

static int _irc_send_msg_touser(struct irc_msgdata *irc, const char *receiver, const char *text, int msgtype)
{
    struct ircuser *dest = irc_user_bynick(irc->globdata, receiver);
    struct ircuser *sender = irc_user_byid(irc->globdata, irc->msgdata->fd);
    char *cmd_name = msgtype == MSG_PRIVMSG ? "PRIVMSG" : "NOTICE";

    if (!dest && msgtype == MSG_PRIVMSG)
        irc_send_numericreply(irc, ERR_NOSUCHNICK, receiver);
    else if (dest->is_away && msgtype == MSG_PRIVMSG)
        irc_send_numericreply_withtext(irc, RPL_AWAY, receiver, dest->away_msg);
    else if (dest && !dest->is_away)
        list_add(irc->msg_tosend, irc_response_create(dest->fd, ":%s %s %s :%s", sender->nick, cmd_name, receiver, text));

    return OK;
}

int _irc_internal_msg(void *data, int msgtype)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    char *params[2];
    int param_num;
    char *dests, *text, *receiver;

    param_num = irc_parse_paramlist(ircdata->msg, params, 2);

    if (param_num < 1)
    {
        irc_send_numericreply(ircdata, ERR_NORECIPIENT, NULL);
        return OK;
    }
    else if (param_num < 2)
    {
        irc_send_numericreply(ircdata, ERR_NOTEXTTOSEND, NULL);
        return OK;
    }

    dests = params[0];
    text = params[1];

    while ((receiver = strsep(&dests, ",")) != NULL)
    {
        if (receiver[0] == '#' || receiver[0] == '&')
            _irc_send_msg_tochan(ircdata, receiver, text, msgtype);
        else
            _irc_send_msg_touser(ircdata, receiver, text, msgtype);
    }

    return OK;
}

int irc_privmsg(void *data)
{
    return _irc_internal_msg(data, MSG_PRIVMSG);
}

int irc_ping(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct sockcomm_data *msg_tosend = malloc(sizeof(struct sockcomm_data));

    slog(LOG_INFO, "ping!");
    strcpy(msg_tosend->data, "PONG\r\n");
    msg_tosend->len = strlen(msg_tosend->data);
    msg_tosend->fd = ircdata->msgdata->fd;

    list_add(ircdata->msg_tosend, msg_tosend);

    return OK;
}

int irc_nick(void *data)
{
    int retval;
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    char *new_nick[1];

    if (!irc_parse_paramlist(ircdata->msg, new_nick, 1))
    {
        irc_send_numericreply(ircdata, ERR_UNKNOWNCOMMAND, NULL);
        return ERR;
    }

    retval = irc_set_usernick(ircdata->globdata, ircdata->msgdata->fd, new_nick[0]);

    if (retval != OK)
    {
        if (retval == ERR_NOTFOUND)
            irc_send_numericreply(ircdata, ERR_ERRONEUSNICKNAME, NULL);
        else if (retval == ERR_NICKCOLLISION)
            irc_send_numericreply(ircdata, ERR_NICKCOLLISION, NULL);
        else
            slog(LOG_ERR, "Error desconocido %d al cambiar nick del usuario a %s", retval, new_nick[0]);
    }

    return OK;
}

int irc_user(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    char *params[4];
    struct ircuser *user;

    if (irc_parse_paramlist(ircdata->msg, params, 4) < 4)
    {
        irc_send_numericreply(ircdata, ERR_NEEDMOREPARAMS, NULL);
        return ERR;
    }
    user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);

    if (user)
        strncpy(user->name, params[3], MAX_NAME_LEN);

    return OK;
}

int irc_join(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircchan *channel;
    char *params[2];
    char *channels[10];
    char *passwords[10];
    int chan_num, pass_num, i;
    char *key;
    struct ircuser *user;
    int retval = OK;

    if (irc_parse_paramlist(ircdata->msg, params, 2) == 0)
    {
        list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_NEEDMOREPARAMS, NULL));
        return ERR;
    }

    user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);

    if (!user)
    {
        irc_send_numericreply(ircdata, ERR_NOTFOUND, "No has podido unirte al canal porque no eres un usuario");
        return OK;
    }

    chan_num = str_arrsep(params[0], ",", channels, 10);
    pass_num = str_arrsep(params[1], ",", passwords, 10);

    for(i = 0; i < chan_num; i++)
    {
        channel = irc_channel_byname(ircdata->globdata, channels[i]);

        if (!channel)
            channel = irc_register_channel(ircdata->globdata, channels[i]);

        key = i >= pass_num ? NULL : passwords[i];
        retval = irc_channel_adduser(ircdata->globdata, channel, user, key);

        if (retval != OK) 
        {
            irc_send_numericreply(ircdata, retval, channels[i]);
        }
        else
        {
            irc_send_numericreply_withtext(ircdata, RPL_TOPIC, channels[i], channel->topic);
            irc_send_names_messages(channel, ircdata);
            irc_channel_broadcast(channel, ircdata->msg_tosend, NULL, ":%s JOIN %s", user->nick, channel->name);
        }
    }

    return OK;
}

int irc_quit(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircuser *user;
    char *bye_msg;
    char *params[1];

    user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);

    if (!user)
        return ERR_NOTFOUND;

    if (irc_parse_paramlist(ircdata->msg, params, 1) == 0)
        bye_msg = user->nick; /* Mensaje de salida por defecto */
    else
        bye_msg = params[0];

    irc_create_quit_messages(user, ircdata->msg_tosend, bye_msg);
    ircdata->connection_to_terminate = ircdata->msgdata->fd;
    return OK;
}

int irc_part(void *data)
{
    int i;
    char *channel_name;
    char *params[1];
    char *chans[50];
    int channum;
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircuser *user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    struct ircchan *channel;

    if (!user)
    {
        list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_NOTREGISTERED, NULL));
        return OK;
    }

    irc_parse_paramlist(ircdata->msg, params, 1);

    channum = str_arrsep(params[0], ",", chans, 50);

    for (i = 0; i < channum; i++)
    {
        channel_name = chans[i];
        channel = irc_channel_byname(ircdata->globdata, channel_name);

        if (!channel)
            irc_send_numericreply(ircdata, ERR_NOSUCHCHANNEL, channel_name);
        else if (irc_user_inchannel(channel, user) != OK)
            irc_send_numericreply(ircdata, ERR_USERNOTINCHANNEL, NULL);
        else
        {
            irc_channel_broadcast(channel, ircdata->msg_tosend, NULL, ":%s PART %s", user->nick, channel_name);
            irc_channel_part(ircdata->globdata, channel, user);
        }
    }

    return OK;
}

/** Pendientes **/
int irc_topic(void *data)
{
    char *params[2];
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircchan *chan;
    char *chan_name, *topic;
    struct ircuser *user;
    int pnum;

    pnum = irc_parse_paramlist(ircdata->msg, params, 2);
    user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);

    if (pnum == 0)
    {
        list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_NEEDMOREPARAMS, "TOPIC"));
        return OK;
    }

    chan_name = params[0];
    chan = irc_channel_byname(ircdata->globdata, chan_name);

    if (chan == NULL || irc_user_inchannel(chan, user) == ERR_NOTFOUND)
        return irc_send_numericreply(ircdata, ERR_NOTONCHANNEL, chan_name);

    if ((chan->mode & chan_topiclock) && !irc_is_channel_op(chan, user))
    {
        irc_send_numericreply(ircdata, ERR_CHANOPRIVSNEEDED, chan_name);
        return OK;
    }

    if (pnum == 1) /* Sólo nos piden el canal */
    {
        if (strnlen(chan->topic, MAX_TOPIC_LEN) == 0)
            irc_send_numericreply(ircdata, RPL_NOTOPIC, chan_name);
        else
            irc_send_numericreply_withtext(ircdata, RPL_TOPIC, chan_name, chan->topic);
    }
    else
    {
        topic = params[1];
        strncpy(chan->topic, topic, MAX_TOPIC_LEN);
        irc_channel_broadcast(chan, ircdata->msg_tosend, NULL, ":%s TOPIC %s :%s", user->nick, chan_name, topic);
    }

    return OK;
}

int irc_names(void *data)
{
    char *params[1];
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircchan *chan;
    struct ircuser *source = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    char *chanlist = NULL;
    int i;

    if (irc_parse_paramlist(ircdata->msg, params, 1) == 1)
        chanlist = params[0];

    for (i = 0; i < list_count(ircdata->globdata->chan_list); i++)
    {
        chan = list_at(ircdata->globdata->chan_list, i);

        if ((!(chan->mode & (chan_priv | chan_secret)) || irc_user_inchannel(chan, source) == OK) /* No mostramos canales ni secretos ni privados si el usuario no está en ellos */
                && (chanlist == NULL || strnstr(chanlist, chan->name, MAX_IRC_MSG) == 0)) /* Si el usuario ha especificado un canal, mostrar sólo esos */
        {
            irc_send_names_messages(chan, ircdata);
        }
    }

    return OK;
}

int irc_list(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircchan *chan;
    struct ircuser *source = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    char *chanlist = NULL;
    char *params[1];
    char chandata[50];
    int i;

    if (irc_parse_paramlist(ircdata->msg, params, 1) == 1)
        chanlist = params[0];

    irc_send_numericreply(ircdata, RPL_LISTSTART, NULL);

    for (i = 0; i < list_count(ircdata->globdata->chan_list); i++)
    {
        chan = list_at(ircdata->globdata->chan_list, i);

        if ((!(chan->mode & (chan_priv | chan_secret)) || irc_user_inchannel(chan, source) == OK) /* No mostramos canales ni secretos ni privados si el usuario no está en ellos */
                && (chanlist == NULL || strnstr(chanlist, chan->name, MAX_IRC_MSG) == 0)) /* Si el usuario ha especificado un canal, mostrar sólo esos */
        {
            snprintf(chandata, 50, "%s %d", chan->name, list_count(chan->users));
            irc_send_numericreply_withtext(ircdata, RPL_LIST, chandata, chan->topic);
        }
    }

    irc_send_numericreply(ircdata, RPL_LISTEND, NULL);

    return OK;
}

int irc_kick(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircuser *sender, *kicked;
    struct ircchan *chan;
    char *params[3];
    char *chan_name, *username;
    int pnum;

    pnum = irc_parse_paramlist(ircdata->msg, params, 3);

    if (pnum < 2)
    {
        irc_send_numericreply(ircdata, ERR_NEEDMOREPARAMS, "KICK");
        return OK;
    }

    chan_name = params[0];
    username = params[1];

    chan = irc_channel_byname(ircdata->globdata, chan_name);

    if (!chan)
    {
        irc_send_numericreply(ircdata, ERR_NOSUCHCHANNEL, chan_name);
        return OK;
    }

    sender = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    kicked = irc_user_bynick(ircdata->globdata, username);

    if (!kicked || !irc_user_inchannel(chan, kicked))
    {
        irc_send_numericreply(ircdata, ERR_NOTONCHANNEL, chan_name); /* No estoy seguro de esto */
        return OK;
    }

    if (!irc_is_channel_op(chan, sender))
    {
        irc_send_numericreply(ircdata, ERR_CHANOPRIVSNEEDED, chan_name);
        return OK;
    }

    irc_channel_broadcast(chan, ircdata->msg_tosend, NULL, ":%s PART %s %s", sender->nick, chan->name, kicked->nick);
    irc_channel_part(ircdata->globdata, chan, kicked);

    return OK;
}

int irc_time(void *data)
{
    /* Ignoramos la parte de servidores */
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    time_t t;
    char date_buf[30];
    time(&t);

    ctime_r(&t, date_buf);
    date_buf[strlen(date_buf) - 1] = '\0'; /* Eliminamos último \n */
    irc_send_numericreply_withtext(ircdata, RPL_TIME, ircdata->globdata->servername, date_buf);

    return OK;
}

int irc_notice(void *data)
{
    return _irc_internal_msg(data, MSG_NOTICE);
}

int irc_pong(void *data)
{
    /* Quizás en un futuro queramos hacer algo con esto. Pero de momento, no. */
    return OK;
}

int irc_users(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    irc_send_numericreply(ircdata, ERR_USERSDISABLED, NULL); /* De momento. */
    return OK;
}

int irc_oper(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircuser *user;
    char *params[2];
    char *username, *pass;
    char *actual_pass;

    if (irc_parse_paramlist(ircdata->msg, params, 2) != 2)
    {
        irc_send_numericreply(ircdata, ERR_NEEDMOREPARAMS, "OPER");
        return OK;
    }

    user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    username = params[0];
    pass = params[1];

    actual_pass = dic_lookup(ircdata->globdata->oper_passwords, username);

    if (actual_pass == NULL || strncmp(actual_pass, pass, MAX_KEY_LEN))
    {
        slog(LOG_WARNING, "Autenticación OPER fallida en socket %d por %s, user/pass %s/%s",
             ircdata->msgdata->fd, user->nick, username, pass);
        irc_send_numericreply(ircdata, ERR_PASSWDMISMATCH, NULL);
        return OK;
    }

    user->mode |= user_op;
    irc_send_numericreply(ircdata, RPL_YOUREOPER, NULL);
    return OK;
}

static int _try_getint(const char *str, int *value)
{
    int val = strtol(str, NULL, 10);

    if (!val && errno == EINVAL)
        return 0;

    *value = val;
    return 1;
}

int irc_mode(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircchan *chan;
    struct ircuser *user;
    char *params[3];
    int pnum;
    char *target, *mode, *param;
    short was_op, mode_add;
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
    struct ircflag user_flags[] =
    {
        {'i', user_invisible},
        {'s', user_rcvnotices},
        {'w', user_rcvwallops},
        {'o', user_op},
        IRCFLAGS_END
    };

    pnum = irc_parse_paramlist(ircdata->msg, params, 3);

    if (pnum < 2)
    {
        irc_send_numericreply(ircdata, ERR_NEEDMOREPARAMS, "MODE");
        return OK;
    }

    target = params[0];
    mode = params[1];
    param = params[2];

    mode_add = mode[0] == '+';

    user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);

    if (target[0] == '#' || target[0] == '&')
    {
        chan = irc_channel_byname(ircdata->globdata, target);

        if (!chan)
        {
            irc_send_numericreply(ircdata, ERR_NOSUCHNICK, target);
        }
        else if (!irc_is_channel_op(chan, user))
        {
            irc_send_numericreply(ircdata, ERR_CHANOPRIVSNEEDED, NULL);
        }
        else
        {
            irc_flagparse(mode, (int *) & (chan->mode), chan_flags);

            if (strchr(mode, 'l'))
            {
                if (mode_add && pnum < 3)
                    irc_send_numericreply(ircdata, ERR_NEEDMOREPARAMS, "MODE +l");
                else if (mode_add)
                    _try_getint(param, &(chan->user_limit));
                else
                    chan->user_limit = -1;
            }

            if (strchr(mode, 'k'))
            {
                if (mode_add && pnum < 3)
                    irc_send_numericreply(ircdata, ERR_NEEDMOREPARAMS, "MODE +k");
                else if (mode_add)
                    irc_set_channel_pass(chan, param);
                else
                    chan->has_password = 0;
            }
        }
    }
    else
    {
        was_op = user->mode & user_op;

        if (!strncmp(user->nick, target, MAX_NICK_LEN))
        {
            irc_send_numericreply(ircdata, ERR_USERSDONTMATCH, target);
        }
        else
        {
            irc_flagparse(mode, (int *) & (user->mode), user_flags);

            if (!was_op && (user->mode & user_op))
            {
                slog(LOG_WARNING, "El usuario %s ha tratado de hacerse operador a través del comando MODE.", target);
                user->mode &= ~(user_op);
            }
        }
    }

    return OK;
}

int irc_invite(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircuser *invited, *user;
    struct ircchan *chan;
    char *params[2];

    if (irc_parse_paramlist(ircdata->msg, params, 2) != 2)
    {
        irc_send_numericreply(ircdata, ERR_NEEDMOREPARAMS, "INVITE");
        return OK;
    }

    user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    invited = irc_user_bynick(ircdata->globdata, params[0]);
    chan = irc_channel_byname(ircdata->globdata, params[1]);

    if (!invited)
        irc_send_numericreply(ircdata, ERR_NOSUCHNICK, params[0]);
    else if (chan != NULL && irc_user_inchannel(chan, invited))
        irc_send_numericreply(ircdata, ERR_USERONCHANNEL, params[0]);
    else if (chan != NULL && (chan->mode & chan_invite) && !irc_is_channel_op(chan, user))
        irc_send_numericreply(ircdata, ERR_CHANOPRIVSNEEDED, params[1]);
    else if (invited->is_away)
        irc_send_numericreply_withtext(ircdata, RPL_AWAY, params[0], invited->away_msg);
    else
    {
        irc_send_numericreply_withtext(ircdata, RPL_INVITING, params[0], params[1]);
        list_add(ircdata->msg_tosend, irc_response_create(invited->fd, ":%s INVITE %s %s", user->nick, params[0], params[1]));

        if (chan->mode & chan_invite)
            dic_add(chan->invited_users, invited->nick, invited);
    }

    return OK;
}

int irc_version(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    char versionmsg[MAX_IRC_MSG];

#ifdef DEBUG
    char debug[] = "DEBUG";
#else
    char debug[] = "RELEASE";
#endif

    snprintf(versionmsg, MAX_IRC_MSG, "%d.%s %s", SERVER_VERSION, debug, ircdata->globdata->servername);
    irc_send_numericreply_withtext(ircdata, RPL_VERSION, versionmsg, "Servidor de IRC - Redes II EPS UAM- Guillermo Julián / Víctor de Juan");

    return OK;
}

int irc_kill(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircuser *tokill, *user;
    char *params[2];

    if (irc_parse_paramlist(ircdata->msg, params, 2) != 2)
    {
        irc_send_numericreply(ircdata, ERR_NEEDMOREPARAMS, "KILL");
        return OK;
    }

    user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    tokill = irc_user_bynick(ircdata->globdata, params[0]);

    if (!tokill)
    {
        irc_send_numericreply(ircdata, ERR_NOSUCHNICK, params[0]);
        return OK;
    }

    if (!(user->mode & user_op))
    {
        irc_send_numericreply(ircdata, ERR_NOPRIVILEGES, "KILL");
        return OK;
    }

    irc_create_quit_messages(tokill, ircdata->msg_tosend, "Killed by operator");
    list_add(ircdata->msg_tosend, irc_response_create(tokill->fd, ":%s KILL %s :%s", user->nick, params[0], params[1]));

    ircdata->connection_to_terminate = tokill->fd;

    return OK;
}

int irc_away(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircuser* user;
    char* params[1];

    user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);

    if (irc_parse_paramlist(ircdata->msg, params, 1) == 0)
    {
        user->is_away = 0;
        irc_send_numericreply(ircdata, RPL_UNAWAY, NULL);
    }
    else
    {
        user->is_away = 1;
        strncpy(user->away_msg, params[0], MAX_AWAYMSG_LEN);
        irc_send_numericreply(ircdata, RPL_NOWAWAY, NULL);
    }

    return OK;
}

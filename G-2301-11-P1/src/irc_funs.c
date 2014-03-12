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
	struct ircchan* chan = irc_channel_byname(irc->globdata, receiver);
	struct ircuser* sender = irc_user_byid(irc->globdata, irc->msgdata->fd);
    char* cmd_name = msgtype == MSG_PRIVMSG ? "PRIVMSG" : "NOTICE";

	if(!chan)
	{
        if(msgtype == MSG_PRIVMSG)
            irc_send_numericreply(irc, ERR_NOSUCHNICK, receiver);
        return OK;
    }

    irc_channel_broadcast(chan, irc->msg_tosend, ":%s %s %s :%s", sender->nick, cmd_name, receiver, text);

 	return OK;
}

static int _irc_send_msg_touser(struct irc_msgdata *irc, const char *receiver, const char *text, int msgtype)
{
    struct ircuser *dest = irc_user_bynick(irc->globdata, receiver);
    struct ircuser* sender = irc_user_byid(irc->globdata, irc->msgdata->fd);
    char* cmd_name = msgtype == MSG_PRIVMSG ? "PRIVMSG" : "NOTICE";

    if (!dest && msgtype == MSG_PRIVMSG)
        irc_send_numericreply(irc, ERR_NOSUCHNICK, receiver);
    else if(dest->is_away && msgtype == MSG_PRIVMSG)
        irc_send_numericreply_withtext(irc, RPL_AWAY, receiver, dest->away_msg);
    else if(dest && !dest->is_away)
    	list_add(irc->msg_tosend, irc_response_create(dest->fd, ":%s %s %s :%s", sender->nick, cmd_name, receiver, text));

    return OK;
}

int _irc_internal_msg(void* data, int msgtype)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    char* params[2];
    int param_num;
    char *dests, *text, *receiver;

    param_num = irc_parse_paramlist(ircdata->msgdata->data, params, 2);

    if(param_num < 1)
    {
        irc_send_numericreply(ircdata, ERR_NORECIPIENT, NULL);
        return OK;
    }
    else if(param_num < 2)
    {
        irc_send_numericreply(ircdata, ERR_NOTEXTTOSEND, NULL);
        return OK;
    }

    dests = params[0];
    text = params[1];

    while ((receiver = strsep(&dests, ",")) != NULL)
    {
        if(receiver[0] == '#' || receiver[0] == '&')
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

    if(user)
        strncpy(user->name, params[3],MAX_NAME_LEN);
    
    return OK;
}

int irc_join(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircchan *channel;
    char topic[MAX_TOPIC_LEN], bye_msg[MAX_IRC_MSG + 1];
    char *params[2];
    char *chan_name, *aux_name, *key, *aux_key;
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
        sprintf(bye_msg, "No has podido unirte al canal porque no eres un usuario");
        list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_NOTFOUND, bye_msg));
        return ERR_NOTFOUND;
    }
    chan_name = params[0];
    key = params[1];

    while (!chan_name)
    {

        aux_name = strchr(chan_name, ',');
        if (!aux_name)
        {
            *aux_name = '\0';
            aux_name++;
        }

        if (!key)
            aux_key = strchr(key, ',');
        else
            aux_key = NULL;

        if (!aux_key)
        {
            *aux_key = '\0';
            aux_key++;
        }

        /* TODO: ¿Qué pasa si aux_name es NULL? */
        channel = irc_channel_byname(ircdata->globdata, aux_name);

        if(!channel)
        	list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_NOSUCHCHANNEL, aux_name));

        retval = irc_channel_adduser(ircdata->globdata, channel, user, aux_key);
        /*  Respuesta al cliente:   */

        if (retval != OK) /* Pasando de interpretar errores... ¿no? */
            sprintf(bye_msg, "No te has podido unir al canal %s, porque %s", chan_name, irc_errstr(retval));
        else
            sprintf(bye_msg, "Te has unido al canal %s cuyo tema es: %s y está formado por: ", chan_name, topic);

        list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, 0, bye_msg));

        chan_name = aux_name;
        key = aux_key;
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
        list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata,ERR_NOTREGISTERED, NULL));
        return OK;
    }

    irc_parse_paramlist(ircdata->msg, params, 1);
    
    channum = str_arrsep(params[0], ",", chans, 50);

    for(i = 0; i < channum; i++)
    {
        channel_name = chans[i];
        channel = irc_channel_byname(ircdata->globdata, channel_name);

        if (!channel)
            irc_send_numericreply(ircdata, ERR_NOSUCHCHANNEL, channel_name);
        else if (irc_user_inchannel(channel, user) != OK)
            irc_send_numericreply(ircdata, ERR_USERNOTINCHANNEL, NULL);
        else
        {
            irc_channel_part(ircdata->globdata, channel, user);
            irc_channel_broadcast(channel, ircdata->msg_tosend, ":%s PART %s", user->nick, channel_name);
        }
    }

    return OK;
}

/** Pendientes **/
int irc_topic(void *data)
{
    char* params[2];
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircchan* chan;
    char* chan_name, *topic;
    struct ircuser* user;
    int pnum;

    pnum = irc_parse_paramlist(ircdata->msgdata->data, params, 2);
    user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);

    if(pnum == 0)
    {
        list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_NEEDMOREPARAMS, "TOPIC"));
        return OK;
    }

    chan_name = params[0];
    chan = irc_channel_byname(ircdata->globdata, chan_name);

    if(chan == NULL || irc_user_inchannel(chan, user) == ERR_NOTFOUND)
        return irc_send_numericreply(ircdata, ERR_NOTONCHANNEL, chan_name);

    if((chan->mode & chan_topiclock) && !irc_is_channel_op(chan, user))
    {
        irc_send_numericreply(ircdata, ERR_CHANOPRIVSNEEDED, chan_name);
        return OK;
    }

    if(pnum == 1) /* Sólo nos piden el canal */
    {
        if(strnlen(chan->topic, MAX_TOPIC_LEN) == 0)
            irc_send_numericreply(ircdata, RPL_NOTOPIC, chan_name);
        else
            irc_send_numericreply_withtext(ircdata, RPL_TOPIC, chan_name, chan->topic);
    }
    else
    {
        topic = params[1];
        strncpy(chan->topic, topic, MAX_TOPIC_LEN);
        irc_channel_broadcast(chan, ircdata->msg_tosend, ":%s TOPIC %s :%s", user->nick, chan_name, topic);
    }

    return OK;
}

int irc_names(void *data)
{
    char* params[1];
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircchan* chan;
    struct ircuser* user;
    struct ircuser* source = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    char* chanlist = NULL;
    list* users;
    int i, j;

    if(irc_parse_paramlist(ircdata->msgdata->data, params, 1) == 1)
        chanlist = params[0];

    for(i = 0; i < list_count(ircdata->globdata->chan_list); i++)
    {
        chan = list_at(ircdata->globdata->chan_list, i);

        if((!(chan->mode & (chan_priv | chan_secret)) || irc_user_inchannel(chan, source) == OK) /* No mostramos canales ni secretos ni privados si el usuario no está en ellos */
            && (chanlist == NULL || strnstr(chanlist, chan->name, MAX_IRC_MSG) == 0)) /* Si el usuario ha especificado un canal, mostrar sólo esos */
        {
            users = chan->users;
            for(j = 0; j < list_count(users); j++)
            {
                user = list_at(users, j);
                irc_send_numericreply_withtext(ircdata, RPL_NAMREPLY, chan->name, user->nick);
            }

            irc_send_numericreply(ircdata, RPL_ENDOFNAMES, chan->name);
        }
    }

    return OK;
}

int irc_list(void *data)
{   
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircchan* chan;
    struct ircuser* user;
    struct ircuser* source = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    char* chanlist = NULL;
    char* params[1];
    list* users;
    int i, j;

    if(irc_parse_paramlist(ircdata->msgdata->data, params, 1) == 1)
        chanlist = params[0];

    for(i = 0; i < list_count(ircdata->globdata->chan_list); i++)
    {
        chan = list_at(ircdata->globdata->chan_list, i);

        if((!(chan->mode & (chan_priv | chan_secret)) || irc_user_inchannel(chan, source) == OK) /* No mostramos canales ni secretos ni privados si el usuario no está en ellos */
            && (chanlist == NULL || strnstr(chanlist, chan->name, MAX_IRC_MSG) == 0)) /* Si el usuario ha especificado un canal, mostrar sólo esos */
        {
            users = chan->users;
            for(j = 0; j < list_count(users); j++)
            {
                user = list_at(users, j);
                irc_send_numericreply_withtext(ircdata, RPL_NAMREPLY, chan->name, user->nick);
            }

            irc_send_numericreply(ircdata, RPL_ENDOFNAMES, chan->name);
        }
    }

    return OK;
}

int irc_kick(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircuser *sender, *kicked;
    struct ircchan* chan;
    char *params[3];
    char *chan_name, *username;
    int pnum;

    pnum = irc_parse_paramlist(ircdata->msgdata->data, params, 3);

    if(pnum < 2)
    {
        irc_send_numericreply(ircdata, ERR_NEEDMOREPARAMS, "KICK");
        return OK;
    }

    chan_name = params[0];
    username = params[1];

    chan = irc_channel_byname(ircdata->globdata, chan_name);

    if(!chan)
    {
        irc_send_numericreply(ircdata, ERR_NOSUCHCHANNEL, chan_name);
        return OK;
    }

    sender = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    kicked = irc_user_bynick(ircdata->globdata,username);

    if(!kicked || !irc_user_inchannel(chan, kicked))
    {
        irc_send_numericreply(ircdata, ERR_NOTONCHANNEL, chan_name); /* No estoy seguro de esto */
        return OK;
    }

    if(!irc_is_channel_op(chan, sender))
    {
        irc_send_numericreply(ircdata, ERR_CHANOPRIVSNEEDED, chan_name);
        return OK;
    }

    irc_channel_broadcast(chan, ircdata->msg_tosend, ":%s PART %s %s", sender->nick, chan->name, kicked->nick);
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

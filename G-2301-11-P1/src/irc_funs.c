#include "irc_funs.h"
#include "irc_core.h"
#include "irc_codes.h"
#include "irc_processor.h"
#include "log.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>

static int _irc_send_msg_tochan(struct irc_msgdata *irc, const char *receiver, const char *text)
{
	int i;
	struct ircchan* chan = irc_channel_byname(irc->globdata, receiver);
	struct ircuser* sender = irc_user_byid(irc->globdata, irc->msgdata->fd);
	struct ircuser* user;

	if(!chan)
	{
		list_add(irc->msg_tosend, irc_build_numericreply(irc, ERR_NOSUCHNICK, receiver));
 		return OK;
 	}

 	for(i = 0; i < list_count(chan->users); i++)
 	{
 		user = list_at(chan->users, i);
 		list_add(irc->msg_tosend, irc_response_create(user->fd, ":%s PRIVMSG %s :%s", sender->nick, receiver, text));
 	}

 	return OK;
}

static int _irc_send_msg_touser(struct irc_msgdata *irc, const char *receiver, const char *text)
{
    struct ircuser *dest = irc_user_bynick(irc->globdata, receiver);
    struct ircuser* sender = irc_user_byid(irc->globdata, irc->msgdata->fd);

    if (!dest)
    	list_add(irc->msg_tosend, irc_build_numericreply(irc, ERR_NOSUCHNICK, receiver));
    else if(dest->is_away)
    	list_add(irc->msg_tosend, irc_build_numericreply_withtext(irc, RPL_AWAY, receiver, dest->away_msg));
    else
    	list_add(irc->msg_tosend, irc_response_create(dest->fd, ":%s PRIVMSG %s :%s", sender->nick, receiver, text));

    return OK;
}

int irc_privmsg(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    char* params[2];
    int param_num;
    char *dests, *text, *receiver;

    param_num = irc_parse_paramlist(ircdata->msgdata->data, params, 2);

    if(param_num < 1)
    {
    	list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_NORECIPIENT, NULL));
    	return OK;
    }
    else if(param_num < 2)
    {
    	list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_NOTEXTTOSEND, NULL));
    	return OK;
    }

    dests = params[0];
    text = params[1];

    while ((receiver = strsep(&dests, ",")) != NULL)
    {
    	if(receiver[0] == '#' || receiver[0] == '&')
    		_irc_send_msg_tochan(ircdata, receiver, text);
    	else
    		_irc_send_msg_touser(ircdata, receiver, text);

    }

    return OK;
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
        list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_UNKNOWNCOMMAND, NULL));
        return ERR;
    }

    retval = irc_set_usernick(ircdata->globdata, ircdata->msgdata->fd, new_nick[0]);

    if (retval != OK)
    {
        if (retval == ERR_NOTFOUND)
            list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_ERRONEUSNICKNAME, NULL));
        else if (retval == ERR_NICKCOLLISION)
            list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_NICKCOLLISION, NULL));
        else
            slog(LOG_ERR, "Error desconocido %d al cambiar nick del usuario a %s", retval, new_nick[0]);
    }

    return OK;
}

int irc_user(void *data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    char *params[1];
    struct ircuser *user;

    if (irc_parse_paramlist(ircdata->msg, params, 4) < 4)
    {
        list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_NEEDMOREPARAMS, NULL));
        return ERR;
    }

    user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    if(!user){
        if (strlen(params[0]) >= MAX_NAME_LEN)
        {
            list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata,ERR, NULL));
            return ERR;
        }

        strcpy(user->name, params[0]);

        return OK;
    }  
    else
        return ERR_NOTREGISTERED;
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
    int retval;
    char *channel_name, *aux;
    char *params[1];
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;
    struct ircuser *user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
    struct ircchan *channel;

    if (!user)
    {
        list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata,ERR_NOTREGISTERED, NULL));
        return ERR_NOTREGISTERED;
    }

    irc_parse_paramlist(ircdata->msg, params, 1);
    channel_name = params[0];
    while (!channel_name)
    {
        aux = strchr(channel_name, ',');
        if (!aux)
        {
            aux++;
            *aux = '\0';
        }
        channel = irc_channel_byname(ircdata->globdata, channel_name);
        if (!channel)
            list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_NOSUCHCHANNEL, NULL));
        else if (irc_user_inchannel(channel, user) != OK)
            list_add(ircdata->msg_tosend, irc_build_numericreply(ircdata, ERR_USERNOTINCHANNEL, NULL));
        else
        {
            retval = irc_channel_part(ircdata->globdata, channel, user);
            if ( retval != OK)
                slog(LOG_ALERT, "No se ha podido eliminar al usuario %s del canal %s", user->nick, channel->name);

        }
        channel_name = aux;
    }

    return OK;
}

/** Pendientes **/
int irc_topic(void *data)
{
    return OK;
}
int irc_names(void *data)
{
    return OK;
}
int irc_list(void *data)
{
    return OK;
}
int irc_kick(void *data)
{
    return OK;
}
int irc_time(void *data)
{
    return OK;
}
int irc_notice(void *data)
{
    return OK;
}
int irc_pong(void *data)
{
    return OK;
}
int irc_users(void *data)
{
    return OK;
}

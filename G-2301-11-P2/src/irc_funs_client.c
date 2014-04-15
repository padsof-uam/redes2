#include "irc_funs_client.h"
#include "gui_client.h"
#include "irc_processor.h"
#include "log.h"

#include <string.h>

extern struct irc_clientdata* client;

int irc_default(void * data){
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	messageText("server: %s", msgdata->msg);
	return OK;
}

int irc_recv_topic(void* data)
{
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	char* params[2];
	
	if(irc_parse_paramlist(msgdata->msg, params, 2) < 2)
	{
		slog(LOG_ERR, "Respuesta RPL_TOPIC mal formada: %s", msgdata->msg);
		return OK;
	}

	messageText("El tema del canal \"%s\" es : %s", params[0], params[1]);

	return OK;
}

int irc_recv_join(void* data)
{
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	char user[MAX_NICK_LEN + 1];
	char* params[1];

	if(irc_get_prefix(msgdata->msg, user, MAX_NICK_LEN + 1) != OK || irc_parse_paramlist(msgdata->msg, params, 1) != 1)
	{
		slog(LOG_ERR, "Mensaje JOIN mal formado: %s", msgdata->msg);
		return OK;
	}	

	if(!strncmp(user, client->nick, MAX_NICK_LEN))
	{
		client->in_channel = 1;
		strncpy(client->chan, params[0], MAX_CHAN_LEN);
	}

	messageText("%s ha entrado en el canal \"%s\"", user, params[0]);

	return OK;
}

int irc_recv_part(void* data)
{
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	char user[MAX_NICK_LEN + 1];
	char* params[1];

	if(irc_get_prefix(msgdata->msg, user, MAX_NICK_LEN + 1) != OK || irc_parse_paramlist(msgdata->msg, params, 1) != 1)
	{
		slog(LOG_ERR, "Mensaje PART mal formado: %s", msgdata->msg);
		return OK;
	}	

	if(!strncmp(user, client->nick, MAX_NICK_LEN))
	{
		client->in_channel = 0;
	}

	messageText("%s ha salido del canal \"%s\"", user, params[0]);

	return OK;
}

int irc_recv_privmsg(void* data)
{
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	char user[MAX_NICK_LEN];
	char* params[2];

	if(irc_get_prefix(msgdata->msg, user, MAX_NICK_LEN) != OK || 
		irc_parse_paramlist(msgdata->msg, params, 2) != 2)
	{
		slog(LOG_ERR, "Mensaje PRIVMSG mal formado: %s", msgdata->msg);
		return OK;
	}	

	publicText(user, params[1]);

	return OK;	
}

int irc_needmoreparams(void* data)
{
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	char* params[2];
	
	if(irc_parse_paramlist(msgdata->msg, params, 2) < 2)
	{
		slog(LOG_ERR, "Respuesta ERR_NEEDMOREPARAMS mal formada: %s", msgdata->msg);
		return OK;
	}

	errorText("Se necesitan más parámetros para el comando %s", params[0]);

	return OK;
}

int irc_nickcollision(void* data)
{
	errorText("El nick que has escogido (%s) está en uso. Elige otro con el comando \"/nick tunuevonick\"", client->nick);
	client->connected = 0;
	return OK;
}

int irc_recv_nick(void* data)
{
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	char prefix[MAX_NICK_LEN + 1];
	char* params[1];

	irc_get_prefix(msgdata->msg, prefix, MAX_NICK_LEN + 1);

	if(irc_parse_paramlist(msgdata->msg, params, 1) != 1)
	{
		slog(LOG_ERR, "Mensaje NICK mal formado: %s", msgdata->msg);
		return OK;
	}

	messageText("%s cambió su apodo a %s", prefix, params[0]);

	if(!strncmp(prefix, client->nick, MAX_NICK_LEN))
	{
		client->connected = 1;
		setApodo(prefix);
	}

	return OK;
}
	

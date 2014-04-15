#include "irc_funs_client.h"
#include "gui_client.h"
#include "irc_processor.h"
#include "log.h"

#include <string.h>


int irc_default(void * data){
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	noticeText("server: %s", msgdata->msg);
	return OK;
}

int irc_recv_topic(void* data)
{
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	char* params[3];
	
	if(irc_parse_paramlist(msgdata->msg, params, 3) < 3)
	{
		slog(LOG_ERR, "Respuesta RPL_TOPIC mal formada: %s", msgdata->msg);
		return OK;
	}

	messageText("El tema del canal \"%s\" es : %s", params[1], params[2]);

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

	if(!strncmp(user, msgdata->clientdata->nick, MAX_NICK_LEN))
	{
		msgdata->clientdata->in_channel = 1;
		strncpy(msgdata->clientdata->chan, params[0], MAX_CHAN_LEN);
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

	if(!strncmp(user, msgdata->clientdata->nick, MAX_NICK_LEN))
	{
		msgdata->clientdata->in_channel = 0;
	}

	messageText("%s ha salido del canal \"%s\"", user, params[0]);

	return OK;
}

int irc_recv_privmsg(void* data)
{
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	char user[MAX_NICK_LEN + 1];
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
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	
	errorText("El nick que has escogido (%s) está en uso. Elige otro con el comando \"/nick tunuevonick\"", msgdata->clientdata->nick);
	msgdata->clientdata->connected = 0;
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

	if(!strncmp(prefix, msgdata->clientdata->nick, MAX_NICK_LEN))
	{
		msgdata->clientdata->connected = 1;
		setApodo(prefix);
	}

	return OK;
}
	
int irc_recv_quit(void* data)
{
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	char prefix[MAX_NICK_LEN + 1];
	char* params[1];

	irc_get_prefix(msgdata->msg, prefix, MAX_NICK_LEN + 1);

	if(irc_parse_paramlist(msgdata->msg, params, 1) != 1)
	{
		slog(LOG_ERR, "Mensaje QUIT mal formado: %s", msgdata->msg);
		return OK;
	}

	messageText("%s ha salido del canal: %s", prefix, params[0]);

	if(!strncmp(prefix, msgdata->clientdata->nick, MAX_NICK_LEN))
		msgdata->clientdata->connected = 0;

	return OK;
}

int irc_recv_notice(void* data)
{
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;
	char* params[2];

	if(irc_parse_paramlist(msgdata->msg, params, 2) != 2)
	{
		slog(LOG_ERR, "Mensaje NOTICE mal formado: %s", msgdata->msg);
		return OK;
	}	

	noticeText("server: %s", params[1]);

	return OK;	
}

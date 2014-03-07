#include "irc_funs.h"
#include "irc_core.h"
#include "irc_codes.h"
#include "irc_processor.h"

#include <sys/syslog.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static struct sockcomm_data* _irc_build_msg_for(const char* receiver, const char* text, struct irc_msgdata* irc)
{
	struct sockcomm_data* scdata = malloc(sizeof(struct sockcomm_data));
	struct ircuser* dest = irc_user_bynick(irc->globdata, receiver);

	if(dest && !dest->is_away)
	{
		scdata->fd = dest->fd;
		scdata->len = sprintf(scdata->data, "PRIVMSG %s: %s", receiver, text);
	}
	else
	{
		/* TODO: Pues eso */
		syslog(LOG_WARNING, "Deberíamos plantearnos controlar este caso.");
	}

	return scdata;
}

int irc_privmsg(void *data)
{
	struct irc_msgdata* ircdata = (struct irc_msgdata*) data;
	struct sockcomm_data* msg_tosend;
	char separators[] = ",";
	char* param_str = strchr(ircdata->msgdata->data, ' ');
	char *param_str_end, *receiver, *msgstart;

	if(param_str == NULL)
		return ERR_PARSE;

	param_str++; /* Apuntamos a la posición de inicio de los parámetros */
	param_str_end = strchr(param_str, ' ');
	*param_str_end = '\0'; /* Marcamos fin de la lista de parámetros */
	param_str_end++;
	msgstart = strchr(param_str_end, ':');

	if(msgstart == NULL)
		return ERR_PARSE;

	msgstart++; /* Marcamos inicio del texto del mensaje */

	while((receiver = strsep(&param_str, separators)) != NULL)
	{
		msg_tosend = _irc_build_msg_for(receiver, msgstart, ircdata);

		if(msg_tosend)
			list_add(ircdata->msg_tosend, msg_tosend);
	}

	return OK;
}

int irc_ping(void *data)
{
	struct irc_msgdata* ircdata = (struct irc_msgdata*) data;
	struct sockcomm_data* msg_tosend = malloc(sizeof(struct sockcomm_data));

	strcpy(msg_tosend->data, "PONG\r\n");
	msg_tosend->len = strlen(msg_tosend->data);
	msg_tosend->fd = ircdata->msgdata->fd;

	list_add(ircdata->msg_tosend, msg_tosend);

	return OK;
}

int irc_nick(void *data)
{
	int retval;
	struct irc_msgdata* ircdata = (struct irc_msgdata*) data;
	char * new_nick[1];

	if (!irc_parse_paramlist(ircdata->msg,new_nick, 1)){
		list_add(ircdata->msg_tosend, irc_build_errmsg(ERR_UNKNOWNCOMMAND,ircdata,NULL));
		return ERR;
	}

	retval = irc_set_usernick(ircdata->globdata, ircdata->msgdata->fd, new_nick[0]);
	
	if (retval != OK)
	{
		if(retval == ERR_NOTFOUND)
			list_add(ircdata->msg_tosend, irc_build_errmsg(ERR_ERRONEUSNICKNAME,ircdata,NULL));
		else if(retval == ERR_REPEAT)
			list_add(ircdata->msg_tosend, irc_build_errmsg(ERR_NICKCOLLISION,ircdata,NULL));
		else
			syslog(LOG_ERR, "Error desconocido %d al cambiar nick del usuario a %s", retval, new_nick[0]);
	}	
	
	return OK;
}

int irc_user(void *data)
{
	struct irc_msgdata* ircdata = (struct irc_msgdata*) data;	
	char * params[1];
	struct ircuser * user;

	if (irc_parse_paramlist(ircdata->msg, params, 4) < 4)
	{
		list_add(ircdata->msg_tosend, irc_build_errmsg(ERR_NEEDMOREPARAMS,ircdata,NULL));
		return ERR;
	}

	user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
	if (strlen(params[0])>= MAX_NAME_LEN)
	{
		list_add(ircdata->msg_tosend, irc_build_errmsg(ERR_ERRONEUSNICKNAME,ircdata,NULL));
		return ERR;
	}

	strcpy(user->name,params[0]);
	return OK;
}

int irc_join(void * data)
{
	struct irc_msgdata* ircdata = (struct irc_msgdata*) data;
	struct ircchan * channel;
	char topic[MAX_TOPIC_LEN],bye_msg[MAX_IRC_MSG+1];
	char * params[2];
	char * chan_name, *aux_name,*key,*aux_key;
	struct ircuser * user;
	int retval = OK;

	if(irc_parse_paramlist(ircdata->msg, params, 2)==0)
	{
		list_add(ircdata->msg_tosend,irc_build_errmsg(ERR_NEEDMOREPARAMS,ircdata,NULL));
		return ERR;
	}


	user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);
	if(!user)
	{
		sprintf(bye_msg, "No has podido unirte al canal porque no eres un usuario");
		irc_build_errmsg(ERR_NOTFOUND,ircdata,bye_msg);
		return ERR_NOTFOUND;
	}
	chan_name = params[0];
	key = params[1];

	while(!chan_name){

		aux_name = strchr(chan_name, ',');
		if (!aux_name)
		{
			*aux_name='\0';
			aux_name++;
		}

		if(!key)
			aux_key = strchr(key,',');
		else
			aux_key = NULL;
		
		if (!aux_key)
		{
			*aux_key='\0';
			aux_key++;
		}

		retval = irc_channel_adduser(ircdata->globdata, chan_name , user , key,channel);

		/*	Respuesta al cliente:	*/

		if(retval != OK) /* Pasando de interpretar errores... ¿no? */
			sprintf(bye_msg, "No te has podido unir al canal %s, porque %s",chan_name,_irc_errmsg(retval));
		else
			sprintf(bye_msg, "Te has unido al canal %s cuyo tema es: %s y está formado por: ",chan_name,topic);
		
		list_add(ircdata->msg_tosend, irc_build_errmsg(0,ircdata, bye_msg));

		chan_name = aux_name;
		key = aux_key;
	}

	return OK;
}

int irc_quit(void* data)
{
	struct irc_msgdata* ircdata = (struct irc_msgdata*) data;
	struct ircuser* user;
	char* bye_msg;
	char* params[1];

	user = irc_user_byid(ircdata->globdata, ircdata->msgdata->fd);

	if(!user)
		return ERR_NOTFOUND;

	if(irc_parse_paramlist(ircdata->msg, params, 1) == 0)
		bye_msg = user->nick; /* Mensaje de salida por defecto */
	else
		bye_msg = params[0];

	irc_create_quit_messages(user, ircdata->msg_tosend, bye_msg);

	return OK;
}

/** Pendientes **/
int irc_part(void* data)
{
	return OK;
}
int irc_topic(void* data)
{
	return OK;
}
int irc_names(void* data)
{
	return OK;
}
int irc_list(void* data)
{
	return OK;
}
int irc_kick(void* data)
{
	return OK;
}
int irc_time(void* data)
{
	return OK;
}
int irc_notice(void* data)
{
	return OK;
}
int irc_pong(void* data)
{
	return OK;
}
int irc_users(void* data)
{
	return OK;
}

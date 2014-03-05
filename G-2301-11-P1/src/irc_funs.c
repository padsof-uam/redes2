#include "irc_funs.h"
#include "irc_core.h"
#include "irc_codes.h"
#include "types.h"
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
	char* old_nick,*new_nick;
	struct ircuser* new_user,*old_user;

	if (!strncasecmp(ircdata->msgdata->data, "NICK", 4*sizeof(char)))
	{
		new_nick = strchr(ircdata->msgdata->data, ' ');	
		new_nick++;
		old_nick=NULL;
	}
	else if (!strncasecmp(ircdata->msgdata->data, ":", sizeof(char)))
	{
		old_nick = ircdata->msgdata->data;
		/* Ignoramos los ':' */
		old_nick++;

		new_nick = strchr(ircdata->msgdata->data, ' ');
		/* Este caso no debería ser posible, porque en new_nick está el comando NICK*/
		if (!new_nick)	/* Implicit function */
			list_add(ircdata->msg_tosend, irc_build_errmsg(ERR_NONICKNAMEGIVEN));
		
		/* Sustituimos el ' ' por \0 para marcar el final de la cadena. */
		old_nick[new_nick-old_nick-1]='\0';


		/* new_nick[0]=' ', por lo que incrementamos 1 para obtener algo útil de strchr*/
		new_nick++;
		new_nick = strchr(ircdata->msgdata->data, ' ');
		if (!new_nick)
		{ 
			list_add(ircdata->msg_tosend, irc_build_errmsg(ERR_NONICKNAMEGIVEN));
		}
		else
		{
			new_nick++;
			if (strlen(new_nick) >= MAX_NICK_LEN)
			{
				/* No se si es este error... NICKNAME inválido (demasiado largo) */		
				list_add(ircdata->msg_tosend, irc_build_errmsg(ERR_ERRONEUSNICKNAME));
			}
		}
	}
	else
	{
		/* ¿Esta posibilidad acaso existe? */
	}

	retval = irc_set_usernick(ircdata->globdata, ircdata->msgdata->fd, new_nick);
	
	if (retval != OK)
	{
		if(retval == ERR_NOTFOUND)
			list_add(ircdata->msg_tosend, irc_build_errmsg(ERR_ERRONEUSNICKNAME));
		else if(retval == ERR_REPEAT)
			list_add(ircdata->msg_tosend, irc_build_errmsg(ERR_NICKCOLLISION));
		else
			syslog(LOG_ERR, "Error desconocido %d al cambiar nick del usuario a %s", retval, new_nick);
	}
	
	
	return OK;
}

int irc_user(void *data)
{
	struct irc_msgdata* ircdata = (struct irc_msgdata*) data;
	struct sockcomm_data* msg_tosend = malloc(sizeof(struct sockcomm_data));

	return OK;
}

int irc_quit(void* data)
{

}

int irc_join(void* data)
{

}

int irc_part(void* data)
{

}

int irc_topic(void* data)
{

}

int irc_names(void* data)
{

}

int irc_list(void* data)
{

}

int irc_kick(void* data)
{

}

int irc_time(void* data)
{

}

int irc_notice(void* data)
{

}

int irc_pong(void* data)
{

}

int irc_users(void* data)
{

}


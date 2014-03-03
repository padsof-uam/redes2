#include "irc_processor.h"
#include "commparser.h"
#include "irc_core.h"

#include <sys/syslog.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

const char* _irc_cmds[] = {
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

cmd_action _irc_actions[] = {
	irc_privmsg,
	irc_ping
};

void irc_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_globdata *gdata)
{
    struct irc_msgdata ircdata;
    char* msg;
    char* msg_end;
    int i;

    ircdata.globdata = gdata;
    ircdata.msgdata = data;
    ircdata.msg_tosend = list_new();

    msg = data->data;

    data->data[data->len] = '\0';

    while((msg_end = irc_msgsep(msg, MAX_IRC_MSG)) != NULL)
    {
    	ircdata.msg = msg;

    	if (parse_exec_command(msg, _irc_cmds, _irc_actions, (sizeof(_irc_actions) / sizeof(cmd_action)), &ircdata) == -1)
        	syslog(LOG_WARNING, "Error parsing command %s", data->data);

        msg = msg_end;
	}

	for(i = 0; i < list_count(ircdata.msg_tosend); i++)
		irc_enqueue_msg(list_at(ircdata.msg_tosend, i), snd_qid);

	list_destroy(ircdata.msg_tosend, free);
}

char* irc_msgsep(char* str, int len)
{
	char crlf[] = "\r\n";
    char* msgend;

    if(!str)
    	return NULL;

    msgend = strnstr(str, crlf, len);

    if(msgend)
    {
    	/* Quitamos el \r\n */
    	*msgend = '\0';
    	msgend++;
    	*msgend = '\0';
    	msgend++; 
    }

    return msgend;
}

void irc_enqueue_msg(struct sockcomm_data* msg, int snd_qid)
{
	struct msg_sockcommdata qdata;

	memcpy(&(qdata.scdata), msg, sizeof(struct sockcomm_data));
	qdata.msgtype = 1;

	if(msgsnd(snd_qid, &qdata, sizeof(struct msg_sockcommdata), 0) == -1)
		syslog(LOG_ERR, "error enviando en cola de mensajes de envío: %s", strerror(errno));
}

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

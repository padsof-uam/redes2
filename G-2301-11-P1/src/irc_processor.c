#include "irc_processor.h"
#include "commparser.h"
#include "irc_core.h"
#include "irc_codes.h"
#include "irc_funs.h"

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
    char* msg;
    char* msg_end;
    int i;

    ircdata.globdata = gdata;
    ircdata.msgdata = data;
    ircdata.msg_tosend = list_new();

    msg = data->data;
    msg[MAX_IRC_MSG] = '\0';
    msg = irc_remove_prefix(msg);

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


char* irc_remove_prefix(char* msg)
{
	char* space;

	if(!msg)
		return NULL;

	if(msg[0] != ':')
		return msg;

	space = strchr(msg, ' ');

	if(!space)
		return NULL;
	else
		return space + 1;
}



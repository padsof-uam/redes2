#include "irc_processor.h"
#include "commparser.h"

#include <sys/syslog.h>
#include <string.h>

const char* _irc_cmds[] = {
	"PRIVMSG",
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
	irc_privmsg
};

void irc_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_globdata *gdata)
{
    struct irc_msgdata ircdata;
    char* msg;
    char* msg_end;
    ircdata.globdata = gdata;
    ircdata.msgdata = data;

    msg = data->data;

    while((msg_end = irc_msgsep(msg, MAX_IRC_MSG)) != NULL)
    {
    	ircdata.msg = msg;

    	if (parse_exec_command(msg, _irc_cmds, _irc_actions, sizeof(_irc_actions), &ircdata) == -1)
        	syslog(LOG_WARNING, "Error parsing command %s", data->data);

        msg = msg_end;
	}
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


int irc_privmsg(void *data)
{
	struct irc_msgdata* ircdata = (struct irc_msgdata*) data;
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
		
	}
}

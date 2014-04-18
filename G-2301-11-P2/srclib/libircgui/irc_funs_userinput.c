#include "irc_funs_userinput.h"
#include "irc_processor.h"
#include "gui_client.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

extern int snd_qid;

const char* _ui_commands[] = 
{
	"msg",
	"nick",
	"me",
	"server",
	"*"
};

cmd_action _ui_actions[] = 
{
	irc_msg,
	irc_server_forward,
	irc_me,
	irc_server,
	irc_server_forward
};

static void irc_send_to_server(struct irc_msgdata* msgdata, const char* fmt_string, ...)
{
	va_list ap;
	struct sockcomm_data *msg;

    va_start(ap, fmt_string);
    msg = irc_response_vcreate(msgdata->clientdata->serv_sock, fmt_string, ap);
    va_end(ap);

    irc_enqueue_msg(msg, snd_qid);
}

void irc_user_msgprocess(const char* msg, struct irc_clientdata* cdata)
{
	struct sockcomm_data data;
	strncpy(data.data, msg, MAX_IRC_MSG);

	_irc_msgprocess(snd_qid, &data, NULL, cdata, _ui_commands, _ui_actions, (sizeof(_ui_actions)/sizeof(cmd_action)));
}

int irc_server_forward(void* data)
{
	struct irc_msgdata* msgdata = (struct irc_msgdata*) data;
	irc_send_to_server(msgdata, msgdata->msg);

	return OK;
}

int irc_msg(void* data)
{
	char *nick_start, *msg_start;
	struct irc_msgdata* msgdata = (struct irc_msgdata*) data;
	const char* msg = msgdata->msg;
	char user_dst[40];

	nick_start = strchr(msg, ' ');

	if(nick_start == NULL)
	{
		errorText("Sintaxis incorrecta. Uso: /msg nick (mensaje)");
		return ERR_PARSE;
	}

	nick_start++;

	msg_start = strchr(msg, ' ');

	if(msg_start == NULL)
	{
		errorText("Sintaxis incorrecta. Uso: /msg nick (mensaje)");
		return ERR_PARSE;
	}

	msg_start = '\0';
	msg_start++;

	irc_send_to_server(msgdata, "PRIVMSG %s :%s", nick_start, msg_start);

	snprintf(user_dst, 40, "%s -> %s", msgdata->clientdata->nick, nick_start);
	privateText(user_dst, msg);

	return OK;
}

int irc_me(void* data)
{
	struct irc_msgdata* msgdata = (struct irc_msgdata*) data;
	const char* msg = irc_next_param(msgdata->msg);

	if(!msgdata->clientdata->in_channel)
	{
		errorText("No estás en ningún canal.");
		return ERR_NOTFOUND;
	}	

	if(!msg)
	{
		errorText("Sintaxis incorrecta. Uso: /me (mensaje)");
		return ERR_PARSE;
	}

	irc_send_to_server(msgdata, "PRIVMSG %s :ACTION %s", msgdata->clientdata->chan, msg);
	actionText(msgdata->clientdata->nick, msg);

	return OK;
}

int irc_server(void* data)
{
	struct irc_msgdata* msgdata = (struct irc_msgdata*) data;
	char* params[2];
	int parnum;
	int servnum;
	char* port;

	parnum = irc_parse_paramlist(msgdata->msg, params, 2);

	if(parnum == 0)
	{
		connectToFavServ(0);
	}
	else if(parnum >= 1)
	{	
		servnum = strtol(params[0], NULL, 10);

		if(servnum != 0)
		{
			connectToFavServ(servnum - 1);
		}
		else
		{
			if(parnum == 2)
				port = params[1];
			else
				port = "6667";

			connectToServer(params[0], port);
		}
	}

	return OK;
}

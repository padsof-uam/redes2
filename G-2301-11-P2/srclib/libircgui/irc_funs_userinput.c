#include "irc_funs_userinput.h"
#include "irc_processor.h"
#include "gui_client.h"
#include "voicecall.h"
#include "log.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define CALL_TIMEOUT_SEC 10

extern int snd_qid; 

const char* _ui_commands[] = 
{
	"msg",
	"nick",
	"me",
	"server",
	"pcall",
	"paccept",
	"pclose",
	"quit",
	"ban",
	"*"
};

cmd_action _ui_actions[] = 
{
	irc_msg,
	irc_ui_nick,
	irc_me,
	irc_server,
	irc_pcall,
	irc_paccept,
	irc_pclose,
	irc_ui_quit,
	irc_ui_ban,
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

int *_call_socket;
int *_call_status;

static void _call_timeout(int sig)
{
	if(sig == SIGALRM)
	{
		close(*_call_socket);
		*_call_socket = -1;
		*_call_status = call_none;
		errorText("Tiempo de espera superado, cancelando llamada.");
	}
}

int irc_pcall(void* data)
{
	struct irc_msgdata* msgdata = (struct irc_msgdata*) data;
	char* user;
	int port;
	int socket;
	
	if(msgdata->clientdata->call_status != call_none)
	{
		errorText("Sólo puedes hacer una llamada a la vez.");
		return OK;
	}

	if(irc_parse_paramlist(msgdata->msg, &user, 1) != 1)
	{
		errorText("Error de sintaxis. Uso: /pcall <nick>");
		return OK;
	}

	socket = open_listen_socket();

	if(socket <= 0)
	{
		errorText("No se pudo crear el socket de escucha.", strerror(errno));
		slog(LOG_ERR, "No se pudo crear el socket de escucha, retorno %d. %s", socket, strerror(errno));
		return OK;
	}

	get_socket_port(socket, &port);
	irc_send_to_server(msgdata, "PRIVMSG %s :$PCALL %d %d", user, msgdata->clientdata->client_ip, port);
	messageText("Esperando respuesta de %s...", user);

	strncpy(msgdata->clientdata->call_user, user, MAX_NICK_LEN + 1);
	msgdata->clientdata->call_socket = socket;
	msgdata->clientdata->call_status = call_outgoing;

	_call_socket = &(msgdata->clientdata->call_socket);
	_call_status = (int*) &(msgdata->clientdata->call_status);

	signal(SIGALRM, _call_timeout);
	alarm(CALL_TIMEOUT_SEC);

	return OK;
}

int irc_paccept(void* data)
{
	struct irc_msgdata* msgdata = (struct irc_msgdata*) data;
	int socket;
	int port;

	if(msgdata->clientdata->call_status != call_incoming)
	{
		errorText("No tienes ninguna llamada pendiente.");
		return OK;
	}

	socket = open_listen_socket();

	if(socket <= 0)
	{
		errorText("No se pudo crear el socket de escucha.", strerror(errno));
		slog(LOG_ERR, "No se pudo crear el socket de escucha, retorno %d. %s", socket, strerror(errno));
		return OK;
	}

	get_socket_port(socket, &port);
	irc_send_to_server(msgdata, "PRIVMSG %s :$PACCEPT %d %d", msgdata->clientdata->call_user, msgdata->clientdata->client_ip, port);

	msgdata->clientdata->call_status = call_running;
	msgdata->clientdata->call_socket = socket;

	spawn_call_manager_thread(&(msgdata->clientdata->call_info), msgdata->clientdata->call_ip, msgdata->clientdata->call_port, socket, VC_FORMAT, VC_CHANNELS, VC_CHUNK_TIME_MS);
	messageText("Aceptando llamada...");
	return OK;
} 

int irc_pclose(void* data)
{
	struct irc_msgdata* msgdata = (struct irc_msgdata*) data;

	if(msgdata->clientdata->call_status == call_none)
	{
		errorText("No hay ninguna llamada que cancelar.");
		return OK;
	}


	irc_send_to_server(msgdata, "PRIVMSG %s :$PCLOSE ", msgdata->clientdata->call_user);
	
	if(msgdata->clientdata->call_status == call_running)
	{
		call_stop(&(msgdata->clientdata->call_info));
	}
	else if(msgdata->clientdata->call_status == call_outgoing)
	{
		close(msgdata->clientdata->call_socket);
		signal(SIGALRM, SIG_IGN);
	}
	
	msgdata->clientdata->call_status = call_none;
	messageText("Llamada terminada.");
	return OK;
} 

int irc_ui_quit(void* data)
{
	struct irc_msgdata* msgdata = (struct irc_msgdata*) data;
	const char* msg = irc_next_param(msgdata->msg);

	disconnectClient(msg);

	return OK;
}

int irc_ui_nick(void* data)
{
	struct irc_msgdata* msgdata = (struct irc_msgdata*) data;
	const char* nick = irc_next_param(msgdata->msg);

	setApodo(nick);
	strncpy(msgdata->clientdata->nick, nick, MAX_NICK_LEN);	

	irc_send_to_server(msgdata, msgdata->msg);

	return OK;
}

int irc_ui_ban(void* data)
{
	struct irc_msgdata* msgdata = (struct irc_msgdata*) data;
	char* msg_dup = strdup(msgdata->msg);
	char* params[2];

	if(irc_parse_paramlist(msg_dup, params, 2) != 2)
	{
		free(msg_dup);
		errorText("Error de sintaxis. Uso: /ban canal usuario");
		return OK;
	}

	irc_send_to_server(msgdata, "MODE %s +b %s", params[0], params[1]);
	free(msg_dup);

	return OK;
}


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

	strncpy(client->chan, params[0], MAX_CHAN_LEN);
	client->in_channel = 1;

	return OK;
}

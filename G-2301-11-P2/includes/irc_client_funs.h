#ifndef IRC_CLIENT_FUNS_H
#define IRC_CLIENT_FUNS_H

#include "types.h"
#include "gui_client.h"

int irc_default(void * data){
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;

	messageText("msg: %s",msgdata->msg);
	messageText("data: %s",msgdata->msgdata->data);
	
	return OK;
}

#endif
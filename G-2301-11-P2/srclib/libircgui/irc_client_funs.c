#include "irc_client_funs.h"


int irc_default(void * data){
	struct irc_msgdata * msgdata = (struct irc_msgdata *) data;

	messageText("msg: %s",msgdata->msg);
	messageText("data: %s",msgdata->msgdata->data);
	
	return OK;
}
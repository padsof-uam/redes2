#include "irc_processor.h"
#include "irc_funs_client.h"
#include "gui_client.h"

int irc_c_all(void* data)
{
    struct irc_msgdata *ircdata = (struct irc_msgdata *) data;	
	messageText(ircdata->msg);

	return OK;
}

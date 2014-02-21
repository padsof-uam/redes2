#include "irc_processor.h"
#include "commparser.h"

void irc_msgprocess(int snd_qid, struct sockcomm_data* data, struct irc_globdata* gdata) {
	struct irc_msgdata ircdata;
	
	ircdata.globdata = gdata;
	ircdata.msgdata = data;

	parse_exec_command(data->data, _irc_cmds, _irc_actions, sizeof(_irc_actions), &ircdata);
}



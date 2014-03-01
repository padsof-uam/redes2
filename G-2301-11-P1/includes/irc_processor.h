#ifndef IRC_PROCESSOR_H
#define IRC_PROCESSOR_H

#include "types.h"
#include "commparser.h"

int irc_privmsg(void* data);

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


void irc_msgprocess(int snd_qid, struct sockcomm_data* data, struct irc_globdata* gdata);
#endif


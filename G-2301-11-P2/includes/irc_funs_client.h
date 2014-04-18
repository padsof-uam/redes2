#ifndef IRC_CLIENT_FUNS_H
#define IRC_CLIENT_FUNS_H

#include "types.h"

int irc_default(void * data);
int irc_recv_topic(void* data);
int irc_recv_join(void* data);
int irc_recv_part(void* data);
int irc_recv_privmsg(void* data);
int irc_needmoreparams(void* data);
int irc_nickcollision(void* data);
int irc_recv_nick(void* data);
int irc_recv_quit(void* data);
int irc_recv_notice(void* data);
int irc_recv_mode(void* data);

#endif

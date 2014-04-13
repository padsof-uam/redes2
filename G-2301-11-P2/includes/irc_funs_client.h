#ifndef IRC_CLIENT_FUNS_H
#define IRC_CLIENT_FUNS_H

#include "types.h"

int irc_default(void * data);
int irc_recv_topic(void* data);
int irc_recv_join(void* data);
int irc_recv_part(void* data);
int irc_recv_privmsg(void* data);

#endif

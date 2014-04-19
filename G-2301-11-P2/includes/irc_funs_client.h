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
int irc_recv_ping(void* data);
int irc_recv_who(void* data);

void parse_pcall(struct irc_clientdata* cdata, char* text, char* source);
void parse_paccept(struct irc_clientdata* cdata, char* text, char* source);
void parse_pclose(struct irc_clientdata* cdata, char* text, char* source);

#endif

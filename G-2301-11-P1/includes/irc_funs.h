#ifndef IRC_FUNS_H
#define IRC_FUNS_H
#include "types.h"

int irc_privmsg(void* data);
int irc_ping(void* data);
int irc_nick(void* data);
int irc_user(void* data);
int irc_quit(void* data);
int irc_join(void* data);
int irc_part(void* data);
int irc_topic(void* data);
int irc_names(void* data);
int irc_list(void* data);
int irc_kick(void* data);
int irc_time(void* data);
int irc_notice(void* data);
int irc_pong(void* data);
int irc_users(void* data);
int irc_oper(void* data);
int irc_mode(void* data);
int irc_invite(void* data);
int irc_version(void* data);
int irc_kill(void* data);
int irc_away(void* data);
int irc_who(void* data);
int irc_ison(void* data);
#endif


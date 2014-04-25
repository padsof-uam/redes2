#ifndef IRC_FUNS_USERINPUT_H
#define IRC_FUNS_USERINPUT_H

#include "G-2301-11-P2-commparser.h"
#include "G-2301-11-P2-types.h"

void irc_user_msgprocess(const char* msg, struct irc_clientdata* cdata);
int irc_msg(void* data);
int irc_me(void* data);
int irc_server_forward(void* data);
int irc_server(void* data);
int irc_pcall(void* data);
int irc_paccept(void* data);
int irc_pclose(void* data);
int irc_ui_quit(void* data);
int irc_ui_nick(void* data);
int irc_ui_ban(void* data);
int irc_ui_exit(void* data);

#endif

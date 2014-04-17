#ifndef IRC_FUNS_USERINPUT_H
#define IRC_FUNS_USERINPUT_H

#include "commparser.h"
#include "types.h"

void irc_user_msgprocess(const char* msg, struct irc_clientdata* cdata);
int irc_msg(void* data);
int irc_me(void* data);
int irc_server_forward(void* data);
int irc_server(void* data);

#endif

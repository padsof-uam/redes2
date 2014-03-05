#ifndef IRC_CORE_H
#define IRC_CORE_H

#include "types.h"

struct irc_globdata* irc_init();
struct ircuser* irc_user_bynick(struct irc_globdata* gdata, const char* nick);


void irc_destroy(struct irc_globdata* data);

/**
Faltan:
*/
void irc_add_user_bynick(struct irc_globdata * ircdata,const char * new_nick);

void irc_add_user(struct irc_globdata * ircdata,const struct ircuser * user);

void irc_rm_user(struct irc_globdata * ircdata,const char * new_nick);

#endif

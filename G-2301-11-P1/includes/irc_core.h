#ifndef IRC_CORE_H
#define IRC_CORE_H

#include "types.h"

int irc_init(struct irc_globdata* gdata);
struct ircuser* irc_user_bynick(struct irc_globdata* gdata, const char* nick);
void irc_destroy(struct irc_globdata* data);
#endif

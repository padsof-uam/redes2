#ifndef IRC_CORE_H
#define IRC_CORE_H

#include "types.h"

struct irc_globdata* irc_init();
struct ircuser* irc_user_bynick(struct irc_globdata* gdata, const char* nick);
struct ircuser* irc_user_byid(struct irc_globdata* gdata, int id);
int irc_set_usernick(struct irc_globdata* data, int id, const char* nick);
int irc_create_user(struct irc_globdata* data, int id);
struct ircuser* irc_register_user(struct irc_globdata* data, int id);
void irc_destroy(struct irc_globdata* data);
int irc_channel_part(struct irc_globdata* data, struct ircchan* channel, struct ircuser* user);
void irc_delete_user(struct irc_globdata* data, struct ircuser* user);

/*	Falta:	 */

int irc_channel_adduser(struct irc_globdata* data, struct ircchan* channel, struct ircuser* user, char * key);
struct ircchan * irc_channel_byname(struct irc_globdata* data, char * name);

#endif

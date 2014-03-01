#include "irc_core.h"
#include "dictionary.h"
#include "list.h"

#include <string.h>

int irc_init(struct irc_globdata* gdata)
{
	gdata->chan_map = dic_new_withstr();
	gdata->fd_user_map = dic_new_withint();
	gdata->nick_user_map = dic_new_withstr();
	gdata->chan_list = list_new();

	if(!(gdata->chan_list && gdata->fd_user_map && gdata->nick_user_map && gdata->chan_list))
		return ERR_MEM; /* Fallo en la reserva */
	else
		return OK;
}

struct ircuser* irc_user_bynick(struct irc_globdata* gdata, const char* nick)
{
	return (struct ircuser*) dic_lookup(gdata->nick_user_map, nick);
}

static void _chan_destructor(void* ptr)
{
	struct ircchan* chan = (struct ircchan*) ptr;
	list_destroy(chan->users, NULL);
	free(chan);
}

void irc_destroy(struct irc_globdata* data)
{
	if(data->chan_list)
		list_destroy(data->chan_list, NULL); /* Liberamos la estructura de lista, no la memoria de cada canal */

	if(data->chan_map)
		dic_destroy(data->chan_map, _chan_destructor); /* Ahora liberamos además los datos de canales */

	/* Igual que antes */
	if(data->fd_user_map)
		dic_destroy(data->fd_user_map, NULL);

	if(data->nick_user_map)
		dic_destroy(data->nick_user_map, free);

	free(data);
}

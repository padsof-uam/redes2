#include "irc_core.h"
#include "dictionary.h"
#include "list.h"

#include <string.h>

struct irc_globdata* irc_init()
{
	struct irc_globdata* gdata = malloc(sizeof(struct irc_globdata));

	if(!gdata)
		return NULL;

	gdata->chan_map = dic_new_withstr();
	gdata->fd_user_map = dic_new_withint();
	gdata->nick_user_map = dic_new_withstr();
	gdata->chan_list = list_new();

	if(!(gdata->chan_list && gdata->fd_user_map && gdata->nick_user_map && gdata->chan_list))
		irc_destroy(gdata);

	return gdata;
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
	if(!data)
		return;

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

int irc_set_usernick(struct irc_globdata* data, int id, const char* nick)
{
	struct ircuser* user;
	struct ircuser* user_samenick;

	if(!nick)
		return ERR_RANGE;

	user = dic_lookup(data->fd_user_map, &id);

	if(!user)
		return ERR_NOTFOUND;

	user_samenick = dic_lookup(data->nick_user_map, nick);

	if(user_samenick != NULL) /* Hay alguien con el mismo nick*/
		return ERR_REPEAT;

	dic_remove(data->nick_user_map, user->nick); /* Eliminamos el nick viejo del dic. */
	strncpy(user->nick, nick, MAX_NICK_LEN); /* Copiamos el nuevo a la estructura */
	dic_add(data->nick_user_map, nick, user); /* Y actualizamos el diccionario */

	return OK;
}

struct ircuser* irc_register_user(struct irc_globdata* data, int id)
{
	struct ircuser* user;

	if(dic_lookup(data->fd_user_map, &id) != NULL)
		return NULL; /* ID ya en la base de datos */

	user = malloc(sizeof(struct ircuser));
	bzero(user, sizeof(struct ircuser)); /* Ponemos todos los datos a 0 */
	
	user->fd = id;
	user->channels = list_new();

	dic_add(data->fd_user_map, &id, user);

	return user;
}

void irc_delete_user(struct irc_globdata* data, struct ircuser* user)
{
	int chan_count;
	int i;

	dic_remove(data->fd_user_map, &(user->fd));
	dic_remove(data->nick_user_map, user->name);

	chan_count = list_count(user->channels);

	for (i = 0; i < chan_count; ++i)
		irc_channel_part(data, list_at(user->channels, i), user);

	list_destroy(user->channels, NULL);

	free(user);
}


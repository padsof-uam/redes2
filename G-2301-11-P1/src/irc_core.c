#include "irc_core.h"
#include "dictionary.h"
#include "list.h"
#include "irc_codes.h"

#include <string.h>

struct irc_globdata *irc_init()
{
    struct irc_globdata *gdata = malloc(sizeof(struct irc_globdata));

    if (!gdata)
        return NULL;

    gdata->chan_map = dic_new_withstr();
    gdata->fd_user_map = dic_new_withint();
    gdata->nick_user_map = dic_new_withstr();
    gdata->chan_list = list_new();

    strncpy(gdata->servername, "redes-ircd", MAX_SERVER_NAME); /* Nombre por defecto */

    if (!(gdata->chan_list && gdata->fd_user_map && gdata->nick_user_map && gdata->chan_list))
        irc_destroy(gdata);

    return gdata;
}

struct ircuser *irc_user_bynick(struct irc_globdata *gdata, const char *nick)
{
    return (struct ircuser *) dic_lookup(gdata->nick_user_map, nick);
}

struct ircuser *irc_user_byid(struct irc_globdata *gdata, const int id)
{
    return (struct ircuser *) dic_lookup(gdata->fd_user_map, &id);
}

static void _chan_destructor(void *ptr)
{
    struct ircchan *chan = (struct ircchan *) ptr;
    list_destroy(chan->users, NULL);
    dic_destroy(chan->invited_users, NULL);
    free(chan);
}

static void _user_destructor(void *ptr)
{
	struct ircuser* user = (struct ircuser*) ptr;

	if(!user)
		return;

	list_destroy(user->channels, NULL);
	free(user);
}

void irc_destroy(struct irc_globdata *data)
{
    if (!data)
        return;

    if (data->chan_list)
        list_destroy(data->chan_list, NULL); /* Liberamos la estructura de lista, no la memoria de cada canal */

    if (data->chan_map)
        dic_destroy(data->chan_map, _chan_destructor); /* Ahora liberamos además los datos de canales */

    /* Igual que antes */
    if (data->nick_user_map)
        dic_destroy(data->nick_user_map, NULL);

    if (data->fd_user_map)
        dic_destroy(data->fd_user_map, _user_destructor);

    free(data);
}

int irc_set_usernick(struct irc_globdata *data, int id, const char *nick)
{
    struct ircuser *user;
    struct ircuser *user_samenick;

    if (!nick)
        return ERR_RANGE;

    user = dic_lookup(data->fd_user_map, &id);

    if (!user)
        return ERR_NOTFOUND;

    user_samenick = dic_lookup(data->nick_user_map, nick);

    if (user_samenick != NULL) /* Hay alguien con el mismo nick*/
        return ERR_NICKCOLLISION;

    dic_remove(data->nick_user_map, user->nick); /* Eliminamos el nick viejo del dic. */
    strncpy(user->nick, nick, MAX_NICK_LEN); /* Copiamos el nuevo a la estructura */
    dic_add(data->nick_user_map, nick, user); /* Y actualizamos el diccionario */

    return OK;
}

struct ircuser *irc_register_user(struct irc_globdata *data, int id)
{
    struct ircuser *user;

    if (dic_lookup(data->fd_user_map, &id) != NULL)
        return NULL; /* ID ya en la base de datos */

    user = malloc(sizeof(struct ircuser));

    bzero(user, sizeof(struct ircuser)); /* Ponemos todos los datos a cero */

    user->fd = id;
    user->channels = list_new();

    dic_add(data->fd_user_map, &id, user);

    return user;
}

void irc_delete_user(struct irc_globdata *data, struct ircuser *user)
{
    int chan_count;
    int i;

    dic_remove(data->fd_user_map, &(user->fd));
    dic_remove(data->nick_user_map, user->name);

    chan_count = list_count(user->channels);

    for (i = 0; i < chan_count; ++i)
        irc_channel_part(data, list_at(user->channels, i), user);

    _user_destructor(user);
}

int irc_compare_user(const void *user1, const void *user2)
{
    struct ircuser *first = (struct ircuser *) user1;
    struct ircuser *second = (struct ircuser *) user2;
    return strncmp(first->nick, second->nick, MAX_NICK_LEN);
}

short irc_user_inchannel(struct ircchan *channel, struct ircuser *user)
{
    if (list_find(channel->users, irc_compare_user, (void *)user ) == -1)
        return ERR_NOTFOUND;
    else
        return OK;
}


struct ircchan *irc_channel_byname(struct irc_globdata *data, const char *name)
{
    return dic_lookup(data->chan_map, name);
}


int irc_channel_adduser(struct irc_globdata *data, struct ircchan *channel, struct ircuser *user, const char *password)
{
    if (channel->has_password && (password == NULL || strcmp(password, channel->password) != 0))
        return ERR_BADCHANNELKEY;

    if (irc_user_inchannel(channel, user) != ERR_NOTFOUND)
        return ERR_ALREADYREGISTRED;

    if (channel->mode & chan_invite)
    {
        if (!dic_lookup(channel->invited_users, user->nick))
            return ERR_INVITEONLYCHAN;
    }

    if (list_count(channel->users) >= MAX_MEMBERS_IN_CHANNEL)
        return ERR_CHANNELISFULL;

    if (list_count(user->channels) >= MAX_CHANNELES_USER)
        return ERR_TOOMANYCHANNELS;

    /*
    Comrprobados:
        - Contraseña
        - Invitados
        - Already in channel
        - Demasiada gente
        - too many chanels;
        */

    if (list_add(channel->users, user) != OK)
        return ERR;
    else if (list_add(user->channels, channel) != OK)
    {
        list_remove_last(channel->users);
        return ERR;
    }
    return OK;
}

int irc_channel_part(struct irc_globdata *data, struct ircchan *channel, struct ircuser *user)
{
    list_remove_element(channel->users, ptr_comparator, user);
    list_remove_element(user->channels, ptr_comparator, channel);

    return OK;
}

struct ircchan *irc_register_channel(struct irc_globdata *data, const char *name)
{
    struct ircchan *chan = malloc(sizeof(struct ircchan));

    if (!chan)
        return NULL;

    strncpy(chan->name, name, MAX_CHAN_LEN);
    list_add(data->chan_list, chan);
    dic_add(data->chan_map, name, chan);
    
    chan->users = list_new();
    chan->mode = 0;
    chan->has_password = 0;
    chan->invited_users = dic_new_withstr();

    return chan;
}

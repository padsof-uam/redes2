#include "irc_core.h"
#include "dictionary.h"
#include "list.h"
#include "irc_codes.h"
#include "jsmn.h"
#include "log.h"

#include <string.h>
#include <stdio.h>

#define MAX_CONF_LEN 1000

struct irc_globdata *irc_init()
{
    struct irc_globdata *gdata = malloc(sizeof(struct irc_globdata));

    if (!gdata)
        return NULL;

    gdata->chan_map = dic_new_withstr();
    gdata->fd_user_map = dic_new_withint();
    gdata->nick_user_map = dic_new_withstr();
    gdata->chan_list = list_new();
    gdata->oper_passwords = dic_new_withstr();

    strncpy(gdata->servername, "redes-ircd", MAX_SERVER_NAME); /* Nombre por defecto */

    if (!(gdata->chan_list && gdata->fd_user_map && gdata->nick_user_map && gdata->chan_list && gdata->oper_passwords))
    {
        irc_destroy(gdata);
        return NULL;
    }

    return gdata;
}

static void _parse_opers(struct irc_globdata *irc, char *contents, jsmntok_t *tok)
{
    int i;
    int childs = tok[0].size;
    char *user, *pass;

    for (i = 1; i < childs; i += 2)
    {
        *(contents + tok[i].end) = '\0';
        *(contents + tok[i + 1].end) = '\0';
        user = contents + tok[i].start;
        pass = contents + tok[i + 1].start;

        dic_add(irc->oper_passwords, user, strdup(pass));
        slog(LOG_DEBUG, "Configurada contraseña para %s", user);
    }
}

int irc_load_config(struct irc_globdata *irc, const char *file)
{
    long conf_len;
    char *conf_contents;
    FILE *f_conf;
    jsmn_parser parser;
    jsmntok_t tokens[100];
    int i, parsed;

    f_conf = fopen(file, "r");

    if (!f_conf)
        return ERR_NOTFOUND;

    fseek( f_conf , 0L , SEEK_END);
    conf_len = ftell( f_conf );
    rewind( f_conf );

    conf_contents = calloc( 1, conf_len + 1 );

    if (!conf_contents)
        return ERR_MEM;

    if (fread(conf_contents, conf_len, 1, f_conf) != 1)
    {
        free(conf_contents);
        return ERR_IO;
    }

    fclose(f_conf);

    jsmn_init(&parser);

    parsed = jsmn_parse(&parser, conf_contents, conf_len, tokens, 100);

    if (parsed < 0)
        return ERR_PARSE;

    for (i = 0; i < parsed; i++)
    {
        if (tokens[i].type == JSMN_PRIMITIVE && !strncmp(conf_contents + tokens[i].start, "opers", strlen("opers")))
            _parse_opers(irc, conf_contents, &(tokens[i + 1]));
    }

    free(conf_contents);
    return OK;
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

    if (!chan)
        return;

    list_destroy(chan->users, NULL);
    dic_destroy(chan->invited_users, NULL);
    list_destroy(chan->operators, NULL);
    list_destroy(chan->banned_users, free);

    free(chan);
}

static void _user_destructor(void *ptr)
{
    struct ircuser *user = (struct ircuser *) ptr;

    if (!user)
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

    if (data->oper_passwords)
        dic_destroy(data->oper_passwords, NULL);
    

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
    struct ircchan *chan;
    list* chans_todelete;

    chans_todelete = list_new();
    dic_remove(data->fd_user_map, &(user->fd));
    dic_remove(data->nick_user_map, user->nick);

    chan_count = list_count(user->channels);

    for (i = 0; i < chan_count; ++i)
        list_add(chans_todelete, list_at(user->channels, i));

    /* No se modifican las listas sobre las que se iteran. */
    for (i = 0; i < chan_count; ++i)
    {
        chan = list_at(chans_todelete, i);
        irc_channel_part(data, chan, user);
        irc_channel_removeop(chan, user);
    }

    list_destroy(chans_todelete, NULL);
    _user_destructor(user);
}


short irc_user_inchannel(struct ircchan *channel, struct ircuser *user)
{
    if (list_find(channel->users, ptr_comparator, (void *)user ) == -1)
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

    if(irc_is_banned(channel, user))
        return ERR_BANNEDFROMCHAN;

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

    bzero(chan, sizeof(struct ircchan));

    strncpy(chan->name, name, MAX_CHAN_LEN);
    list_add(data->chan_list, chan);
    dic_add(data->chan_map, name, chan);

    chan->users = list_new();
    chan->mode = 0;
    chan->has_password = 0;
    chan->invited_users = dic_new_withstr();
    chan->operators = list_new();
    chan->user_limit = -1;
    chan->banned_users = list_new();

    return chan;
}

int irc_channel_addop(struct ircchan *channel, struct ircuser *user)
{
    if (irc_user_inchannel(channel, user) == OK)
        return list_add(channel->operators, user);
    else
        return ERR;
}
int irc_is_channel_op(struct ircchan *chan, struct ircuser *user)
{
    return (user->mode & user_op) || (list_find(chan->operators, ptr_comparator, user) != -1);
}


int irc_set_channel_pass(struct ircchan *chan, const char *pass)
{
    strncpy(chan->password, pass, MAX_KEY_LEN);
    chan->has_password = 1;

    return OK;
}

int irc_channel_removeop(struct ircchan *chan, struct ircuser *user)
{
    return list_remove_element(chan->operators, ptr_comparator, user);
}

int irc_add_ban(struct ircchan* chan, const char* banmask)
{
    int idx;
    char* banmask_cpy = strdup(banmask);
    int retval;

    idx = list_find(chan->banned_users, (comparator) strcmp, banmask_cpy);

    if(idx != -1)
    {
        free(banmask_cpy);
        return ERR_REPEAT;
    }

    retval =  list_add(chan->banned_users, banmask_cpy);

    if(retval != OK)
        free(banmask_cpy);

    return retval;
}

int irc_lift_ban(struct ircchan* chan, const char* banmask)
{
    int idx;
    char* ban;

    idx = list_find(chan->banned_users, (comparator) strcmp, banmask);

    if(idx == -1)
        return ERR_NOTFOUND;

    ban = list_at(chan->banned_users, idx);
    
    if(ban)
        free(ban);

    return list_remove(chan->banned_users, idx);
}

int irc_is_banned(struct ircchan* chan, struct ircuser* user)
{
    int i;
    char* banmask;

    for(i = 0; i < list_count(chan->banned_users); i++)
    { 
        banmask = list_at(chan->banned_users, i);  

        if(irc_name_matches(banmask, user->nick))
            return 1;
    }

    return 0;
}

int irc_name_matches(const char* banmask, const char* name)
{
    short in_wildcard;

    if(!banmask || !name)
        return 0;

    while((*banmask != '\0' || in_wildcard) && *name != '\0')
    {
        if(*banmask == '*')
        {
            in_wildcard = 1;
            banmask++;
        }
        else if(*banmask == *name)
        {
            in_wildcard = 0;
            banmask++;
            name++;
        }
        else if(in_wildcard)
        {
            name++;
        }
        else
        {
            break;
        }
    }

    return *banmask == *name;
}

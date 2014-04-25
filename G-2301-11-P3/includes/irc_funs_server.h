#ifndef IRC_FUNS_SERVER_H
#define IRC_FUNS_SERVER_H

#include "types.h"

/**
 * Función de procesado de mensajes en el lado del servidor.
 * @param snd_qid Cola de mensajes donde encolar las respuestas.
 * @param data    Datos del mensaje para procesar.
 * @param gdata   Estructura de los datos de irc.
 */
void irc_server_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_globdata *gdata);

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


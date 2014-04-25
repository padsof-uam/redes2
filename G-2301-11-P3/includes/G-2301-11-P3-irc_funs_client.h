#ifndef IRC_CLIENT_FUNS_H
#define IRC_CLIENT_FUNS_H

#include "G-2301-11-P3-types.h"

/**
 * Función de procesado de mensajes en el lado del cliente.
 * @param snd_qid Cola de mensajes donde encolar las respuestas.
 * @param data    Datos del mensaje para procesar.
 * @param cdata   Estructura de los datos del cliente irc.
 */
void irc_client_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_clientdata *cdata);

int irc_ignore(void* data);
int irc_default(void * data);
int irc_recv_topic(void* data);
int irc_recv_join(void* data);
int irc_recv_part(void* data);
int irc_recv_privmsg(void* data);
int irc_needmoreparams(void* data);
int irc_nickcollision(void* data);
int irc_recv_nick(void* data);
int irc_recv_quit(void* data);
int irc_recv_notice(void* data);
int irc_recv_mode(void* data);
int irc_recv_ping(void* data);
int irc_recv_who(void* data);
int irc_recv_end_motd(void* data);
int irc_recv_nosuchnick(void* data);

void parse_pcall(struct irc_clientdata* cdata, char* text, char* source);
void parse_paccept(struct irc_clientdata* cdata, char* text, char* source);
void parse_pclose(struct irc_clientdata* cdata, char* text, char* source);

void parse_fsend(struct irc_clientdata * con_data, char* text, char* source);
void parse_faccept(struct irc_clientdata * con_data, char* text, char* source);
void parse_fcancel(struct irc_clientdata * con_data, char* text, char* source);

#endif

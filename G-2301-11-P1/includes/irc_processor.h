#ifndef IRC_PROCESSOR_H
#define IRC_PROCESSOR_H

#include "types.h"
#include "commparser.h"

void irc_msgprocess(int snd_qid, struct sockcomm_data* data, struct irc_globdata* gdata);

/**
 * Separa los mensajes por CRLF, modificando y sustituyendo por \0.
 * @param  str Cadena a buscar. Al salir de la función, esta cadena acaba en el 
 *             primer CRLF.
 * @param  len Longitud máxima de la cadena.
 * @return     Puntero al primer carácter después del CRLF
 */
char* irc_msgsep(char* str, int len);
char* irc_remove_prefix(char* msg);
void irc_enqueue_msg(struct sockcomm_data* msg, int snd_qid);
struct sockcomm_data* irc_build_errmsg(int errcode);
int irc_create_welcome_messages(struct ircuser* user, list* msgqueue);
#endif


#ifndef IRC_PROCESSOR_H
#define IRC_PROCESSOR_H

#include "types.h"
#include "commparser.h"

int irc_privmsg(void* data);

void irc_msgprocess(int snd_qid, struct sockcomm_data* data, struct irc_globdata* gdata);

/**
 * Separa los mensajes por CRLF, modificando y sustituyendo por \0.
 * @param  str Cadena a buscar. Al salir de la función, esta cadena acaba en el 
 *             primer CRLF.
 * @param  len Longitud máxima de la cadena.
 * @return     Puntero al primer carácter después del CRLF
 */
char* irc_msgsep(char* str, int len);

#endif

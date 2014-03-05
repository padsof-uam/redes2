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

/**
 * Analiza una cadena y extrae los argumentos al mensaje IRC, ignorando tanto 
 * 	el prefijo como el nombre de mensaje. Si se la llama con mensaje
 * 		:pref PRIVMSG a,b,c,d Hola : Nuevo mensaje
 *	guardará en el array params los valores
 *		{"a,b,c,d", "Hola", "Nuevo mensaje" }
 * @param  msg        Mensaje.
 * @param  params     Array de cadenas donde se guardarán los parámetros.
 * @param  max_params Número máximo de parámetros a leer.
 * @return            Número de parámetros leídos.
 */
int irc_parse_paramlist(char* msg, char** params, size_t max_params);
void irc_enqueue_msg(struct sockcomm_data* msg, int snd_qid);
struct sockcomm_data* irc_build_errmsg(int errcode);
int irc_create_welcome_messages(struct ircuser* user, list* msgqueue);
int irc_create_quit_messages(struct ircuser* user, list* msgqueue, const char* message);
#endif


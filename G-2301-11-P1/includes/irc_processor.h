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
int irc_parse_paramlist(char* msg, char** params, size_t max_params);
void irc_enqueue_msg(struct sockcomm_data* msg, int snd_qid);

/**
* Construyte un mensaje para ser enviado (pero no lo envía).
* @param 	errcode		Código de error. (Consultar "irc_codes.h" para más información sobre los errores)
* @param 	fd			Socket asociado al destinatario.
* @param 	msg 		Este argumento no es necesario. Sólo si queremos especificar algo en el mensaje de error. 
*						Si este argumento no es NULL, se enviará este mensaje descartando el código de error correspondiente.
* @return	La estructura rellena, lista para ser encolada o para ser añadida a la lista de mensaje por enviar.
*/
struct sockcomm_data* irc_build_errmsg(int errcode, int fd,char * msg);

/**
* Recibe un código de error y devuelve una frase interpretando el error.
* @param 	errcode		Código de error.
* @return 	Interpretación del error.
*/
char * _irc_errmsg (int errcode);

int irc_create_welcome_messages(struct ircuser* user, list* msgqueue);
int irc_create_quit_messages(struct ircuser* user, list* msgqueue, const char* message);
#endif


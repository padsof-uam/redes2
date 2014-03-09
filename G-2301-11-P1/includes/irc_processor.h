#ifndef IRC_PROCESSOR_H
#define IRC_PROCESSOR_H

#include "types.h"
#include "commparser.h"

/**
 * Procesa un mensaje IRC.
 * @param snd_qid ID de la cola donde enviar los mensajes generados.
 * @param data    Estructura con los datos del mensaje.
 * @param gdata   Estructura con los datos de IRC.
 */
void irc_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_globdata *gdata);

/**
 * Separa los mensajes por CRLF, modificando y sustituyendo por \0.
 * @param  str Cadena a buscar. Al salir de la función, esta cadena acaba en el
 *             primer CRLF.
 * @param  len Longitud máxima de la cadena.
 * @return     Puntero al primer carácter después del CRLF
 */
char *irc_msgsep(char *str, int len);

/**
 * Ignora el prefijo de un mensaje IRC.
 * Una llamada con ":pref PRIVMSG" como argumento devolverá un puntero
 *  a la "P" inicial de PRIVMSG.
 *
 * @param  msg Puntero al mensaje.
 * @return     Puntero al inicio del mensaje con el prefijo ignorado.
 */
char *irc_remove_prefix(char *msg);

/**
 * Analiza una cadena y extrae los argumentos al mensaje IRC, ignorando tanto
 *  el prefijo como el nombre de mensaje. Si se la llama con mensaje
 *      :pref PRIVMSG a,b,c,d Hola : Nuevo mensaje
 *  guardará en el array params los valores
 *      {"a,b,c,d", "Hola", "Nuevo mensaje" }
 * @param  msg        Mensaje.
 * @param  params     Array de cadenas donde se guardarán los parámetros.
 * @param  max_params Número máximo de parámetros a leer.
 * @return            Número de parámetros leídos.
 */
int irc_parse_paramlist(char *msg, char **params, size_t max_params);

/**
 * Encola un mensaje para ser enviado.
 * @param msg     Mensaje.
 * @param snd_qid ID de cola.
 * @return Código de error.
 */
int irc_enqueue_msg(struct sockcomm_data *msg, int snd_qid);

/**
 * Construye un mensaje de respuesta.
 * @param  irc               Estructura con los datos de un mensaje IRC.
 * @param  errcode           Código de respuesta.
 * @see irc_codes.h
 * @param  additional_params (Opcional) Parámetros adicionales del mensaje de error.
 * @return                   Estructura rellena con el mensaje.
 */
struct sockcomm_data *irc_build_errmsg(struct irc_msgdata *irc, int errcode, const char *additional_params);

/**
 * Construye un mensaje de respuesta.
 * @param  irc               Estructura con los datos de un mensaje IRC.
 * @param  errcode           Código de respuesta.
 * @see irc_codes.h
 * @param  additional_params (Opcional) Parámetros adicionales del mensaje de error.
 * @param  msg_text			 Texto adicional de la respuesta.
 * @return                   Estructura rellena con el mensaje.
 */
struct sockcomm_data *irc_build_errmsg_withtext(struct irc_msgdata *irc, int errcode, const char *additional_params, const char* text);

/**
 * Crea el mensaje de respuesta numérica para el código de error dato.
 * @param irc     Estructura de datos IRC:
 * @param errcode Código de error.
 * @param additional_params (Opcional) Parámetros adicionales del mensaje de error.
 * @param[out] message_str  Cadena donde escribir el código.
 * @return Número de caracteres escritos.
 */
/*int irc_build_numeric_response(struct irc_msgdata *irc, int errcode, const char* additional_params, char *message_str);
*/
/**
* Recibe un código de error y devuelve una frase interpretando el error.
* @param    errcode     Código de error.
* @return   Interpretación del error.
*/
char *irc_errstr (int errcode);

/**
 * Crea los mensajes de bienvenida especificados en el RFC para un usuario.
 * @notes Esta función no modifica la estructura de usuario.
 * @param  user     Usuario.
 * @param  msgqueue Lista donde se introducirán los mensajes.
 * @return          Código de retorno.
 */
int irc_create_welcome_messages(struct ircuser *user, list *msgqueue);

/**
 * Crea los mensajes de despedida especificados en el RFC para un usuario.
 * @param  user     Usuario.
 * @param  msgqueue Lista donde se introducirán los mensajes.
 * @param  message  Mensaje de despedida.
 * @return          Código de error.
 */
int irc_create_quit_messages(struct ircuser *user, list *msgqueue, const char *message);

/**
 * Crea un mensaje para un cliente con el texto que se dé como formato
 * @see printf
 * @param  fd         Socket de destino.
 * @param  fmt_string Cadena (estilo printf) que escribir en el mensaje.
 * @param  ...        Lista de parámetros para printf.
 * @return            Estructura sockcomm_data lista para enviar.
 */
struct sockcomm_data* irc_response_create(int fd, const char* fmt_string, ...);
#endif


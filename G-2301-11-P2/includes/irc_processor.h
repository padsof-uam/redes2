#ifndef IRC_PROCESSOR_H
#define IRC_PROCESSOR_H

#include "types.h"
#include "commparser.h"

#include <stdarg.h>

struct ircflag
{
    char code;
    int value;
};

#define IRCFLAGS_END { -1, -1 }
#define IS_IRCFLAGS_END(flag) ((flag).code == -1 && (flag).value == -1)

/**
 * Función de procesado de mensajes en el lado del servidor.
 * @param snd_qid Cola de mensajes donde encolar las respuestas.
 * @param data    Datos del mensaje para procesar.
 * @param gdata   Estructura de los datos de irc.
 */
void irc_server_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_globdata *gdata);

/**
 * Función de procesado de mensajes en el lado del cliente.
 * @param snd_qid Cola de mensajes donde encolar las respuestas.
 * @param data    Datos del mensaje para procesar.
 * @param gdata   Estructura de los datos de irc.
 */
void irc_client_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_globdata *gdata);

/**
 * @internal
 * @param snd_qid Cola de mensajes que serán enviados.
 * @param data    Datos del mensaje para procesar.
 * @param gdata   La estructura global de irc
 * @param cmds    Array de comandos soportados.
 * @param actions Array de cmd_actions soportadas
 * @param len     Longitud de los arrays.
 */
void _irc_msgprocess(int snd_qid, struct sockcomm_data *data, struct irc_globdata *gdata,const char ** cmds, cmd_action * actions, int len);

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
struct sockcomm_data *irc_build_numericreply(struct irc_msgdata *irc, int errcode, const char *additional_params);

/**
 * Construye un mensaje de respuesta.
 * @param  irc               Estructura con los datos de un mensaje IRC.
 * @param  errcode           Código de respuesta.
 * @see irc_codes.h
 * @param  additional_params (Opcional) Parámetros adicionales del mensaje de error.
 * @param  msg_text          Texto adicional de la respuesta.
 * @return                   Estructura rellena con el mensaje.
 */
struct sockcomm_data *irc_build_numericreply_withtext(struct irc_msgdata *irc, int errcode, const char *additional_params, const char *text);

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
 * Crea los mensajes de salida de un usuario especificados en el RFC para un usuario.
 * @param  user     Usuario.
 * @param  msgqueue Lista donde se introducirán los mensajes.
 * @param  message  Comentario de kill.
 * @return          Código de error.
 */
int irc_create_kill_messages(struct ircuser *user, list *msgqueue, const char *tokill_name, const char *message);

/**
 * Crea un mensaje para un cliente con el texto que se dé como formato
 * @see printf
 * @param  fd         Socket de destino.
 * @param  fmt_string Cadena (estilo printf) que escribir en el mensaje.
 * @param  ...        Lista de parámetros para printf.
 * @return            Estructura sockcomm_data lista para enviar.
 */
struct sockcomm_data *irc_response_create(int fd, const char *fmt_string, ...);

/**
 * Crea un mensaje para un cliente con el texto que se dé como formato, con la lista
 *  de argumentos en formato va_list.
 * @see vfprintf
 * @see stdarg
 * @param  fd         Socket de destino.
 * @param  fmt_string Cadena (estilo printf) que escribir en el mensaje.
 * @param  ap         Lista de parámetros para printf.
 * @return            Estructura sockcomm_data lista para enviar.
 */
struct sockcomm_data *irc_response_vcreate(int fd, const char *fmt_string, va_list ap);


/**
 * Construye un mensaje de respuesta y lo inserta en la lista de mensajes a enviar.
 * @param  irc               Estructura con los datos de un mensaje IRC.
 * @param  errcode           Código de respuesta.
 * @see irc_codes.h
 * @param  additional_params (Opcional) Parámetros adicionales del mensaje de error.
 * @return                   Código OK/ERR
 * @see irc_build_numeric_reply
 */
int irc_send_numericreply(struct irc_msgdata *irc, int errcode, const char *additional_params);

/**
 * Construye un mensaje de respuesta con texto adicional y lo inserta en la lista
 *  de mensajes a enviar.
 * @param  irc               Estructura con los datos de un mensaje IRC.
 * @param  errcode           Código de respuesta.
 * @see irc_codes.h
 * @param  additional_params (Opcional) Parámetros adicionales del mensaje de error.
 * @param  msg_text          Texto adicional de la respuesta.
 * @return                   Código OK/ERR.
 * @see irc_build_numericreply_withtext
 */
int irc_send_numericreply_withtext(struct irc_msgdata *irc, int errcode, const char *additional_params, const char *text);

/**
 * Envía un mensaje a todos los usuarios de un canal.
 * @param  channel    Canal.
 * @param  msg_tosend Lista de mensajes para enviar.
 * @param  sender     Usuario que envía el mensaje, importante para no reenviarle a él las emisiones.
 *                      Se ignorará si es NULL.
 * @param  message    Texto del mensaje.
 * @param  ...        Parámetros para formatear el mensaje.
 * @see printf
 * @see stdarg.h
 * @see irc_response_create
 * @return            Código OK/ERR.
 */
int irc_channel_broadcast(struct ircchan *channel, list *msg_tosend, struct ircuser *sender, const char *message, ...);

/**
 * Analiza una cadena del estilo "[+|-]abcd" para extraer las distintas flags.
 * @param  flags   Cadena con los flags.
 * @param  flagval Valor a modificar con los flags.
 * @param  flagdic Lista de estructuras ircflag terminada en IRCFLAG_END
 * @return         OK/ERR.
 */
int irc_flagparse(const char *flags, int *flagval, const struct ircflag *flagdic);

/**
 * Analiza los valores de una variable almacenando banderas y guarda sus valores
 * @param  flags   Banderas.
 * @param  str     Buffer donde guardar la cadena
 * @param  len     Longitud del buffer
 * @param  flagdic Diccionario valor de flag-carácter.
 * @return         OK/ERR
 */
int irc_strflag(int flags, char *str, size_t len, const struct ircflag *flagdic);


/**
 * Crea una respuesta tipo NAMES para un canal dado.
 * @param  channel Canal
 * @param  irc     Datos del mensaje
 * @return         OK/ERR.
 */
int irc_send_names_messages(struct ircchan *channel, struct irc_msgdata *irc);

/**
 * Envía una respuesta con la cadena formateada.
 * @param  irc        Datos de mensaje.
 * @param  msg_format Cadena de formato.
 * @param  ...        Variables a formatear.
 * @return            OK/ERR
 */
int irc_send_response(struct irc_msgdata *irc, const char *msg_format, ...);

/**
 * Devuelve si dos usuarios tienen algún canal en común.
 * @param  a Usuario a
 * @param  b Usuario b
 * @return   0 o valor distinto de 0.
 */
int irc_users_have_common_chans(struct ircuser* a, struct ircuser* b);

#endif


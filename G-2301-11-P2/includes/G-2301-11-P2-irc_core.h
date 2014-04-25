#ifndef IRC_CORE_H
#define IRC_CORE_H

#include "G-2301-11-P2-types.h"

/**
 * Inicializa una estructura de irc_globdata.
 * @return El puntero a la estructura.
 */
struct irc_globdata* irc_init();
/**
 * Busca un usuario por nick en la estructura general
 * @param  gdata Estructura globdata en la que está el diccionario de usuarios.
 * @param  nick  Nick del usuario que queremos buscar.
 * @see    irc_globdata.
 * @return       Puntero al usuario o NULL en caso de no encontrarse.
 */
struct ircuser* irc_user_bynick(struct irc_globdata* gdata, const char* nick);

/**
 * Busca un usuario por nick en la estructura general
 * @param  gdata Estructura globdata en la que está el diccionario de usuarios.
 * @param  id  Id del usuario que queremos buscar.
 * @see    irc_globdata.
 * @return       Puntero al usuario o NULL en caso de no encontrarse.
 */
struct ircuser* irc_user_byid(struct irc_globdata* gdata, int id);

/**
 * Cambia el nick de un usuario.
 * @param  data Estructura globdata en la que está el diccionario de usuarios.
 * @param  id   Id del usuario al que queremos cambiar el nick.
 * @param  nick Nuevo nick para el usuario.
 * @return      OK/ERR
 */
int irc_set_usernick(struct irc_globdata* data, int id, const char* nick);

/**
 * Crea un usuario nuevo.
 * @param  data Estructura globdata en la que está el diccionario de usuarios.
 * @param  id   Id (fd asociado) con el que creamos el nuevo usuario.
 * @return      El usuario creado o NULL si ya se encuentra. 
 */
struct ircuser* irc_register_user(struct irc_globdata* data, int id);

/**
 * Libera la memoria de la estructura.
 * @param data La estructura de tipo irc_globdata a liberar.
 */
void irc_destroy(struct irc_globdata* data);

/**
 * Añade el usuario como operador del canal.
 * @param  channel Canal en el que añadir el usuario.
 * @param  user    Usuario que se convertirá en operador.
 * @return         OK/ERR.
 */
int irc_channel_addop(struct ircchan * channel, struct ircuser * user);

/**
 * Elimina a un usuario del canal.
 * @param  data 		La estructura de contiene toda la información.
 * @param  channel 		El canal de que eliminar al usuario.
 * @param  user 		El usuario a eliminar del canal.
 * @return	OK/ERR
 */	
int irc_channel_part(struct irc_globdata* data, struct ircchan* channel, struct ircuser* user);

/**
 * Elimina a un usuario del sistema.
 * @param data Datos de IRC.
 * @param user Usuario a eliminar. Se libera la memoria del usuario.
 */
void irc_delete_user(struct irc_globdata* data, struct ircuser* user);


/**
 * Crea un canal y lo añade al sistema.
 * @param  data Datos IRC.
 * @param  name Nombre del canal.
 * @return      Estructura de canal.
 */
struct ircchan* irc_register_channel(struct irc_globdata* data, const char* name);

/**
 * Añade un usuario a un canal.
 * @param  data     Datos de IRC.
 * @param  chan     Canal.
 * @param  user     Usuario.
 * @param  password Contraseña (opcional).
 * @return          Código de error.
 */
int irc_channel_adduser(struct irc_globdata* data, struct ircchan* chan, struct ircuser* user, const char * password);

/**
* Busca al usuario en el canal.
* @param	channel 	Canal en el que buscar al usuario.
* @param	user 		Usuario a ser buscado.
* @return 	OK si se ha encontrado, ERR_NOTFOUND si no.
*/
short irc_user_inchannel(struct ircchan * channel, struct ircuser * user);

/**
* Busca el canal por nombre.
* @param 	data	La estructura que contiene los diccionarios.
* @param	name	Nombre del canal que buscar.
* @return	El ircchan correspondiente, o NULL en caso de no existir.
*/
struct ircchan * irc_channel_byname(struct irc_globdata* data, const char * name);

/**
 * Devuelve si el usuario dado está en la lista de operadores del canal.
 * @param  chan Canal.
 * @param  user Usuario.
 * @return      1 ó 0 según el usuario sea o no operador.
 */
int irc_is_channel_op(struct ircchan* chan, struct ircuser* user);

/**
 * Carga la configuración del servidor de un archivo.
 * @param  irc  Estructura IRC.
 * @param  file Archivo de configuración.
 * @return      OK/ERR.
 */
int irc_load_config(struct irc_globdata* irc, const char *file);

/**
 * Establece una contraseña para un canal.
 * @param  chan Canal.
 * @param  pass Contraseña.
 * @return      OK.
 */
int irc_set_channel_pass(struct ircchan* chan, const char* pass);

/**
 * Elimina privilegios de operador en un canal a un usuario, si los tiene.
 * @param  chan Canal.
 * @param  user Usuario
 * @return      OK/ERR.
 */
int irc_channel_removeop(struct ircchan* chan, struct ircuser* user);

/**
 * Devuelve 1 o 0 según un usuario esté baneado en un canal o no.
 * @param  chan  Canal
 * @param  user  Usuario
 * @return       1 si está baneado, 0 si no.
 */
int irc_is_banned(struct ircchan* chan, struct ircuser* user);

/**
 * Elimina el baneo de un usuario.
 * @param  chan    Canal.
 * @param  banmask Máscara de baneo.
 * @return         OK o ERR_NOTFOUND si no se ha encontrado.
 */
int irc_lift_ban(struct ircchan* chan, const char* banmask);

/**
 * Añade un usuario baneado a un canal.	
 * @param  chan    Canal
 * @param  banmask Máscara de baneo.
 * @return         OK o ERR_REPEAT si la máscara ya existía.
 */
int irc_add_ban(struct ircchan* chan, const char* banmask);

/**
 * Comprueba si el nombre dado coincide con la máscara de baneo.
 * @param  banmask Máscara.
 * @param  name    Nombre.
 * @return         1 ó 0 ssegún coincida o no.
 */
int irc_name_matches(const char* banmask, const char* name);

/**
 * Comprueba si un usuario ha sido dado la voz en un canal.
 * @param  chan Canal
 * @param  user Usuario
 * @return      0 si no tiene voz, 1 si la tiene.
 */
int irc_has_voice(struct ircchan* chan, struct ircuser* user);

/**
 * Da la voz a un usuario en un canal.
 * @param  chan Canal
 * @param  user Usuario
 * @return      OK o ERR_REPEAT si el usuario ya tenía la voz.
 */
int irc_give_voice(struct ircchan* chan, struct ircuser* user);

/**
 * Quita la voz a un usuario en un canal.
 * @param  chan Canal
 * @param  user Usuario
 * @return      OK o ERR_NOTFOUND si el usuario no tenía la voz.
 */
int irc_remove_voice(struct ircchan* chan, struct ircuser* user);

/**
 * Comprueba si el canal está moderado o no y si el usuario tiene la voz
 * 	para decidir si puede enviar mensajes.
 * @param  chan Canal.
 * @param  user Usuario.
 * @return      0 si no puede hablar, otro valor en otro caso.
 */
int irc_can_talk_in_channel(struct ircchan* chan, struct ircuser* user);



#endif

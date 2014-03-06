#ifndef IRC_CORE_H
#define IRC_CORE_H

#include "types.h"

struct irc_globdata* irc_init();
struct ircuser* irc_user_bynick(struct irc_globdata* gdata, const char* nick);
struct ircuser* irc_user_byid(struct irc_globdata* gdata, int id);
int irc_set_usernick(struct irc_globdata* data, int id, const char* nick);
int irc_create_user(struct irc_globdata* data, int id);
struct ircuser* irc_register_user(struct irc_globdata* data, int id);
void irc_destroy(struct irc_globdata* data);
int irc_channel_part(struct irc_globdata* data, struct ircchan* channel, struct ircuser* user);
void irc_delete_user(struct irc_globdata* data, struct ircuser* user);

/**
* Añade un usuario a un canal.
* En caso de crearse un canal, el argumento key es descartado. Se tendrá que ejecutar mode después.
* 	Además, el topic será el nombre del canal y el único usuario pertenciente, el que haya ejecutado JOIN.
* @param	data		La estructura que contiene los diccionarios.
* @param 	channel 	El nombre del canal al que queremos añædir el usuario.
* @param 	user 		El usuario que queremos añadir al canal.
* @param	key			La clave del canal proporcionada por el usuario. En caso de que el usuario no haya proporcionado ninguna este argumento será NULL.
* @param 	channel 	Canal en el que se ha añadido el usuario.
* @param 	ret_users 	Lista de usuarios pertenecientes al canal.
* @return	Código de error.  Si la clave del canal no coincide, si es un canal para el que se necesita invitación,
* 								si el usuario ya pertenece son posibles casos de error. Si el canal no existe, se creará.
*/
int irc_channel_adduser(struct irc_globdata* data, char* channel_name, struct ircuser* user, char * key , struct ircchan * channel);


/***************************************************************************
								Falta:	 
***************************************************************************/

/**
* Busca el canal por nombre.
* @param 	data	La estructura que contiene los diccionarios.
* @param	name	Nombre del canal que buscar.
* @return	El ircchan correspondiente, o NULL en caso de no existir.
*/

struct ircchan * irc_channel_byname(struct irc_globdata* data, char * name);


/**
* Busca al usuario en el canal.
* @param	channel 	Canal en el que buscar al usuario.
* @param	user 		Usuario a ser buscado.
* @return 	OK si se ha encontrado, ERR_NOTFOUND si no.
*/
short irc_user_inchannel(struct ircchan * channel, struct ircuser * user);


struct ircchan * irc_channel_create(char * name, int modes);

#endif

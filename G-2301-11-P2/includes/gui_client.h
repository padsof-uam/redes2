/***************************************************************************************************
* Redes de Comunicaciones II                                                                       *
* 25/2/2014                                                                                        *
*                                                                                                  *
*     Include necesario para el uso de las funciones públicas definidas para la práctica           *
*                                                                                                  *
***************************************************************************************************/

#ifndef GUI_CLIENT_H
#define GUI_CLIENT_H

#include <glib.h>
#include "irc_ftp.h"

#ifndef FALSE
	#define FALSE	0
#endif
#ifndef TRUE
	#define TRUE	1
#endif

#define USERSETTINGS_FILE ".redirc_user"

/* Función de ventana de error */
void errorWindow(char *msg);

/* Pregunta al usuario. */
int promptWindow(const char* msg);

void ftp_uicallback(ftp_status status);

/* Interfaz de impresión de textos */
void publicText(const char *username, const char *text);
void privateText(const char *username, const char *text);
void errorText(const char *errormessage, ...);
void messageText(const char *message, ...);
void noticeText(const char* message, ...);
void actionText(const char* username, const char *message, ...);

/* Funciones de conexión y desconexión */
void connectClient(void);
void disconnectClient(const char* bye_msg);

void connectToServer(const char *server, const char* port);
void connectToFavServ(int favserv);

/* Funciones llamadas cuando se produce un cambio de estado */
void topicProtect(gboolean state);
void externMsg(gboolean state);
void secret(gboolean state);
void guests(gboolean state);
void privated(gboolean state);
void moderated(gboolean state);

/* Funciones para cambiar el estado */
void setTopicProtect(gboolean state, gboolean enabled);
void setExternMsg(gboolean state, gboolean enabled);
void setSecret(gboolean state, gboolean enabled);
void setGuests(gboolean state, gboolean enabled);
void setPrivate(gboolean state, gboolean enabled);
void setModerated(gboolean state, gboolean enabled);

/* Función llamada cuando se introduce una entrada */
void newText (const char *msg);

void setApodo(const char* text);
void setNombre(const char* text);
void setNombreReal(const char* text);
void setServidor(const char* text);
void setPuerto(const char* text);

char * getApodo();
char * getNombreReal();
char * getNombre();
char * getServidor();
char * getPuerto();

int saveUserSettings(const char* nick, const char* name, const char* realname);
void tryReadUserSettings();
void setUserConnectionState(const gboolean isConnected);

int spawn_thread_gui(pthread_t *recv_thread,int argc, const char**argv);

void * thread_gui(void * data);

void enableChanMods();
void disableChanMods();

struct gui_thdata
{
	int queue;
	int argc;
	char ** argv;
};



#endif

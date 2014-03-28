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

#ifndef FALSE
	#define FALSE	0
#endif
#ifndef TRUE
	#define TRUE	1
#endif

/* Función de ventana de error */
void errorWindow(char *msg);

/* Interfaz de impresión de textos */
void publicText(char *username, char *text);
void privateText(char *username, char *text);
void errorText(char *errormessage, ...);
void messageText(char *message, ...);

/* Funciones de conexión y desconexión */
void connectClient(void);
void disconnectClient(void);

/* Funciones llamadas cuando se produce un cambio de estado */
void topicProtect(gboolean state);
void externMsg(gboolean state);
void secret(gboolean state);
void guests(gboolean state);
void privated(gboolean state);
void moderated(gboolean state);

/* Funciones para cambiar el estado */
void setTopicProtect(gboolean state);
void setExternMsg(gboolean state);
void setSecret(gboolean state);
void setGuests(gboolean state);
void setPrivated(gboolean state);
void setModerated(gboolean state);

/* Función llamada cuando se introduce una entrada */
void newText (const char *msg);

char * getApodo();
char * getNombre();
char * getNombreReal();
char * getServidor();
char * getPuerto();

int spawn_thread_gui(pthread_t *recv_thread,int argc, const char**argv);

void * thread_gui(void * data);

struct gui_thdata
{
	int queue;
	int argc;
	char ** argv;
};



#endif

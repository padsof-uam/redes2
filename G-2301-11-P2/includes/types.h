#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "dictionary.h"
#include "list.h"
#include "voicecall.h"

#ifndef COMMSTRUCTS_H
#define COMMSTRUCTS_H

#define MAX_IRC_MSG 512
#define MAX_SERVER_NAME 100

/**
 * Constantes que definen la longitud de las cadenas.
 */
#define MAX_NICK_LEN 9
#define MAX_NAME_LEN 100
#define MAX_CHAN_LEN 200
#define MAX_KEY_LEN 50
#define MAX_TOPIC_LEN 500
#define MAX_AWAYMSG_LEN 100

#define MAX_MEMBERS_IN_CHANNEL 100
#define MAX_CHANNELES_USER 10

#define MAX_ERR_THRESHOLD 5

#define SERVER_VERSION 0

/**
* Longitud máxima de los tamaños ftp.
*/
#define MAX_LEN_FTP 10240
#define FTP_TIMEOUT_MS 700

typedef enum {
	call_none, call_incoming, call_outgoing, call_running
} call_status;

/**
 * Modos de canal como banderas binarias.
 */
typedef enum  { 
	chan_moderated = 1, chan_priv = 2, chan_secret = 4, chan_invite = 8, 
	chan_topiclock = 16, chan_nooutside = 32 
} chan_mode;

/**
 * Modos de usuario como banderas binarias.
 */
typedef enum {
	user_invisible = 1, user_rcvnotices = 2, user_rcvwallops = 4, user_op = 8
} user_mode;

/**
 * Estructura de canal de IRC.
 */
struct ircchan {
	char name[MAX_CHAN_LEN + 1];
	char topic[MAX_TOPIC_LEN + 1];
	chan_mode mode; /**< Modos de canal. @see chan_mode >*/
	list* users; /**< Lista de punteros a las estructuras de usuario de los participantes del canal>. */
	dictionary* invited_users; /**< Diccionario de punteros a las estructuras de usuarios con los nick's como clave>*/
	list* operators; /**< Lista de punteros a las estructuras de usuario de los operadores */
	list* banned_users;
	list* voiced_users;
	char password[MAX_KEY_LEN + 1];
	short has_password;
	int user_limit;
};

/**
 * Estructura de usuario de IRC.
 */
struct ircuser {
	int fd; /**< Descriptor del socket de comunición con este usuario. Se utiliza como identificador del usuario*/
	char nick[MAX_NICK_LEN + 1];
	char name[MAX_NAME_LEN + 1];
	char username[MAX_NICK_LEN + 1];
	short is_away;
	char away_msg[MAX_AWAYMSG_LEN + 1];
	user_mode mode;
	list* channels; /**<Lista de punteros a estructuras de los canales a los que pertenece.>*/
};

/**
 * Enumerado de posibles estados de una conexión ftp
 */
typedef enum {
	ftp_finished, ftp_aborted, ftp_timeout, ftp_started
} ftp_status;

struct ftp_connection
{
	ftp_status status;
	pthread_t * th_manager;
	pthread_mutex_t stop_mutex;
	pthread_cond_t stop_cond;
};

struct irc_clientdata {
	short connected;
	char chan[MAX_CHAN_LEN + 1];
	char nick[MAX_NICK_LEN + 1];
	short in_channel;
	int serv_sock;
	int chanmode;
	int call_socket;
	uint32_t call_ip;
	uint16_t call_port;
	call_status call_status;
	struct cm_info call_info;
	char call_user[MAX_NICK_LEN + 1];
	uint32_t client_ip;
	struct ftp_connection * ftp_con_data;
};

/**
 * Estructura global de los datos del servidor IRC.
 * Se utilizan 2 diccionarios de usuarios que apuntan a las mismas estructuras de usuario, uno con los nicks y otro con los descriptores de socket.
 * Para los canales se utiliza una lista y un diccionario con entradas repetidas entre ambas estructuras con los canales.
 */
struct irc_globdata {
	dictionary* fd_user_map; /**< Diccionario con fd (socket de comunicación con usuario) como clave y el puntero al usuario como valor*/
	dictionary* nick_user_map; /**< Diccionario nick_usuario (clave) -> usuario (valor) >*/
	dictionary* chan_map; /**< Diccionario nombre_canal (clave) -> canal (valor)*/
	list* chan_list; /**< Lista de los canales >*/
	dictionary* oper_passwords; /**< Diccionario usuario -> contraseña operador */
	char servername[MAX_SERVER_NAME];
};


struct irc_msgdata {
	struct sockcomm_data* msgdata;
	struct irc_globdata* globdata;
	struct irc_clientdata* clientdata;
	char* msg;
	list* msg_tosend;
	int connection_to_terminate;
};	

/**
 * Estructura de mensaje de comunicación entre los hilos del servidor.
 */
struct sockcomm_data {
	int fd;
	char data[MAX_IRC_MSG + 1];
	ssize_t len;
};

struct msg_sockcommdata {
	long msgtype;
	struct sockcomm_data scdata;
};

#endif

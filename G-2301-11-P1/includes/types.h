#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "dictionary.h"
#include "list.h"

#ifndef COMMSTRUCTS_H
#define COMMSTRUCTS_H

#define MAX_IRC_MSG 512
#define MAX_SERVER_NAME 100

#define MAX_NICK_LEN 9
#define MAX_NAME_LEN 100
#define MAX_CHAN_LEN 200
#define MAX_KEY_LEN 50
#define MAX_TOPIC_LEN 500
#define MAX_AWAYMSG_LEN 100

#define MAX_MEMBERS_IN_CHANNEL 100
#define MAX_CHANNELES_USER 10

#define MAX_ERR_THRESHOLD 5

typedef enum  { 
	chan_moderated = 1, chan_priv = 2, chan_secret = 4, chan_invite = 8, 
	chan_topiclock = 16, chan_nooutside = 32 
} chan_mode;

typedef enum {
	user_invisible = 1, user_rcvnotices = 2, user_rcvwallops = 4, user_op = 8
} user_mode;

struct ircchan {
	char name[MAX_CHAN_LEN + 1];
	char topic[MAX_TOPIC_LEN + 1];
	chan_mode mode;
	list* users; /* Lista de usuarios. */
	dictionary* invited_users;
	list* operators; /**< Lista de cadenas con los nicks de los operadores */
	char password[MAX_KEY_LEN + 1];
	short has_password;
	int user_limit;
};

struct ircuser {
	int fd;
	char nick[MAX_NICK_LEN + 1];
	char name[MAX_NAME_LEN + 1];
	char username[MAX_NICK_LEN + 1];
	short is_away;
	char away_msg[MAX_AWAYMSG_LEN + 1];
	user_mode mode;
	list* channels; /* Lista de canales*/
};

struct irc_globdata {
	dictionary* fd_user_map;
	dictionary* nick_user_map;
	dictionary* chan_map;
	list* chan_list;
	dictionary* oper_passwords; /**< Diccionario usuario -> contraseña operador */
	char servername[MAX_SERVER_NAME];
};

struct irc_msgdata {
	struct sockcomm_data* msgdata;
	struct irc_globdata* globdata;
	char* msg;
	list* msg_tosend;
	short terminate_connection;
};	

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

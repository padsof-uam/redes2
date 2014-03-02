#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "dictionary.h"
#include "list.h"

#ifndef COMMSTRUCTS_H
#define COMMSTRUCTS_H

#define MAX_IRC_MSG 512
#define MAX_NICK_LEN 30
#define MAX_NAME_LEN 100
#define MAX_CHAN_LEN 200
#define MAX_TOPIC_LEN 500
#define MAX_ERR_THRESHOLD 5

typedef enum  { 
	chan_op = 1, chan_priv = 2, chan_secret = 4, chan_invite = 8, 
	chan_optopic = 16, chan_nooutside = 32, chan_moderated = 64 
} chan_mode;

typedef enum {
	user_invisible = 1, user_rcvnotices = 2, user_rcvwallops = 4, user_wallops = 8
} user_mode;

struct ircchan {
	char name[MAX_CHAN_LEN];
	char topic[MAX_TOPIC_LEN];
	chan_mode mode;
	list* users;
};

struct ircuser {
	int fd;
	char nick[MAX_NICK_LEN];
	char name[MAX_NICK_LEN];
	short is_away;
	user_mode mode;
};

struct irc_globdata {
	dictionary* fd_user_map;
	dictionary* nick_user_map;
	dictionary* chan_map;
	list* chan_list;
};

struct irc_msgdata {
	struct sockcomm_data* msgdata;
	struct irc_globdata* globdata;
	char* msg;
	list* msg_tosend;
};	

struct sockcomm_data {
	int fd;
	char data[MAX_IRC_MSG];
	ssize_t len;
};

struct msg_sockcommdata {
	long msgtype;
	struct sockcomm_data scdata;
};

typedef enum {
	MASTER_STOP_REQ, CHILD_CRITICAL_ERR
} master_commcode;

struct msg_mastercomm {
	long msgtype;
	master_commcode code;
};

#endif

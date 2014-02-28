#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "dictionary.h"
#include "list.h"


#ifndef COMMSTRUCTS_H
#define COMMSTRUCTS_H

#define MAX_IRC_MSG 512
#define MAX_ERR_THRESHOLD 5

struct irc_globdata {
	dictionary* fd_user_map;
	dictionary* nick_fd_cachemap;
	list* chan_list;
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

#endif

#ifndef SERVER_HISTORY
#define SERVER_HISTORY

#include <stdio.h>

#define FAVSERVS_FILE ".redirc_favservs"

#define SERV_NAMELEN 50
#define MAX_SERV_HISTORY 20
#define MAX_PORT_LEN 10

struct serv_info {
	char servname[SERV_NAMELEN];
	char port[MAX_PORT_LEN];
	int times_used;
};

int serv_get_number(int servnum, struct serv_info* info);
int serv_save_connection(struct serv_info* info);
int parse_servinfo(char* line, struct serv_info* info);
int serv_getlist(FILE* f, struct serv_info* list, size_t list_size);
int serv_save_connection_to(const char* file, struct serv_info *info);

#endif

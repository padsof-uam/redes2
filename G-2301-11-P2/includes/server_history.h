#ifndef SERVER_HISTORY
#define SERVER_HISTORY

#define FAVSERVS_FILE ".redirc_favservs"

#define SERV_NAMELEN 100
#define MAX_SERV_HISTORY 20
#define MAX_PORT_LEN 10

struct serv_info {
	char servname[SERV_NAMELEN];
	char port[MAX_PORT_LEN];
	int times_used;
};

int serv_get_number(int servnum, struct serv_info* info);
int serv_save_connection(struct serv_info* info);

#endif

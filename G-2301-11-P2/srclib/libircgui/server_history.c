#include "server_history.h"
#include "types.h"
#include "log.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static int parse_servinfo(char* line, struct serv_info* info)
{
	char *server, *port, *times_used;

	server = strsep(&line, " ");
	port = strsep(&line, " ");
	times_used = strsep(&line, " ");

	if(!server)
		return ERR_PARSE;

	if(!port)
		port = "6667";

	if(!times_used)
		times_used = "0";

	strncpy(info->servname, server, SERV_NAMELEN);
	strncpy(info->port, port, MAX_PORT_LEN);
	info->times_used = strtol(times_used, NULL, 10);

	return OK;
}

static int _servhistory_getlist(FILE* f, struct serv_info* list, size_t list_size)
{
	int linenum = 0;
	char line[200];

	while(linenum < list_size && fgets(line, 200, f) != NULL)
		parse_servinfo(line, list + linenum);

	if(feof(f) || linenum == list_size)
		return linenum;
	else
		return ERR_SYS;
}

static int _serv_compare(const void* a, const void* b)
{
	const struct serv_info* sa = a;
	const struct serv_info* sb = b;

	return sa->times_used - sb->times_used;
}

int serv_get_number(int servnum, struct serv_info *info)
{
    FILE *serv_f = fopen(FAVSERVS_FILE, "r");
    char line[200];
    int line_num = 0;

    if (!serv_f)
    {
        if (errno == ENOENT)
            return ERR_NOTFOUND;
        else
            return ERR_SYS;
    }

    while(fgets(line, 200, serv_f) && line_num < servnum);
	
	if(feof(serv_f))
	{
		fclose(serv_f);
		return ERR_NOTFOUND;
	}
	else if(ferror(serv_f))
	{
		fclose(serv_f);
		return ERR_SYS;
	}
 
 	parse_servinfo(line, info);
	return OK;
}

int serv_save_connection(struct serv_info *info)
{
	FILE *serv_f = fopen(FAVSERVS_FILE, "r");
	struct serv_info infolist[MAX_SERV_HISTORY];
	int i, serv_size = 0;

	if(serv_f)	
		serv_size = _servhistory_getlist(serv_f, infolist, MAX_SERV_HISTORY);

	if(serv_size < 0)
	{
		slog(LOG_ERR, "Error %s leyendo lista de servidores preferidos", strerror(errno));
		return ERR_SYS;
	}

	for(i = 0; i < serv_size; i++)
	{
		if(!strncmp(infolist[i].servname, info->servname, SERV_NAMELEN) && 
			!strncmp(infolist[i].port, info->port, MAX_PORT_LEN))
			break;
	}

	if(i == serv_size) /* Not found */
	{
		if(i == MAX_SERV_HISTORY)
			i--;
		else
			serv_size++;

		strncpy(infolist[i].servname, info->servname, SERV_NAMELEN);
		strncpy(infolist[i].port, info->port, MAX_PORT_LEN);
		infolist[i].times_used = 1;
	}
	else
	{
		infolist[i].times_used++;
	}

	qsort(infolist, serv_size, sizeof(struct serv_info), _serv_compare);

	fclose(serv_f);
	serv_f = fopen(FAVSERVS_FILE, "w");

	for(i = 0; i < serv_size; i++)
		fprintf(serv_f, "%s %s %d\n", infolist[i].servname, infolist[i].port, infolist[i].times_used);

	fclose(serv_f);

	return OK;
}

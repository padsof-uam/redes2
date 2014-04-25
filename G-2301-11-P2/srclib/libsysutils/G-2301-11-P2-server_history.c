#include "G-2301-11-P2-server_history.h"
#include "G-2301-11-P2-types.h"
#include "G-2301-11-P2-log.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

int parse_servinfo(char *line, struct serv_info *info)
{
    char *server, *port, *times_used;

    server = strsep(&line, " ");
    port = strsep(&line, " ");
    times_used = strsep(&line, " ");

    if (!server)
        return ERR_PARSE;

    if (!port)
        port = "6667";

    if (!times_used)
        times_used = "0";

    strncpy(info->servname, server, SERV_NAMELEN);
    strncpy(info->port, port, MAX_PORT_LEN);
    info->times_used = strtol(times_used, NULL, 10);

    return OK;
}

int serv_getlist(FILE *f, struct serv_info *list, size_t list_size)
{
    int linenum = 0;
    char line[200];

    while (linenum < list_size && fgets(line, 200, f) != NULL)
    {
        parse_servinfo(line, list + linenum);
        linenum++;
    }

    if (feof(f) || linenum == list_size)
        return linenum;
    else
        return ERR_SYS;
}

static int _serv_compare(const void *a, const void *b)
{
    const struct serv_info *sa = a;
    const struct serv_info *sb = b;

    return sb->times_used - sa->times_used;
}

int serv_get_number(int servnum, struct serv_info *info)
{
    return serv_get_number_in(FAVSERVS_FILE, servnum, info);
}

int serv_get_number_in(const char* file, int servnum, struct serv_info* info)
{   
    size_t lsize;
    struct serv_info list[MAX_SERV_HISTORY];
    FILE *serv_f;

    if(servnum < 0)
        return ERR_RANGE;

    serv_f = fopen(file, "r");

    if (!serv_f)
    {
        if (errno == ENOENT)
            return ERR_NOTFOUND;
        else
            return ERR_SYS;
    }

    lsize = serv_getlist(serv_f, list, servnum + 1);
    fclose(serv_f);

    if(servnum >= lsize)
        return ERR_RANGE;

    memcpy(info, list + servnum, sizeof(struct serv_info));

    return OK;
}

int serv_save_connection(struct serv_info *info)
{
    return serv_save_connection_to(FAVSERVS_FILE, info);
}

int serv_save_connection_to(const char *file, struct serv_info *info)
{
    FILE *serv_f = fopen(file, "r");
    struct serv_info infolist[MAX_SERV_HISTORY];
    int i, serv_size = 0;

    if (serv_f)
    {
        serv_size = serv_getlist(serv_f, infolist, MAX_SERV_HISTORY);
        fclose(serv_f);
    }
    
    if (serv_size < 0)
    {
        slog(LOG_ERR, "Error %s leyendo lista de servidores preferidos", strerror(errno));
        return ERR_SYS;
    }

    for (i = 0; i < serv_size; i++)
    {
        if (!strncmp(infolist[i].servname, info->servname, SERV_NAMELEN) &&
                !strncmp(infolist[i].port, info->port, MAX_PORT_LEN))
            break;
    }

    if (i == serv_size) /* Not found */
    {
        if (i == MAX_SERV_HISTORY)
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

    serv_f = fopen(file, "w");

    for (i = 0; i < serv_size; i++)
        fprintf(serv_f, "%s %s %d\n", infolist[i].servname, infolist[i].port, infolist[i].times_used);

    fclose(serv_f);

    return OK;
}

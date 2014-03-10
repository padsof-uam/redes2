#include "poller.h"

#include <stdlib.h>
#include <string.h>

struct pollfds *pollfds_init(short default_flags)
{
    struct pollfds *pfds = (struct pollfds *) malloc(sizeof(struct pollfds));

    if (!pfds)
        return NULL;

    pfds->fds = (struct pollfd *) calloc(DEFAULT_PFDS_LEN, sizeof(struct pollfd));

    if (!pfds->fds)
    {
        free(pfds);
        return NULL;
    }

    pfds->len = 0;
    pfds->capacity = DEFAULT_PFDS_LEN;
    pfds->flags = default_flags;

    return pfds;
}

int pollfds_setcapacity(struct pollfds *pfds, int capacity)
{
    struct pollfd *new_fds_list = NULL;

    if (capacity < pfds->capacity)
        return OK;

    new_fds_list = realloc(pfds->fds, capacity * sizeof(struct pollfd));

    if (!new_fds_list)
        return ERR;

    pfds->fds = new_fds_list;
    pfds->capacity = capacity;

    return OK;
}

int pollfds_add(struct pollfds *pfds, int fd)
{
    if (pfds->len == pfds->capacity)
        if (pollfds_setcapacity(pfds, 2 * pfds->capacity))
            return ERR;

    pfds->fds[pfds->len].fd = fd;
    pfds->fds[pfds->len].events = pfds->flags;
    pfds->len++;

    return OK;
}

int pollfds_remove(struct pollfds *pfds, int fd)
{
    int i;

    for (i = 0; i < pfds->len; i++)
        if (pfds->fds[i].fd == fd)
            break;

    if (i == pfds->len)
        return ERR;

    pfds->len--;

    if(i != pfds->len)
        memcpy(pfds->fds + i, pfds->fds + pfds->len, sizeof(struct pollfd));

    return OK;
}

int pollfds_poll(struct pollfds *pfds, int timeout)
{
    return poll(pfds->fds, pfds->len, timeout);
}

void pollfds_destroy(struct pollfds *pfds)
{
    if (pfds)
    {
        if (pfds->fds)
            free(pfds->fds);
        free(pfds);
    }
}

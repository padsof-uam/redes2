#include "poller.h"

#include <stdlib.h>
#include <string.h>

/**
 * initialize a dynamic pollfds structure.
 * @param default_flags default flags for new fds.
 * @return pollfds structure.
 */
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

/**
 * Set the capacity for the dynamic struct list.
 * @param  capacity New capacity.
 * @return OK if correct, ERR if error.
 */
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

/**
 * Add a file descriptor
 * @param pfds poll fds structure.
 * @param fd   fd to add.
 */
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

/**
 * Remove a file descriptor.
 * @param pfds poll fds structure.
 * @param fd   file descriptor.
 * @return error code.
 */
int pollfds_remove(struct pollfds *pfds, int fd)
{
    int i;

    for (i = 0; i < pfds->len; i++)
        if (pfds->fds[i].fd == fd)
            break;

    if (i == pfds->len)
        return ERR;

    pfds->len--;
    memcpy(pfds->fds + i, pfds->fds + pfds->len, sizeof(struct pollfd));

    return OK;
}

/**
 * Executes the poll(2) system call.
 * @param  pfds    Poll fds structure
 * @param  timeout Timeout as defined in poll man.
 * @return         fds ready for I/O
 */
int pollfds_poll(struct pollfds *pfds, int timeout)
{
    return poll(pfds->fds, pfds->len, timeout);
}

/**
 * Destroy memory and free resources
 * @param pfds poll fds structure
 */
void pollfds_destroy(struct pollfds *pfds)
{
    if (pfds)
    {
        if (pfds->fds)
            free(pfds->fds);
        free(pfds);
    }
}

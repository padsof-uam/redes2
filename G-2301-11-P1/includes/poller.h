#ifndef POLLER_H
#define POLLER_H

#include <poll.h>
#include "errors.h"

#define DEFAULT_PFDS_LEN 20

struct pollfds {
	struct pollfd* fds;
	int len;
	int capacity;
	short flags;
};

/**
 * initialize a dynamic pollfds structure.
 * @param default_flags default flags for new fds.
 * @return pollfds structure.
 */
struct pollfds* pollfds_init(short default_flags);

/**
 * Set the capacity for the dynamic struct list.
 * @param  capacity New capacity.
 * @return error code.
 */
int pollfds_setcapacity(struct pollfds* pfds, int capacity);

/**
 * Add a file descriptor 
 * @param pfds poll fds structure.
 * @param fd   fd to add.
 * @return error code.
 */
int pollfds_add(struct pollfds* pfds, int fd);

/**
 * Remove a file descriptor.
 * @param pfds poll fds structure.
 * @param fd   file descriptor.
 * @return error code.
 */
int pollfds_remove(struct pollfds* pfds, int fd);

/**
 * Executes the poll(2) system call.
 * @param  pfds    Poll fds structure
 * @param  timeout Timeout as defined in poll man.
 * @return         fds ready for I/O
 */
int pollfds_poll(struct pollfds* pfds, int timeout);

/**
 * Destroy memory and free resources
 * @param pfds poll fds structure
 */
void pollfds_destroy(struct pollfds* pfds);

#endif

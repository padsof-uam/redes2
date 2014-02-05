#ifndef DAEMONIZE_H
#define DAEMONIZE_H 

int daemonize(const char* log_id);
int close_open_fds();
int unlink_proc();

#endif

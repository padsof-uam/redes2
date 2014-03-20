#ifndef CMD_PROCESSOR_H
#define CMD_PROCESSOR_H

#include <pthread.h>
#include "types.h"


typedef void (*msg_process)(int snd_qid, struct sockcomm_data* data, struct irc_globdata* arg);

int spawn_proc_thread(pthread_t *pth, int rcv_qid, int snd_qid, msg_process pfun, const char* conf_file);
void* proc_thread_entrypoint(void* data);
#endif


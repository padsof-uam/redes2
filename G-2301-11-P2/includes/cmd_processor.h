#ifndef CMD_PROCESSOR_H
#define CMD_PROCESSOR_H

#include <pthread.h>
#include "types.h"


typedef void (*msg_process)(int snd_qid, struct sockcomm_data* data, void* pass_through);

int spawn_proc_thread(pthread_t *pth, int rcv_qid, int snd_qid, msg_process pfun, void* pass_through);
void* proc_thread_entrypoint(void* data);
#endif


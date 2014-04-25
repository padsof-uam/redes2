#include "G-2301-11-P2-cmd_processor.h"
#include "G-2301-11-P2-log.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>
#include "G-2301-11-P2-errors.h"
#include "G-2301-11-P2-types.h"

struct proc_thdata
{
    int rcv_qid;
    int snd_qid;
    msg_process pfun;
    void *pass_through;
};

int spawn_proc_thread(pthread_t *pth, int rcv_qid, int snd_qid, msg_process pfun, void *pass_through)
{
    struct proc_thdata *thdata = malloc(sizeof(struct proc_thdata));
    thdata->rcv_qid = rcv_qid;
    thdata->snd_qid = snd_qid;
    thdata->pass_through = pass_through;
    thdata->pfun = pfun;

    if (pthread_create(pth, NULL, proc_thread_entrypoint, thdata) < 0 )
    {
        slog(LOG_CRIT, "No se ha podido iniciar proc_thread: %s", strerror(errno));
        return ERR;
    }

    slog(LOG_DEBUG, "Hilo de procesado iniciado.");

    return OK;
}

static void _proc_thread_cleanup(void *thdata)
{
    slog(LOG_DEBUG, "proc: limpiando.");

    free(thdata);
}

void *proc_thread_entrypoint(void *data)
{
    struct proc_thdata *thdata = (struct proc_thdata *) data;
    struct msg_sockcommdata msg;

    pthread_cleanup_push(_proc_thread_cleanup, thdata);

    slog(LOG_DEBUG, "Hilo de procesado iniciado.");

    while (1)
    {
        if (msgrcv(thdata->rcv_qid, &msg, sizeof(struct sockcomm_data), 0, 0) != -1)
        {
            thdata->pfun(thdata->snd_qid, &msg.scdata, thdata->pass_through);
        }
        else
        {
            slog(LOG_ERR, "error recibiendo de msgrcv: %s", strerror(errno));
        }
    }

    pthread_cleanup_pop(0);
    return NULL;
}


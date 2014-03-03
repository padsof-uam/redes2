#include "cmd_processor.h"
#include <stdlib.h>
#include <sys/syslog.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>
#include "errors.h"
#include "types.h"
#include "irc_core.h"

struct proc_thdata
{
    int rcv_qid;
    int snd_qid;
    msg_process pfun;
};

int spawn_proc_thread(pthread_t *pth, int rcv_qid, int snd_qid, msg_process pfun)
{
    struct proc_thdata *thdata = malloc(sizeof(struct proc_thdata));
    thdata->rcv_qid = rcv_qid;
    thdata->snd_qid = snd_qid;
    thdata->pfun = pfun;

    if (pthread_create(pth, NULL, proc_thread_entrypoint, thdata) < 0 )
    {
        syslog(LOG_CRIT, "No se ha podido iniciar proc_thread: %s", strerror(errno));
        return ERR;
    }

    syslog(LOG_DEBUG, "Hilo de procesado iniciado.");

    return OK;
}

static void _proc_thread_cleanup(void *thdata)
{
    syslog(LOG_DEBUG, "proc: limpiando.");

    free(thdata);
}

void *proc_thread_entrypoint(void *data)
{
    struct proc_thdata *thdata = (struct proc_thdata *) data;
    struct msg_sockcommdata msg;
    struct irc_globdata* gdata; 

    gdata = irc_init();

    pthread_cleanup_push(_proc_thread_cleanup, thdata);
    pthread_cleanup_push((void (*)(void *))irc_destroy, gdata);

    syslog(LOG_DEBUG, "Hilo de procesado iniciado.");

    if (gdata)
    {
        while (1)
        {
            if (msgrcv(thdata->rcv_qid, &msg, sizeof(struct msg_sockcommdata), 0, 0) != -1)
            {
                thdata->pfun(thdata->snd_qid, &msg.scdata, gdata);
            }
            else
            {
                syslog(LOG_ERR, "error recibiendo de msgrcv: %s", strerror(errno));
            }
        }
    }
    else
    {
        syslog(LOG_CRIT, "error inicializando estructuras: %s", strerror(errno));
    }

    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    return NULL;
}


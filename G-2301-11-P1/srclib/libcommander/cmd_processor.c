#include "cmd_processor.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>
#include "errors.h"
#include "types.h"
#include "irc_core.h"

#define MAX_CONF_LEN 100

struct proc_thdata
{
    int rcv_qid;
    int snd_qid;
    msg_process pfun;
    char conffile[MAX_CONF_LEN + 1];
};

int spawn_proc_thread(pthread_t *pth, int rcv_qid, int snd_qid, msg_process pfun, const char* conf_file)
{
    struct proc_thdata *thdata = malloc(sizeof(struct proc_thdata));
    thdata->rcv_qid = rcv_qid;
    thdata->snd_qid = snd_qid;
    thdata->pfun = pfun;

    strncpy(thdata->conffile, conf_file, MAX_CONF_LEN);
    thdata->conffile[MAX_CONF_LEN] = '\0';

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
    struct irc_globdata* gdata; 
    int retval;

    gdata = irc_init();

    pthread_cleanup_push(_proc_thread_cleanup, thdata);
    pthread_cleanup_push((void (*)(void *))irc_destroy, gdata);

    slog(LOG_DEBUG, "Hilo de procesado iniciado.");

    if (gdata)
    {
        retval = irc_load_config(gdata, thdata->conffile);

        if(retval != OK)
            slog(LOG_WARNING, "No se ha podido cargar correctamente la configuración: %d. %s", retval, strerror(errno));
        else
            slog(LOG_NOTICE, "Cargada configuración.");

        while (1)
        {
            if (msgrcv(thdata->rcv_qid, &msg, sizeof(struct sockcomm_data), 0, 0) != -1)
            {
                thdata->pfun(thdata->snd_qid, &msg.scdata, gdata);
            }
            else
            {
                slog(LOG_ERR, "error recibiendo de msgrcv: %s", strerror(errno));
            }
        }
    }
    else
    {
        slog(LOG_CRIT, "error inicializando estructuras: %s", strerror(errno));
    }

    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    return NULL;
}


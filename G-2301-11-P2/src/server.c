#include "daemonize.h"
#include "errors.h"
#include "listener.h"
#include "poller.h"
#include "list.h"
#include "receiver.h"
#include "sender.h"
#include "cmd_processor.h"
#include "irc_processor.h"
#include "sysutils.h"
#include "log.h"
#include "irc_core.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/file.h>
#include <time.h>

#define IRC_PORT 6667
#define LOG_ID "redirc"
#define MAX_ERR_THRESHOLD 5
#define RCV_QKEY 3981
#define SND_QKEY 3982
#define MASTER_QKEY 3983
#define PID_FILE "/tmp/redirc.pid"
#define LOCK_FILE "/tmp/redirc.lock"
#define DEFAULT_CONF_FILE "redirc.conf"

/**
 * Macro para cancelar un hilo si está funcionando.
 * Deberá existir una variable th_name_running que indique si está funcionando,
 *     y otra th_name_th con el identificador del hilo (tipo pthread_t).
 * @param th_name Nombre del hilo.
 */
#define irc_pth_exit(th_name) do { if(th_name##_running) pthread_cancel_join(th_name##_th); } while(0)
#define irc_exit(code) do { retval = code; goto cleanup; } while (0);

pthread_mutex_t stop_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t stop_cond = PTHREAD_COND_INITIALIZER;
sig_atomic_t stop = 0;

static void pthread_cancel_join(pthread_t th)
{
    int err;
    if ((err = pthread_cancel(th)) < 0)
    {
        slog(LOG_ERR, "error cerrando hilo: %d", err);
        return;
    }

    if ((err = pthread_join(th, NULL)) < 0)
    {
        slog(LOG_ERR, "error saliendo del hilo: %d", err);
        return;
    }
}

static void capture_signal(int sig)
{
    if (sig == SIGTERM)
    {
        stop = 1;
        pthread_cond_signal(&stop_cond);
    }
}

int main(int argc, char const *argv[])
{
    int comm_socks[2];
    pthread_t listener_th = 0, receiver_th = 0, proc_th = 0, sender_th = 0;
    short listener_running = 0, receiver_running = 0, proc_running = 0, sender_running = 0;
    int rcv_qid = 0, snd_qid = 0, master_qid = 0;
    int retval = EXIT_SUCCESS;
    int kill_retval;
    const char *conffile;
    char conf_path[300];
    struct irc_globdata* gdata = NULL;
    int conf_retval;

    if (argc > 1 && !strcmp(argv[1], "stop"))
    {
        printf("Parando...\n");
        kill_retval = read_pid_and_kill(PID_FILE);

        if (kill_retval == 0)
            printf("Daemon parado.\n");
        else if (kill_retval == 1)
            printf("El daemon no responde. Salida forzada.\n");
        else
            fprintf(stderr, "No se ha podido parar el daemon: %s\n", strerror(errno));

        exit(EXIT_SUCCESS);
    }

    if (argc > 1)
        conffile = argv[1];
    else
        conffile = DEFAULT_CONF_FILE;

    if(realpath(conffile, conf_path) == NULL)
    {
        fprintf(stderr, "Archivo de configuración inexistente.\n");
        strncpy(conf_path, conffile, 300);
    }

    comm_socks[0] = 0;
    comm_socks[1] = 0;

    slog_set_level(LOG_DEBUG);
    slog_debug("asdjhka");

#ifndef NODAEMON
    if (daemonize(LOG_ID) != OK)
    {
        slog(LOG_CRIT, "No se ha podido daemonizar.");
        irc_exit(EXIT_FAILURE)
    }
#endif

    if (!try_getlock(LOCK_FILE))
    {
        slog(LOG_CRIT, "Ya hay una instancia en ejecución. Saliendo.");
        irc_exit(EXIT_FAILURE);
    }

    slog(LOG_NOTICE, "Daemon empezando.");

    if (write_pid(PID_FILE) < 0)
    {
        slog(LOG_ERR, "Error escribiendo en pid_file. Sólo se podrá parar el daemon con kill.");
    }

    if (signal(SIGTERM, capture_signal) == SIG_ERR)
    {
        slog(LOG_CRIT, "Error capturando señales.");
        irc_exit(EXIT_FAILURE);
    }

    if (socketpair(PF_LOCAL, SOCK_STREAM, 0, comm_socks) < 0)
    {
        slog(LOG_CRIT, "No se han podido crear los sockets de comunicación con listener: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }

    rcv_qid = msgget(RCV_QKEY, 0666 | IPC_PRIVATE | IPC_CREAT);

    if (rcv_qid == -1)
    {
        slog(LOG_CRIT, "No se ha podido crear la cola de mensajes de recepción: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }

    snd_qid = msgget(SND_QKEY, 0666 | IPC_PRIVATE | IPC_CREAT);

    if (snd_qid == -1)
    {
        slog(LOG_CRIT, "No se ha podido crear la cola de mensajes de envío: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }

    master_qid = msgget(MASTER_QKEY, 0666 | IPC_PRIVATE | IPC_CREAT);

    if (master_qid == -1)
    {
        slog(LOG_CRIT, "No se ha podido crear la cola de mensajes maestra: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }

    if (spawn_listener_thread(&listener_th, IRC_PORT, comm_socks[1]) < 0)
    {
        slog(LOG_CRIT, "No se ha podido crear el hilo de escucha: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }
    listener_running = 1;

    gdata = irc_init();

    if(!gdata)
    {
        slog(LOG_CRIT, "No se han podido crear las estructuras del servidor IRC: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }

    conf_retval = irc_load_config(gdata, conf_path);

    if(conf_retval != OK)
        slog(LOG_WARNING, "No se ha podido cargar correctamente la configuración: %d. %s", conf_retval, strerror(errno));
    else
        slog(LOG_NOTICE, "Cargada configuración.");

    if (spawn_receiver_thread(&receiver_th, comm_socks[0], rcv_qid) < 0)
    {
        slog(LOG_CRIT, "No se ha podido crear el hilo de recepción de datos: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }
    receiver_running = 1;

    if (spawn_proc_thread(&proc_th, rcv_qid, snd_qid, (msg_process) irc_server_msgprocess, gdata) < 0)
    {
        slog(LOG_CRIT, "No se ha podido crear el hilo de procesado: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }
    proc_running = 1;

    if (spawn_sender_thread(&sender_th, snd_qid) < 0)
    {
        slog(LOG_CRIT, "No se ha podido crear el hilo de envío: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }
    sender_running = 1;

    slog(LOG_NOTICE, "Todas las estructuras creadas. Daemon iniciado.");

    /* Nos dedicamos a esperar hasta que nos digan que paramos */
    pthread_mutex_lock(&stop_mutex);
    {
        while (!stop)
            pthread_cond_wait(&stop_cond, &stop_mutex);
    }
    pthread_mutex_unlock(&stop_mutex);

    slog(LOG_NOTICE, "Daemon saliendo...");

cleanup:
    irc_pth_exit(listener);
    irc_pth_exit(receiver);
    irc_pth_exit(proc);
    irc_pth_exit(sender);

    if(gdata)
        irc_destroy(gdata);

    if (rcv_qid > 0)
        msgctl(rcv_qid, IPC_RMID, NULL);

    if (snd_qid > 0)
        msgctl(snd_qid, IPC_RMID, NULL);

    if (master_qid > 0)
        msgctl(master_qid, IPC_RMID, NULL);

    if (comm_socks[0] != 0)
        close(comm_socks[0]);

    if (comm_socks[1] != 0)
        close(comm_socks[1]);

    slog(LOG_NOTICE, "Daemon terminado.\n");

    return retval;
}


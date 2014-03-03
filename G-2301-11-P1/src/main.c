#include "daemonize.h"
#include "errors.h"
#include "listener.h"
#include "poller.h"
#include "list.h"
#include "receiver.h"
#include "sender.h"
#include "cmd_processor.h"
#include "irc_processor.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/file.h>

#define IRC_PORT 6667
#define LOG_ID "redirc"
#define MAX_ERR_THRESHOLD 5
#define RCV_QKEY 3981
#define SND_QKEY 3982
#define MASTER_QKEY 3983
#define PID_FILE "/tmp/redirc.pid"
#define LOCK_FILE "/tmp/redirc.lock"

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
        syslog(LOG_ERR, "error cerrando hilo: %d", err);
        return;
    }

    if ((err = pthread_join(th, NULL)) < 0)
    {
        syslog(LOG_ERR, "error saliendo del hilo: %d", err);
        return;
    }
}

static void capture_signal(int sig)
{
    if (sig == SIGTERM)
    {
        pthread_cond_signal(&stop_cond);
        stop = 1;
    }
}

static int write_pid()
{
    FILE *pid_file;

    pid_file = fopen(PID_FILE, "w");

    if (!pid_file)
        return -ERR;

    fprintf(pid_file, "%d\n", getpid());
    fclose(pid_file);
    return OK;
}

static int send_kill_signal()
{
    FILE *pid_file;
    pid_t pid;
    int read;
    int max_ms_wait = 5000;
    int ms_wait_interval = 200;
    int ms_waited = 0;

    pid_file = fopen(PID_FILE, "r");

    if (!pid_file)
        return -ERR;

    read = fscanf(pid_file, "%d", &pid);
    fclose(pid_file);

    if (read < 1)
        return -ERR;

    if (kill(pid, SIGTERM) == -1)
    {
        perror("kill");
        return -ERR;
    }

    while (ms_waited < max_ms_wait)
    {
        if (kill(pid, 0) == -1) /* Devuelve -1 si el proceso ya ha salido */
            return OK;

        usleep(ms_wait_interval);
    }

    printf("Forzando salida con SIGKILL.\n");

    kill(pid, SIGKILL);

    return OK;
}

int try_getlock()
{
    int pid_file = open(LOCK_FILE, O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    
    if (rc && EWOULDBLOCK == errno)
            return 0;

    return 1;
}

int main(int argc, char const *argv[])
{
    int comm_socks[2];
    pthread_t listener_th = 0, receiver_th = 0, proc_th = 0, sender_th = 0;
    short listener_running = 0, receiver_running = 0, proc_running = 0, sender_running = 0;
    int rcv_qid = 0, snd_qid = 0, master_qid = 0;
    int retval = EXIT_SUCCESS;

    if (argc > 1 && !strcmp(argv[1], "stop"))
    {
        printf("Parando...\n");
        if (send_kill_signal() == OK)
            printf("Daemon parado.\n");
        else
            printf("No se ha podido parar el daemon.\n");

        exit(EXIT_SUCCESS);
    }

    comm_socks[0] = 0;
    comm_socks[1] = 0;

    if (daemonize(LOG_ID) != OK)
    {
        syslog(LOG_CRIT, "No se ha podido daemonizar.");
        irc_exit(EXIT_FAILURE)
    }

    if(!try_getlock())
    {
    	syslog(LOG_CRIT, "Ya hay una instancia en ejecución. Saliendo.");
    	irc_exit(EXIT_FAILURE);
    }

    syslog(LOG_NOTICE, "Daemon empezando.\n");

    if (write_pid() < 0)
    {
        syslog(LOG_ERR, "Error escribiendo en pid_file. Sólo se podrá parar el daemon con kill.");
    }

    if (signal(SIGTERM, capture_signal) == SIG_ERR)
    {
        syslog(LOG_CRIT, "Error capturando señales.");
        irc_exit(EXIT_FAILURE);
    }

    if (socketpair(PF_LOCAL, SOCK_STREAM, 0, comm_socks) < 0)
    {
        syslog(LOG_CRIT, "No se han podido crear los sockets de comunicación con listener: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }

    rcv_qid = msgget(RCV_QKEY, 0666 | IPC_PRIVATE | IPC_CREAT);

    if (rcv_qid == -1)
    {
        syslog(LOG_CRIT, "No se ha podido crear la cola de mensajes de recepción: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }

    snd_qid = msgget(SND_QKEY, 0666 | IPC_PRIVATE | IPC_CREAT);

    if (snd_qid == -1)
    {
        syslog(LOG_CRIT, "No se ha podido crear la cola de mensajes de envío: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }

    master_qid = msgget(MASTER_QKEY, 0666 | IPC_PRIVATE | IPC_CREAT);

    if (master_qid == -1)
    {
        syslog(LOG_CRIT, "No se ha podido crear la cola de mensajes maestra: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }

    /* sleep(50); */
    if (spawn_listener_thread(&listener_th, IRC_PORT, comm_socks[1]) < 0)
    {
        syslog(LOG_CRIT, "No se ha podido crear el hilo de escucha: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }
    listener_running = 1;

    if (spawn_receiver_thread(&receiver_th, comm_socks[0], rcv_qid) < 0)
    {
        syslog(LOG_CRIT, "No se ha podido crear el hilo de recepción de datos: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }
    receiver_running = 1;

    if (spawn_proc_thread(&proc_th, rcv_qid, snd_qid, irc_msgprocess) < 0)
    {
        syslog(LOG_CRIT, "No se ha podido crear el hilo de procesado: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }
    proc_running = 1;

    if (spawn_sender_thread(&sender_th, snd_qid) < 0)
    {
        syslog(LOG_CRIT, "No se ha podido crear el hilo de envío: %s", strerror(errno));
        irc_exit(EXIT_FAILURE);
    }
    sender_running = 1;

    syslog(LOG_NOTICE, "Todas las estructuras creadas. Daemon iniciado.");

    pthread_mutex_lock(&stop_mutex);

    while (!stop)
        pthread_cond_wait(&stop_cond, &stop_mutex);

    pthread_mutex_unlock(&stop_mutex);

    syslog(LOG_NOTICE, "Daemon saliendo...");

cleanup:
    irc_pth_exit(listener);
    irc_pth_exit(receiver);
    irc_pth_exit(proc);
    irc_pth_exit(sender);

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

    syslog(LOG_NOTICE, "Daemon terminado.\n");

    return retval;
}


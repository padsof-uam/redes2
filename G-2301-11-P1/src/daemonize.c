#include "daemonize.h"
#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#ifdef __APPLE__
#define FD_DIR "/dev/fd"
#else
#define FD_DIR "/proc/self/fd"
#endif

/* Hay alguna razón por la que esto sea estático? */
char * _log_id;

static void _open_log(){
    openlog(_log_id, 0, LOG_DAEMON);    
}

static void _close_log(){
    closelog();   
}


void finish_daemon (int signal){
    _close_log();
    exit(EXIT_SUCCESS);
}


int daemonize(const char *log_id)
{
    int pid;
    _log_id = strdup(log_id);
    umask(0);
    _open_log();
    setlogmask (LOG_UPTO (LOG_INFO));

    if (signal(SIGTTOU, SIG_IGN))
    {
        syslog(LOG_ERR, "Error en la captura de SIGTTOU");
        return -ERR;
    }

    if (signal(SIGTTIN, SIG_IGN))
    {
        syslog(LOG_ERR, "Error en la captura de SIGTTIN");
        return -ERR;
    }

    if (signal(SIGTSTP, SIG_IGN))
    {
        syslog(LOG_ERR, "Error en la captura de SIGTSTP");
        return -ERR;
    }

#ifndef __APPLE__
    if (signal(SIGPWR, finish_daemon))
    {
        syslog(LOG_ERR, "Error en la captura de SIGPWR");
        return -ERR;
    }
#endif 

    if (signal(SIGCHLD, finish_daemon))
    {
        syslog(LOG_ERR, "Error en la captura de SIGCHLD");
        return -ERR;
    }


    pid = unlink_proc();
    if (pid > 0) exit(EXIT_SUCCESS);
    if (pid < 0) exit(EXIT_FAILURE);

    syslog(LOG_INFO, "Hijo creado como lider de la sesión");

    if ((chdir("/")) < 0)
        syslog(LOG_ERR, "Error cambiando el directorio de trabajo");

    /* SIGCHLD, SIGPWR */

    syslog(LOG_INFO, "Cerrando los descriptores de fichero.");

    if (close_open_fds() < 0)
        syslog(LOG_ERR, "Error cerrando los ficheros abiertos.");

    syslog(LOG_INFO, "Daemonización completa");
    return OK;
}

int unlink_proc()
{
    pid_t pid;

    pid = fork();

    if (pid < 0)
        return -ERR;
    else if (pid > 0)
        return pid;

    if (setsid() < 0)
    {
        syslog(LOG_ERR, "Error creando un nuevo SID");
    }   

    if (signal(SIGHUP, SIG_IGN))
    {
        syslog(LOG_ERR, "Error en la captura de SIGTSTP");
        return -ERR;
    }

    return 0;
}

int close_open_fds()
{
    DIR *fddir;
    struct dirent *ent;
    int dir_fdno;
    int fd;

    fddir = opendir(FD_DIR);

    if (fddir)
    {
        dir_fdno = dirfd(fddir);
        while ((ent = readdir(fddir)) != NULL)
        {
            fd = atoi(ent->d_name);
            if (fd != dir_fdno) /* No cerramos el propio descriptor del directorio de fds. */
                close(fd);
        }

        closedir(fddir);
    }
    else
    {
        return -ERR;
    }

    return OK;
}

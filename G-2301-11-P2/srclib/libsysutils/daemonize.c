#include "daemonize.h"
#include "errors.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <execinfo.h>

#ifdef __APPLE__
#define FD_DIR "/dev/fd"
#else
#define FD_DIR "/proc/self/fd"
#endif

#define BT_DEPTH 20

char *_log_id;

static void _open_log()
{
    openlog(_log_id, 0, LOG_DAEMON);
}

static void _close_log()
{
    closelog();
}


void finish_daemon (int signal)
{
    _close_log();
    exit(EXIT_SUCCESS);
}

void critical_stop_handler(int signum)
{
    void *callstack[BT_DEPTH];
    int i, frames;
    char **strs;

    signal(signum, SIG_DFL); /* No queremos quedarnos en un bucle infinito capturando la misma señal siempre */
    slog(LOG_CRIT, "Error grave: recibida señal %d (%s). Salida inesperada.", signum, strsignal(signum));
    slog(LOG_CRIT, "Tratando de extraer backtrace (prof. %d)...", BT_DEPTH);

    frames = backtrace(callstack, BT_DEPTH);
    strs = backtrace_symbols(callstack, frames);
    
    for (i = 0; i < frames; ++i)
        slog(LOG_CRIT, strs[i]);

    free(strs);

    /* Y ahora, por si cuela, tratamos de salir ordenadamente y con tranquilidad */
    slog(LOG_CRIT, "Enviando SIGTERM para tratar de salir ordenadamente...");
    kill(getpid(), SIGTERM);
}

int daemonize(const char *log_id)
{
    int pid;
    _log_id = strdup(log_id);
    umask(0);
    _open_log();
    setlogmask (LOG_UPTO (LOG_DEBUG));

    if (signal(SIGTTOU, SIG_IGN))
    {
        slog(LOG_ERR, "Error en la captura de SIGTTOU");
        return ERR;
    }

    if (signal(SIGTTIN, SIG_IGN))
    {
        slog(LOG_ERR, "Error en la captura de SIGTTIN");
        return ERR;
    }

    if (signal(SIGTSTP, SIG_IGN))
    {
        slog(LOG_ERR, "Error en la captura de SIGTSTP");
        return ERR;
    }

    /* Nos interesa capturar estas señales para poder saber que hemos salido en el log */
    if (signal(SIGSEGV, critical_stop_handler))
        slog(LOG_ERR, "Error en la captura de SIGSEGV");

    if (signal(SIGILL, critical_stop_handler))
        slog(LOG_ERR, "Error en la captura de SIGILL");

    if (signal(SIGBUS, critical_stop_handler))
        slog(LOG_ERR, "Error en la captura de SIGBUS");

#ifndef __APPLE__
    if (signal(SIGPWR, finish_daemon))
    {
        slog(LOG_ERR, "Error en la captura de SIGPWR");
        return ERR;
    }
#endif

    if (signal(SIGCHLD, SIG_IGN))
    {
        slog(LOG_ERR, "Error en la captura de SIGCHLD");
        return ERR;
    }

#ifndef NODAEMON
    pid = unlink_proc();
    if (pid > 0)
    {
        printf("Daemon creado: PID %d\n", pid);
        exit(EXIT_SUCCESS);
    }

    if (pid < 0)
        exit(EXIT_FAILURE);
#endif

    slog(LOG_INFO, "Hijo creado como lider de la sesión");

    if ((chdir("/")) < 0)
        slog(LOG_ERR, "Error cambiando el directorio de trabajo");

    /* SIGCHLD, SIGPWR */

    slog(LOG_INFO, "Cerrando los descriptores de fichero.");

    if (close_open_fds() < 0)
        slog(LOG_ERR, "Error cerrando los ficheros abiertos.");

    slog(LOG_INFO, "Daemonización completa");
    return OK;
}

int unlink_proc()
{
    pid_t pid;

    pid = fork();

    if (pid < 0)
        return ERR;
    else if (pid > 0)
        return pid;

    if (setsid() < 0)
    {
        slog(LOG_ERR, "Error creando un nuevo SID");
    }

    if (signal(SIGHUP, SIG_IGN))
    {
        slog(LOG_ERR, "Error en la captura de SIGTSTP");
        return ERR;
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
        return ERR;
    }

    return OK;
}

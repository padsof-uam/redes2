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

#ifdef __APPLE__
#define FD_DIR "/dev/fd"
#else
#define FD_DIR "/proc/self/fd"
#endif

int daemonize()
{
    int pid;

    umask(0);

    setlogmask (LOG_UPTO (LOG_INFO));
    openlog(NULL, 0, LOG_DAEMON);

    if (signal(SIGTTOU, SIG_IGN))
    {
        syslog(LOG_ERR, "Error en la captura de SIGTTOU");
        return ERR;
    }
    if (signal(SIGTTIN, SIG_IGN))
    {
        syslog(LOG_ERR, "Error en la captura de SIGTTIN");
        return ERR;
    }
    if (signal(SIGTSTP, SIG_IGN))
    {
        syslog(LOG_ERR, "Error en la captura de SIGTSTP");
        return ERR;
    }

    pid = fork();
    if (pid > 0) exit(EXIT_SUCCESS);
    if (pid < 0) exit(EXIT_FAILURE);

    if (setsid() < 0)
    {
        syslog(LOG_ERR, "Error creando un nuevo SID");
    }

    syslog(LOG_INFO, "Hijo creado como lider de la sesión");

    if ((chdir("/")) < 0)
    {
        syslog(LOG_ERR, "Error cambiando el directorio de trabajo");
    }

    if (signal(SIGHUP, SIG_IGN))
    {
        syslog(LOG_ERR, "Error en la captura de SIGTSTP");
        exit(EXIT_FAILURE);
    }


    syslog(LOG_INFO, "Cerrando los descriptores standard");
    close_open_fds();

    return OK;
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
        syslog(LOG_ERR, "Error al abrir la lista de descriptores en uso.");
        return ERR;
    }

    return OK;
}

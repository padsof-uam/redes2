#include "sysutils.h"
#include "errors.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

int write_pid(const char* file)
{
    FILE *pid_file;

    pid_file = fopen(file, "w");

    if (!pid_file)
        return ERR;

    fprintf(pid_file, "%d\n", getpid());
    fclose(pid_file);
    return OK;
}

int read_pid_and_kill(const char* file)
{
    FILE *pid_file;
    pid_t pid;
    int read;
    int max_ms_wait = MAX_KILL_WAIT_MS;
    int ms_wait_interval = 200;
    int ms_waited = 0;
    struct timespec ts;

    ts.tv_nsec = ms_wait_interval * 1000;
    ts.tv_sec = 0;

    pid_file = fopen(file, "r");

    if (!pid_file)
        return -1;

    read = fscanf(pid_file, "%d", &pid);
    fclose(pid_file);

    if (read < 1)
        return -1;

    if (kill(pid, SIGTERM) == -1)
    {
        perror("kill");
        return -1;
    }

    while (ms_waited < max_ms_wait)
    {
        if (kill(pid, 0) == -1) /* Kill con señal 0 devuelve -1 si el proceso ya ha salido */
            return 0;

        nanosleep(&ts, NULL);
        ms_waited += ms_wait_interval;
    }

    kill(pid, SIGKILL);

    return 1;
}

int try_getlock(const char* file)
{
    int pid_file = open(file, O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);

    if (rc)
    {
        if (EWOULDBLOCK == errno)
            return 0;
        else
            return -1;
    }

    return 1;
}

#include "sysutils.h"
#include "errors.h"
#include "termcolor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <string.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

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

rlim_t get_soft_limit(int resource)
{
    struct rlimit lim;
    if(getrlimit(resource, &lim) == -1)
        return -1;
    else
        return lim.rlim_cur;
}


#define BT_DEPTH 100

static void _critical_stop_handler(int signum, siginfo_t *info, void *ucontext)
{
    void *callstack[BT_DEPTH];
    int i, frames = 0;
    char **strs;
    void *caller_address = NULL;
    ucontext_t *uc = ucontext;

    signal(signum, SIG_DFL); /* Avoid infinite loops */
    signal(SIGABRT, SIG_DFL); 
    fflush(stdout);
    fprintf(stderr, TRESET "\n\nCritical error: received signal %s. Unexpected exit.\n", strsignal(signum));
    fprintf(stderr, "Trying to get the backtrace (max. depth %d)...\n", BT_DEPTH);

    frames = backtrace(callstack, BT_DEPTH);

    if(uc != NULL)
    {
#ifdef __APPLE__
    caller_address = (void*) uc->uc_mcontext->__ss.__rip;
#endif
    }

    if(frames > 3 && caller_address != NULL)
        callstack[2] = caller_address;

    strs = backtrace_symbols(callstack, frames);

    for (i = 0; i < frames; ++i)
        fprintf(stderr, "%s\n", strs[i]);

    free(strs);

    abort();
}

int install_stop_handlers()
{
    int retval = OK;

    if (signal(SIGSEGV, (sig_t)_critical_stop_handler) == SIG_ERR)
    {
        perror("signal: SIGSEGV");
        retval = ERR;
    }

    if (signal(SIGILL, (sig_t)_critical_stop_handler) == SIG_ERR)
    {
        perror("signal: SIGILL");
        retval = ERR;
    }

    if (signal(SIGBUS, (sig_t)_critical_stop_handler) == SIG_ERR)
    {
        perror("signal: SIGBUS");
        retval = ERR;
    }

    if (signal(SIGABRT, (sig_t)_critical_stop_handler) == SIG_ERR)
    {
        perror("signal: SIGABRT");
        retval = ERR;
    }

    if (signal(SIGFPE, (sig_t)_critical_stop_handler) == SIG_ERR)
    {
        perror("signal: SIGFPE");
        retval = ERR;
    }

    return retval;
}

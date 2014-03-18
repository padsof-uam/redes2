#include "termcolor.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

#define BT_DEPTH 100

static void _critical_stop_handler(int signum)
{
    void *callstack[BT_DEPTH];
    int i, frames;
    char **strs;

    signal(signum, SIG_DFL); /* Avoid infinite loops */
    fflush(stdout);
    fprintf(stderr, TRESET "\n\nCritical error: received signal %d (%s). Unexpected exit.\n", signum, strsignal(signum));
    fprintf(stderr, "Trying to get the backtrace (max. depth %d)...\n", BT_DEPTH);

    frames = backtrace(callstack, BT_DEPTH);
    strs = backtrace_symbols(callstack, frames);
    
    for (i = 0; i < frames; ++i)
        fprintf(stderr, "%s\n", strs[i]);

    free(strs);

    abort();
}

int include_test(const char *testname, int argc, const char **argv)
{
    int is_including = 0;
    int i;

    if (argc == 1)
        return 1; /* No hay especificaciones */

    if (!strncasecmp("include", argv[1], strlen(testname)))
        is_including = 1;
    else if (!strncasecmp("exclude", argv[1], strlen(testname)))
        is_including = 0;
    else
        fprintf(stderr, "include/exclude not recognized. Assuming 'exclude'\n");

    for(i = 2; i < argc; i++)
    	if(!strncasecmp(testname, argv[i], strlen(testname)))
    		return is_including;

    return !is_including;
}

int main(int argc, const char **argv)
{
    time_t t;
    int success = 0, error = 0, run = 0;
    time(&t);

     if (signal(SIGSEGV, _critical_stop_handler))
        perror("signal: SIGSEGV");

    if (signal(SIGILL, _critical_stop_handler))
        perror("signal: SIGILL");

    if (signal(SIGBUS, _critical_stop_handler))
        perror("signal: SIGBUS");

    printf("Begin test run %s\n", ctime(&t));
    /* BEGIN TEST REGION */
    /* END TEST REGION */
    time(&t);
    printf("End test run %s", ctime(&t));
    printf("Run %d." TGREEN " %d success, "TRED "%d errors.\n" TRESET, run, success, error);

    return 0;
}

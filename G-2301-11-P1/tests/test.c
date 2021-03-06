#include "test_irc_core.h"
#include "test_strings.h"
#include "test_irc_funs.h"
#include "test_irc_processor.h"
#include "test_dictionary.h"
#include "test_list.h"
#include "test_poller.h"
#include "test_listener.h"
#include "test_listener.h"
#include "test_messager.h"
#include "test_messager.h"
#include "test_commparser.h"
#include "test_daemonize.h"
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

    signal(signum, SIG_DFL); /* No queremos quedarnos en un bucle infinito capturando la misma señal siempre */
    fflush(stdout);
    fprintf(stderr, TRESET "\n\nError grave: recibida señal %d (%s). Salida inesperada.\n", signum, strsignal(signum));
    fprintf(stderr, "Tratando de extraer backtrace (prof. %d)...\n", BT_DEPTH);

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
	if(include_test("irc_core", argc, argv))
		run += test_irc_core_suite(&error, &success);
	if(include_test("strings", argc, argv))
		run += test_strings_suite(&error, &success);
	if(include_test("irc_funs", argc, argv))
		run += test_irc_funs_suite(&error, &success);
	if(include_test("irc_processor", argc, argv))
		run += test_irc_processor_suite(&error, &success);
	if(include_test("dictionary", argc, argv))
		run += test_dictionary_suite(&error, &success);
	if(include_test("list", argc, argv))
		run += test_list_suite(&error, &success);
	if(include_test("poller", argc, argv))
		run += test_poller_suite(&error, &success);
	if(include_test("listener", argc, argv))
		run += test_listener_suite(&error, &success);
	if(include_test("messager", argc, argv))
		run += test_messager_suite(&error, &success);
    if(include_test("commparser", argc, argv))
	    run += test_commparser_suite(&error, &success);
    if(include_test("daemonize", argc, argv))
    	run += test_daemonize_suite(&error, &success);

    /* END TEST REGION */
    time(&t);
    printf("End test run %s", ctime(&t));
    printf("Run %d." TGREEN " %d success, "TRED "%d errors.\n" TRESET, run, success, error);

    return 0;
}

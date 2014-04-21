#include "test_voicecall.h"
#include "test_server_history.h"
#include "test_lfringbuf.h"
#include "test_irc_core.h"
#include "test_strings.h"
#include "test_irc_funs.h"
#include "test_irc_processor.h"
#include "test_dictionary.h"
#include "test_list.h"
#include "test_poller.h"
#include "test_sockutils.h"
#include "test_messager.h"
#include "test_commparser.h"
#include "test_daemonize.h"
#include "termcolor.h"
#include "sysutils.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

#define BT_DEPTH 100

int include_test(const char *testname, int argc, const char **argv)
{
    int is_including = 0;
    int i;

    if (argc == 0)
        return 1; /* No hay especificaciones */    

    if (!strncasecmp("include", argv[0], strlen(testname)))
        is_including = 1;
    else if (!strncasecmp("exclude", argv[0], strlen(testname)))
        is_including = 0;
    else
        fprintf(stderr, "include/exclude not recognized. Assuming 'exclude'\n");

    for(i = 1; i < argc; i++)
        if(!strncasecmp(testname, argv[i], strlen(testname)))
            return is_including;

    return !is_including;
}

int main(int argc, const char **argv)
{
    time_t t;
    int success = 0, error = 0, run = 0;
    time(&t);   
    const char **spec_start = argv + 1;

    argc--;

    if(argc >= 1 && !strncasecmp("-v", argv[1], 2))
    {
        slog_set_level(LOG_DEBUG);
        spec_start++;
        argc--;
    }
    
    slog_set_output(stderr);

    install_stop_handlers();

    printf("Begin test run %s\n", ctime(&t));
    /* BEGIN TEST REGION */
	if(include_test("voicecall", argc, spec_start))
		run += test_voicecall_suite(&error, &success);
	if(include_test("server_history", argc, spec_start))
		run += test_server_history_suite(&error, &success);
	if(include_test("lfringbuf", argc, spec_start))
		run += test_lfringbuf_suite(&error, &success);
	if(include_test("irc_core", argc, spec_start))
		run += test_irc_core_suite(&error, &success);
	if(include_test("strings", argc, spec_start))
		run += test_strings_suite(&error, &success);
	if(include_test("irc_funs", argc, spec_start))
		run += test_irc_funs_suite(&error, &success);
	if(include_test("irc_processor", argc, spec_start))
		run += test_irc_processor_suite(&error, &success);
	if(include_test("dictionary", argc, spec_start))
		run += test_dictionary_suite(&error, &success);
	if(include_test("list", argc, spec_start))
		run += test_list_suite(&error, &success);
	if(include_test("poller", argc, spec_start))
		run += test_poller_suite(&error, &success);
	if(include_test("listener", argc, spec_start))
		run += test_sockutils_suite(&error, &success);
	if(include_test("messager", argc, spec_start))
		run += test_messager_suite(&error, &success);
    if(include_test("commparser", argc, spec_start))
	    run += test_commparser_suite(&error, &success);
    if(include_test("daemonize", argc, spec_start))
    	run += test_daemonize_suite(&error, &success);

    /* END TEST REGION */
    time(&t);
    printf("End test run %s", ctime(&t));
    printf("Run %d." TGREEN " %d success, "TRED "%d errors.\n" TRESET, run, success, error);

    return 0;
}

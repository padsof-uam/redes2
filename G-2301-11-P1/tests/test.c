#include "test_listener.h"
#include "test_listener.h"
#include "test_messager.h"
#include "test_messager.h"
#include "test_commparser.h"
#include "test_daemonize.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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

    printf("Begin test run %s\n", ctime(&t));
    /* BEGIN TEST REGION */
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
    printf("End test run %s.\n", ctime(&t));
    printf("Run %d. %d success, %d errors.\n", run, success, error);

    return 0;
}

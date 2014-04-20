#include "test_server_history.h"
#include "testmacros.h"
#include "server_history.h"
#include "errors.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define TMP_FSFILE "/tmp/redirc-favserv"

/* BEGIN TESTS */
int t_serv_save_connection__new_connection__added()
{
    FILE *f = fopen(TMP_FSFILE, "w");
    struct serv_info info = { "newhost", "125", 0 };
    int retval;
    char line1[200], line2[200];

    mu_assert("fopen", f);
    fprintf(f, "localhost 6667 4\n");
    fclose(f);

    retval = serv_save_connection_to(TMP_FSFILE, &info);
    mu_assert_eq(retval, OK, "add failed");

    f = fopen(TMP_FSFILE, "r");
    mu_assert("fopen r", f);

    if (fgets(line1, 100, f) == NULL) {
        mu_cleanup_sysfail(cleanup, "fgets 1");
    }
    if (fgets(line2, 100, f) == NULL) {
        mu_cleanup_sysfail(cleanup, "fgets 2");
    }

    fclose(f);
    f = NULL;

    mu_assert_streq(line1, "localhost 6667 4\n", "line 1");
    mu_assert_streq(line2, "newhost 125 1\n", "line 2");

    retval = MU_PASSED;
cleanup:
    if (f)
        fclose(f);

    mu_cleanup_end;
}
int t_serv_save_connection__existing_connection__times_used_increased()
{
    FILE *f = fopen(TMP_FSFILE, "w");
    struct serv_info info = { "localhost", "6667", 0 };
    int retval;
    char line1[200];

    mu_assert("fopen", f);
    fprintf(f, "localhost 6667 4\n");
    fclose(f);

    retval = serv_save_connection_to(TMP_FSFILE, &info);
    mu_assert_eq(retval, OK, "add failed");

    f = fopen(TMP_FSFILE, "r");
    mu_assert("fopen r", f);

    if (fgets(line1, 100, f) == NULL) {
        mu_cleanup_sysfail(cleanup, "fgets 1");
    }

    fclose(f);
    f = NULL;

    mu_assert_streq(line1, "localhost 6667 5\n", "line 1");

    retval = MU_PASSED;
cleanup:
    if (f)
        fclose(f);

    mu_cleanup_end;
}
int t_servhistory_getlist__multiple_items__retrieved()
{
    FILE *f = fopen(TMP_FSFILE, "w");
    int retval;
    struct serv_info list[5];

    mu_assert("fopen", f);
    fprintf(f, "localhost 6667 4\n");
    fprintf(f, "otherhost 612 3\n");
    fclose(f);

    f = fopen(TMP_FSFILE, "r");
    mu_assert("fopen", f);

    retval = serv_getlist(f, list, 5);
    fclose(f);

    mu_assert_eq(retval, 2, "list size");

    mu_assert_streq(list[0].servname, "localhost", "host 1");
    mu_assert_streq(list[1].servname, "otherhost", "host 2");
    mu_assert_streq(list[0].port, "6667", "port 1");
    mu_assert_streq(list[1].port, "612", "port 1"); 
    mu_assert_eq(list[0].times_used, 4, "times_used 1");
    mu_assert_eq(list[1].times_used, 3, "times_used 1");


    mu_end;
}
int t_parse_servinfo__line__parsed()
{
	char line[] = "localhost 6667 1";
	struct serv_info parsed;
	int retval;

	retval = parse_servinfo(line, &parsed);

	mu_assert_eq(retval, OK, "parse retval");
	mu_assert_streq(parsed.port, "6667", "port");
	mu_assert_streq(parsed.servname, "localhost", "servname");
	mu_assert_eq(parsed.times_used, 1, "times_used");

    mu_end;
}
/* END TESTS */

int test_server_history_suite(int *errors, int *success)
{
    int tests_run = 0;
    int tests_passed = 0;

    printf("Begin test_server_history suite.\n");
    /* BEGIN TEST EXEC */
    mu_run_test(t_serv_save_connection__new_connection__added);
    mu_run_test(t_serv_save_connection__existing_connection__times_used_increased);
    mu_run_test(t_servhistory_getlist__multiple_items__retrieved);
    mu_run_test(t_parse_servinfo__line__parsed);
    /* END TEST EXEC */
    if (tests_passed == tests_run)
        printf("End test_server_history suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
    else
        printf("End test_server_history suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


    *errors += (tests_run - tests_passed);
    *success += tests_passed;
    return tests_run;
}

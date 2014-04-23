#include "test_ssltrans.h"
#include "testmacros.h"
#include "ssltrans.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CA_PEM "cert/root.pemr"
#define KEY_PEM "cert/server.pem"

/* BEGIN TESTS */
int t_init_all_ssl__verify_peer__can_init()
{
    int retval;

    retval = init_all_ssl(CA_PEM, KEY_PEM, 1);

    mu_assert_eq(retval, OK, "didn't init");
    cleanup_all_ssl();
    mu_end;
}
int t_init_all_ssl__no_verify_peer__can_init()
{
    int retval;

    retval = init_all_ssl(CA_PEM, KEY_PEM, 0);

    mu_assert_eq(retval, OK, "didn't init");
    cleanup_all_ssl();
    mu_end;
}
/* END TESTS */

int test_ssltrans_suite(int *errors, int *success)
{
    int tests_run = 0;
    int tests_passed = 0;

    printf("Begin test_ssltrans suite.\n");
    /* BEGIN TEST EXEC */
    mu_run_test(t_init_all_ssl__verify_peer__can_init);
    mu_run_test(t_init_all_ssl__no_verify_peer__can_init);
    /* END TEST EXEC */
    if (tests_passed == tests_run)
        printf("End test_ssltrans suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
    else
        printf("End test_ssltrans suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


    *errors += (tests_run - tests_passed);
    *success += tests_passed;
    return tests_run;
}

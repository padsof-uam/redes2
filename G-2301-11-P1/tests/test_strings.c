#include "test_strings.h"
#include "testmacros.h"
#include "termcolor.h"
#include "strings.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* BEGIN TESTS */
int t_strip__tabs__removed()
{
    char *str = strdup("\tasd\t");
    char* orig = str;
    char expected[] = "asd";

    strip(&str);

    mu_assert_streq(str, expected, "Strings are not equal");
    free(orig);
    mu_end;
}
int t_strip__nullstr__nofail()
{
    char *str = NULL;
    strip(&str); /* probamos que no hay seg. fault */

    mu_end;
}
int t_strip__nullptr__nofail()
{
    strip(NULL); /* Probamos que no haya seg. fault */
    mu_end;
}
int t_strip__spaces_both__stripped()
{
    char *str = strdup("  asd  ");
    char* orig = str;
    char expected[] = "asd";

    strip(&str);

    mu_assert_streq(str, expected, "Strings are not equal");
    free(orig);
    mu_end;
}
int t_strip__spaces_end__stripped()
{
    char *str = strdup("asd  ");
    char* orig = str;
    char expected[] = "asd";

    strip(&str);

    mu_assert_streq(str, expected, "Strings are not equal");
    free(orig);
    mu_end;
}
int t_strip__spaces_start__stripped()
{
    char *str = strdup("  asd");
    char* orig = str;
    char expected[] = "asd";

    strip(&str);

    mu_assert_streq(str, expected, "Strings are not equal");
    free(orig);
    mu_end;
}

/* END TESTS */

int test_strings_suite(int *errors, int *success)
{
    int tests_run = 0;
    int tests_passed = 0;

    printf("Begin test_strings suite.\n");
    /* BEGIN TEST EXEC */
    mu_run_test(t_strip__tabs__removed);
    mu_run_test(t_strip__nullstr__nofail);
    mu_run_test(t_strip__nullptr__nofail);
    mu_run_test(t_strip__spaces_both__stripped);
    mu_run_test(t_strip__spaces_end__stripped);
    mu_run_test(t_strip__spaces_start__stripped);

    /* END TEST EXEC */
    if (tests_passed == tests_run)
        printf("End test_strings suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
    else
        printf("End test_strings suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


    *errors += (tests_run - tests_passed);
    *success += tests_passed;
    return tests_run;
}

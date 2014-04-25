#include "G-2301-11-P3-test_strings.h"
#include "G-2301-11-P3-testmacros.h"
#include "G-2301-11-P3-termcolor.h"
#include "G-2301-11-P3-strings.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* BEGIN TESTS */
int t_str_arrsep__null_str__returns_zero() {
    char* parts[2];
    int partnum = str_arrsep(NULL, "," , parts, 2);

    mu_assert_eq(partnum, 0, "partnum is not zero");
	mu_end;
}
int t_str_arrsep__no_separators__one_part() {
    char* parts[3];
    char str[] = "nocommashere";
    int partnum;

    partnum = str_arrsep(str, ",", parts, 3);

	mu_assert_eq(partnum, 1, "bad count");

    mu_end;
}
int t_str_arrsep__nullstring__returns_zero() {

	char* parts[3];
    char* str = NULL;
    int partnum;

    partnum = str_arrsep(str, ",", parts, 3);

    mu_assert_eq(partnum, 0, "count is not 0");
	mu_end;
}
int t_str_arrsep__fill_param_array_minusone__no_overflow() {

	char* parts[3];
    char str[] = "a,param";
    int partnum;
    parts[2] = NULL;

    partnum = str_arrsep(str, ",", parts, 3);

    mu_assert_eq(partnum, 2, "bad number of parts");
    mu_assert_streq(parts[0], "a", "first element is wrong");
    mu_assert_streq(parts[1], "param", "second element is wrong");
    mu_assert("third element was modified", parts[2] == NULL);

	mu_end;
}
int t_str_arrsep__fill_param_array__no_overflow() {

	char* parts[3];
    char str[] = "one,two,three";
    int partnum;

    partnum = str_arrsep(str, ",", parts, 3);
    mu_assert_eq(partnum, 3, "bad number of parts");
    mu_assert_streq(parts[0], "one", "first element is wrong");
    mu_assert_streq(parts[1], "two", "second element is wrong");
    mu_assert_streq(parts[2], "three", "third element is wrong");

	mu_end;
}
int t_str_arrsep__normal_separation__correct() {

	char* parts[5];
    char str[] = "this is, a param";
    int partnum;

    partnum = str_arrsep(str, ",", parts, 5);
    mu_assert_eq(partnum, 2, "bad number of parts");
    mu_assert_streq(parts[0], "this is", "first element is wrong");
    mu_assert_streq(parts[1], " a param", "second element is wrong");

	mu_end;
}
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
	mu_run_test(t_str_arrsep__null_str__returns_zero);
	mu_run_test(t_str_arrsep__no_separators__one_part);
	mu_run_test(t_str_arrsep__nullstring__returns_zero);
	mu_run_test(t_str_arrsep__fill_param_array_minusone__no_overflow);
	mu_run_test(t_str_arrsep__fill_param_array__no_overflow);
	mu_run_test(t_str_arrsep__normal_separation__correct);
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

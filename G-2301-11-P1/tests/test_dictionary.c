#include "test_dictionary.h"
#include "testmacros.h"
#include "dictionary.h"
#include "list_helpers.h"
#include "errors.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* BEGIN TESTS */
int t_dic_lookup__existing__returns_item()
{
    dictionary *dic = dic_new_withint(str_duplicator, free);
    int key = 3;
    char str[] = "test1";
    char *result;

    dic_add(dic, &key, str);

    result = dic_lookup(dic, &key);
    mu_assert("value is null", result);
    mu_assert("value is not the same", strcmp(result, str) == 0);

    dic_destroy(dic, NULL);
    mu_end;
}
int t_dic_lookup__non_existing__returns_null()
{
    dictionary *dic = dic_new_withint(str_duplicator, free);
    int key = 3;
    char str[] = "test1";
    char *result;

    dic_add(dic, &key, str);
    key = 4;
    result = dic_lookup(dic, &key);
    mu_assert("value is not null", result == NULL);

    dic_destroy(dic, NULL);
    mu_end;
}
int t_dic_remove__existing__removed()
{
    dictionary *dic = dic_new_withint(str_duplicator, free);
    int key = 3;
    char str[] = "test1";
    int retval, count;

    dic_add(dic, &key, str);

    retval = dic_remove(dic, &key);
    count = dic_count(dic);
    mu_assert_eq(retval, OK, "retval is not ok");
    mu_assert_eq(count, 0, "number of elements is not correct");

    dic_destroy(dic, NULL);
    mu_end;
}
int t_dic_remove__non_existing__not_removed()
{
    dictionary *dic = dic_new_withint(str_duplicator, free);
    int key = 3;
    char str[] = "test1";
    int retval, count;

    dic_add(dic, &key, str);
    key = 4;
    retval = dic_remove(dic, &key);
    count = dic_count(dic);

    mu_assert_eq(retval, ERR_NOTFOUND, "retval is not ok");
    mu_assert_eq(count, 1, "number of elements is not correct");


    dic_destroy(dic, NULL);
    mu_end;
}

int t_dic_add__repeated_item__not_added()
{
	dictionary *dic = dic_new_withint(str_duplicator, free);
    int key = 3;
    char str[] = "test1";
    int retval;

    dic_add(dic, &key, str);
    retval = dic_add(dic, &key, str);

    mu_assert_eq(retval, OK, "dic_add");
    mu_assert_eq(dic_count(dic), 1, "count");

    dic_destroy(dic, NULL);
    mu_end;
}
int t_dic_add__new_item__added()
{
	dictionary *dic = dic_new_withint(str_duplicator, free);
    int key = 3;
    char str[] = "test1";
    int retval;

    retval = dic_add(dic, &key, str);

    mu_assert_eq(retval, OK, "dic_add");
    mu_assert_eq(dic_count(dic), 1, "count");

    dic_destroy(dic, NULL);
    mu_end;
}

/* END TESTS */

int test_dictionary_suite(int *errors, int *success)
{
    int tests_run = 0;
    int tests_passed = 0;

    printf("Begin test_dictionary suite.\n");
    /* BEGIN TEST EXEC */
    mu_run_test(t_dic_lookup__existing__returns_item);
    mu_run_test(t_dic_lookup__non_existing__returns_null);
    mu_run_test(t_dic_remove__existing__removed);
    mu_run_test(t_dic_remove__non_existing__not_removed);
    mu_run_test(t_dic_add__repeated_item__not_added);
    mu_run_test(t_dic_add__new_item__added);

    /* END TEST EXEC */
    if (tests_passed == tests_run)
        printf("End test_dictionary suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
    else
        printf("End test_dictionary suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


    *errors += (tests_run - tests_passed);
    *success += tests_passed;
    return tests_run;
}

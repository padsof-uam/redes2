#include "G-2301-11-P3-test_irc_processor.h"
#include "G-2301-11-P3-irc_processor.h"
#include "G-2301-11-P3-testmacros.h"
#include <stdio.h>
#include <string.h>

typedef enum
{
    flg1 = 1, flg2 = 2, flg3 = 4
} flg;

struct ircflag fldic[] =
{
    { 'a', flg1 },
    { 'b', flg2 },
    { 'c', flg3 },
    IRCFLAGS_END
};

/* BEGIN TESTS */
int t_irc_get_prefix__full_prefix__ignores_host_part() {
	const char* msg = ":nuts!~nuts@60.247.46.112 one two three";
    char prefix[10];
    int prefix_len = 10;
    int retval;

    retval = irc_get_prefix(msg, prefix, prefix_len);

    mu_assert_eq(retval, OK, "retval is not ok");
    mu_assert_streq(prefix, "nuts", "prefix was not retrieved correctly");

    mu_end;
}
int t_irc_get_prefix__small_buffer__not_overflowed() {
    const char* msg = ":123456 one two three";
    char prefix[10];
    int prefix_len = 3;
    int i;
    int retval;

    for(i = 0; i < 10; i++)
        prefix[i] = 1; /* Marcamos la cadena para comprobar que no se sobreescribe */

    retval = irc_get_prefix(msg, prefix, prefix_len);

    mu_assert_eq(retval, OK, "retval is not ok");
    mu_assert_streq(prefix, "12", "prefix was not retrieved correctly");

    for(i = 3; i< 10; i++)
        mu_assert_eq(prefix[i], 1, "buffer overrun");

    mu_end;
}
int t_irc_get_prefix__multiple_spaces_in_msg__returns_prefix() {
    const char* msg = ":aprefix one two three";
    char prefix[10];
    int prefix_len = 10;
    int retval;

    retval = irc_get_prefix(msg, prefix, prefix_len);

    mu_assert_eq(retval, OK, "retval is not ok");
    mu_assert_streq(prefix, "aprefix", "prefix was not retrieved correctly");

    mu_end;
}
int t_irc_get_prefix__one_space__returns_prefix() {
    const char* msg = ":aprefix one";
    char prefix[10];
    int prefix_len = 10;
    int retval;

    retval = irc_get_prefix(msg, prefix, prefix_len);

    mu_assert_eq(retval, OK, "retval is not ok");
    mu_assert_streq(prefix, "aprefix", "prefix was not retrieved correctly");

    mu_end;
}
int t_irc_get_prefix__no_colon__returns_err() {
    const char* msg = "aprefix one";
    char prefix[10];
    int prefix_len = 10;
    int retval;

    retval = irc_get_prefix(msg, prefix, prefix_len);

    mu_assert_eq(retval, ERR_PARSE, "retval is not ERR_PARSE");

    mu_end;
}
int t_irc_get_prefix__no_spaces__returns_err() {
    const char* msg = ":aprefix";
    char prefix[10];
    int prefix_len = 10;
    int retval;

    retval = irc_get_prefix(msg, prefix, prefix_len);

    mu_assert_eq(retval, ERR_PARSE, "retval is not ERR_PARSE");

    mu_end;
}
int t_irc_get_prefix__empty_string__returns_err() {
    const char* msg = "";
    char prefix[10];
    int prefix_len = 10;
    int retval;

    retval = irc_get_prefix(msg, prefix, prefix_len);

    mu_assert_eq(retval, ERR_PARSE, "retval is not ERR_PARSE");

    mu_end;
}
int t_irc_strflag__parse_flag__str_correct() {
	int flags = flg1 | flg2;
	char str[10];

	irc_strflag(flags, str, 10, fldic);

	mu_assert_streq(str, "+ab", "");

	mu_end;
}
int t_irc_flagparse__multiple_flags_minus__flags_removed()
{
    int actual = flg1 | flg2 | flg3;
    int expected = flg3;
    char str[] = "-ab";

    irc_flagparse(str, &actual, fldic);

    mu_assert_eq(actual, expected, "flags don't match");

    mu_end;
}
int t_irc_flagparse__multiple_flags_add__flags_added()
{
    int actual = 0;
    int expected = flg1 | flg2;
    char str[] = "+ab";

    irc_flagparse(str, &actual, fldic);

    mu_assert_eq(actual, expected, "flags don't match");

    mu_end;
}
int t_irc_flagparse__one_flag_minus__flag_removed()
{
    int actual = 0;
    int expected = flg1;
    char str[] = "+a";

    irc_flagparse(str, &actual, fldic);

    mu_assert_eq(actual, expected, "flags don't match");

    mu_end;
}
int t_irc_flagparse__one_flag_add__flag_added()
{
    int actual = flg1;
    int expected = 0;
    char str[] = "-a";

    irc_flagparse(str, &actual, fldic);

    mu_assert_eq(actual, expected, "flags don't match");

    mu_end;
}
int t_irc_parse_paramlist__colon_at_start__ignored()
{
    int pnum = 4;
    char *params[4];
    char str[] = ":anice_prefix PRIVMSG a param not";
    int count;

    count = irc_parse_paramlist(str, params, pnum);

    mu_assert_eq(count, 3, "Count incorrect");
    mu_assert_streq(params[0], "a", "First param incorrect");
    mu_assert_streq(params[1], "param", "First param incorrect");
    mu_assert_streq(params[2], "not", "Third param has been modified");

    mu_end;
}
int t_irc_parse_paramlist__size_not_enough__params_limited()
{
    int pnum = 2;
    char *params[3];
    char str[] = "PRIVMSG a param not enough";
    int count;

    params[2] = NULL;

    count = irc_parse_paramlist(str, params, pnum);

    mu_assert_eq(count, pnum, "Count incorrect");
    mu_assert_streq(params[0], "a", "First param incorrect");
    mu_assert_streq(params[1], "param", "First param incorrect");
    mu_assert("Third param has been modified", !params[2]);

    mu_end;
}
int t_irc_parse_paramlist__paramnum_eq_paramsize__fillsall()
{
    int pnum = 3;
    char *params[3];
    char str[] = "PRIVMSG a param not";
    int count;

    count = irc_parse_paramlist(str, params, pnum);

    mu_assert_eq(count, pnum, "Count incorrect");
    mu_assert_streq(params[0], "a", "param incorrect");
    mu_assert_streq(params[1], "param", "param incorrect");
    mu_assert_streq(params[2], "not", "param incorrect");

    mu_end;
}
int t_irc_parse_paramlist__only_colon_param__params_filled()
{
    int pnum = 3;
    char *params[3];
    char str[] = ":anice_prefix PRIVMSG :a param not";
    int count;

    count = irc_parse_paramlist(str, params, pnum);

    mu_assert_eq(count, 1, "Count incorrect");
    mu_assert_streq(params[0], "a param not", "First param incorrect");

    mu_end;
}
int t_irc_parse_paramlist__params_and_colon__params_filled()
{
    int pnum = 5;
    char *params[5];
    char str[] = "PRIVMSG two,three b :a param not";
    int count;

    count = irc_parse_paramlist(str, params, pnum);

    mu_assert_eq(count, 3, "Count incorrect");
    mu_assert_streq(params[0], "two,three", "param incorrect");
    mu_assert_streq(params[1], "b", "param incorrect");
    mu_assert_streq(params[2], "a param not", "param incorrect");
    mu_end;
}
int t_irc_parse_paramlist__some_params_no_colon__params_filled()
{
    int pnum = 5;
    char *params[5];
    char str[] = "PRIVMSG two,three b";
    int count;

    count = irc_parse_paramlist(str, params, pnum);

    mu_assert_eq(count, 2, "Count incorrect");
    mu_assert_streq(params[0], "two,three", "param incorrect");
    mu_assert_streq(params[1], "b", "param incorrect");
    mu_end;
}
int t_irc_parse_paramlist__noparams__returnszero()
{
    int pnum = 5;
    char *params[5];
    char str[] = "PRIVMSG";
    int count;

    count = irc_parse_paramlist(str, params, pnum);

    mu_assert_eq(count, 0, "Count incorrect");
    mu_end;
}
int t_irc_msgsep__length_limited__no_len_surpassing()
{
    char str[] = "123456\r\n123";
    char *next = irc_msgsep(str, 3);

    mu_assert("the crlf was found.", !next);

    mu_end;
}
int t_irc_msgsep__null_str__return_null()
{
    char *next = irc_msgsep(NULL, 3);

    mu_assert("retval was not null", !next);

    mu_end;
}
int t_irc_msgsep__with_crlf__separation_correct()
{
    char str[] = "123456\r\n123";
    char *expected = str + 8;
    char *next = irc_msgsep(str, 40);

    mu_assert("the crlf was not found.", next == expected);
    mu_assert("separated message is not the same", !strcmp(str, "123456"));

    mu_end;
}
int t_irc_msgsep__no_crlf__notmodified()
{
    char str[] = "123456123";
    char *original = strdup(str);
    char *next = irc_msgsep(str, 40);

    mu_assert("the crlf was found.", next == NULL);
    mu_assert("separated message is not the same", !strcmp(str, original));

    free(original);
    mu_end;
}

/* END TESTS */

int test_irc_processor_suite(int *errors, int *success)
{
    int tests_run = 0;
    int tests_passed = 0;

    printf("Begin test_irc_processor suite.\n");
    /* BEGIN TEST EXEC */
	mu_run_test(t_irc_get_prefix__full_prefix__ignores_host_part);
	mu_run_test(t_irc_get_prefix__small_buffer__not_overflowed);
	mu_run_test(t_irc_get_prefix__multiple_spaces_in_msg__returns_prefix);
	mu_run_test(t_irc_get_prefix__one_space__returns_prefix);
	mu_run_test(t_irc_get_prefix__no_colon__returns_err);
	mu_run_test(t_irc_get_prefix__no_spaces__returns_err);
	mu_run_test(t_irc_get_prefix__empty_string__returns_err);
	mu_run_test(t_irc_strflag__parse_flag__str_correct);
    mu_run_test(t_irc_flagparse__multiple_flags_minus__flags_removed);
    mu_run_test(t_irc_flagparse__multiple_flags_add__flags_added);
    mu_run_test(t_irc_flagparse__one_flag_minus__flag_removed);
    mu_run_test(t_irc_flagparse__one_flag_add__flag_added);
    mu_run_test(t_irc_parse_paramlist__colon_at_start__ignored);
    mu_run_test(t_irc_parse_paramlist__size_not_enough__params_limited);
    mu_run_test(t_irc_parse_paramlist__paramnum_eq_paramsize__fillsall);
    mu_run_test(t_irc_parse_paramlist__only_colon_param__params_filled);
    mu_run_test(t_irc_parse_paramlist__params_and_colon__params_filled);
    mu_run_test(t_irc_parse_paramlist__some_params_no_colon__params_filled);
    mu_run_test(t_irc_parse_paramlist__noparams__returnszero);
    mu_run_test(t_irc_msgsep__length_limited__no_len_surpassing);
    mu_run_test(t_irc_msgsep__null_str__return_null);
    mu_run_test(t_irc_msgsep__with_crlf__separation_correct);
    mu_run_test(t_irc_msgsep__no_crlf__notmodified);

    /* END TEST EXEC */
    if (tests_passed == tests_run)
        printf("End test_irc_processor suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
    else
        printf("End test_irc_processor suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


    *errors += (tests_run - tests_passed);
    *success += tests_passed;
    return tests_run;
}

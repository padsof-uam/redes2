#include "test_irc_core.h"
#include "testmacros.h"
#include "termcolor.h"
#include "irc_core.h"
#include "irc_testhelper.h"
#include <stdio.h>



/* BEGIN TESTS */
int t_irc_can_talk_in_channel__moderated_voice__returns_true() {
    struct irc_globdata * irc = irc_init();
    struct ircuser * user = _irc_register_withnick(irc, 1, "Paco");
    struct ircchan * chan = _irc_create_chan(irc, "#foobar", 1, user);
    int retval;

    chan->mode |= chan_moderated;

    retval = irc_give_voice(chan, user);
    mu_assert_eq(retval, OK, "irc_give_voice failed");

    retval = irc_can_talk_in_channel(chan, user);
    mu_assert("user doesn't have voice", retval);

    irc_destroy(irc);
    mu_end;
}
int t_irc_can_talk_in_channel__moderated_no_voice__returns_false() {
    struct irc_globdata * irc = irc_init();
    struct ircuser * user = _irc_register_withnick(irc, 1, "Paco");
    struct ircchan * chan = _irc_create_chan(irc, "#foobar", 1, user);
    int retval;

    chan->mode |= chan_moderated;

    retval = irc_can_talk_in_channel(chan, user);
    mu_assert("user has voice", !retval);

    irc_destroy(irc);
    mu_end;
}
int t_irc_can_talk_in_channel__not_moderated__returns_true() {
    struct irc_globdata * irc = irc_init();
    struct ircuser * user = _irc_register_withnick(irc, 1, "Paco");
    struct ircchan * chan = _irc_create_chan(irc, "#foobar", 1, user);
    int retval;

    retval = irc_can_talk_in_channel(chan, user);
    mu_assert("user doesn't have voice", retval);

    irc_destroy(irc);
    mu_end;
}
int t_irc_name_matches__wildcard_combined__correct()
{
    char match[] = "implented";
    char nomatch[] = "wildcard";
    char pattern[] = "*mp*ent*";
    int retval;

    retval = irc_name_matches(pattern, match);
    mu_assert("didn't match", retval);

    retval = irc_name_matches(pattern, nomatch);
    mu_assert("matched", !retval);

    mu_end;
}
int t_irc_name_matches__wildcard_middle__correct()
{
    char match[] = "implented";
    char nomatch[] = "wildcard";
    char pattern[] = "imp*ented";
    int retval;

    retval = irc_name_matches(pattern, match);
    mu_assert("didn't match", retval);

    retval = irc_name_matches(pattern, nomatch);
    mu_assert("matched", !retval);

    mu_end;
}
int t_irc_name_matches__wildcard_end__correct()
{
    char match[] = "implented";
    char nomatch[] = "wildcard";
    char pattern[] = "implent*";
    int retval;

    retval = irc_name_matches(pattern, match);
    mu_assert("didn't match", retval);

    retval = irc_name_matches(pattern, nomatch);
    mu_assert("matched", !retval);

    mu_end;
}
int t_irc_name_matches__wildcard_begin__correct()
{
    char match[] = "implented";
    char nomatch[] = "wildcard";
    char pattern[] = "*ented";
    int retval;

    retval = irc_name_matches(pattern, match);
    mu_assert("didn't match", retval);

    retval = irc_name_matches(pattern, nomatch);
    mu_assert("matched", !retval);

    mu_end;
}
int t_irc_name_matches__no_wildcards__correct()
{
    char match[] = "implented";
    char nomatch[] = "wildcard";
    char pattern[] = "implented";
    int retval;

    retval = irc_name_matches(pattern, match);
    mu_assert("didn't match", retval);

    retval = irc_name_matches(pattern, nomatch);
    mu_assert("matched", !retval);

    mu_end;
}
int t_irc_user_inchannel__user_not_inchannel__returns_false()
{
    struct irc_globdata *irc = irc_init();
    struct ircuser *user = _irc_register_withnick(irc, 1, "pepe");
    struct ircchan *chan = _irc_create_chan(irc, "#test", 0);

    mu_assert_eq(irc_user_inchannel(chan, user), ERR_NOTFOUND, "User is not in channel, incorrect result");

    irc_destroy(irc);
    mu_end;
}
int t_irc_user_inchannel__user_in_channel__returns_true()
{
    struct irc_globdata *irc = irc_init();
    struct ircuser *user = _irc_register_withnick(irc, 1, "pepe");
    struct ircchan *chan = _irc_create_chan(irc, "#test", 1, user);

    mu_assert_eq(irc_user_inchannel(chan, user), OK, "User is in channel, incorrect result");

    irc_destroy(irc);
    mu_end;
}

/* END TESTS */

int test_irc_core_suite(int *errors, int *success)
{
    int tests_run = 0;
    int tests_passed = 0;

    printf("Begin test_irc_core suite.\n");
    /* BEGIN TEST EXEC */
	mu_run_test(t_irc_can_talk_in_channel__moderated_voice__returns_true);
	mu_run_test(t_irc_can_talk_in_channel__moderated_no_voice__returns_false);
	mu_run_test(t_irc_can_talk_in_channel__not_moderated__returns_true);
    mu_run_test(t_irc_name_matches__wildcard_combined__correct);
    mu_run_test(t_irc_name_matches__wildcard_middle__correct);
    mu_run_test(t_irc_name_matches__wildcard_end__correct);
    mu_run_test(t_irc_name_matches__wildcard_begin__correct);
    mu_run_test(t_irc_name_matches__no_wildcards__correct);
    mu_run_test(t_irc_user_inchannel__user_not_inchannel__returns_false);
    mu_run_test(t_irc_user_inchannel__user_in_channel__returns_true);

    /* END TEST EXEC */
    if (tests_passed == tests_run)
        printf("End test_irc_core suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
    else
        printf("End test_irc_core suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


    *errors += (tests_run - tests_passed);
    *success += tests_passed;
    return tests_run;
}

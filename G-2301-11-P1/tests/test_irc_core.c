#include "test_irc_core.h"
#include "testmacros.h"
#include "termcolor.h"
#include "irc_core.h"
#include "irc_testhelper.h"
#include <stdio.h>



/* BEGIN TESTS */
int t_irc_user_inchannel__user_not_inchannel__returns_false() {
	struct irc_globdata *irc = irc_init();
	struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
	struct ircchan* chan = _irc_create_chan(irc, "#test", 0);

	mu_assert_eq(irc_user_inchannel(chan, user), ERR_NOTFOUND, "User is not in channel, incorrect result");

	irc_destroy(irc);
	mu_end;
}
int t_irc_user_inchannel__user_in_channel__returns_true() {
	struct irc_globdata *irc = irc_init();
	struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
	struct ircchan* chan = _irc_create_chan(irc, "#test", 1, user);

	mu_assert_eq(irc_user_inchannel(chan, user), OK, "User is in channel, incorrect result");

	irc_destroy(irc);
	mu_end;
}

/* END TESTS */

int test_irc_core_suite(int* errors, int* success) {
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_irc_core suite.\n");
/* BEGIN TEST EXEC */
	mu_run_test(t_irc_user_inchannel__user_not_inchannel__returns_false);
	mu_run_test(t_irc_user_inchannel__user_in_channel__returns_true);
	
/* END TEST EXEC */
	if(tests_passed == tests_run)
		printf("End test_irc_core suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End test_irc_core suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

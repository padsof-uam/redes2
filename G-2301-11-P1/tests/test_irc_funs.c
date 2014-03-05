#include "test_irc_funs.h"
#include "testmacros.h"
#include <stdio.h>



/* BEGIN TESTS */
int t_irc_quit__message_provided__message_transmitted() {

	mu_end;
}
int t_irc_quit__no_message_provided__msg_is_nick() {

	mu_end;
}

/* END TESTS */

int test_irc_funs_suite(int* errors, int* success) {
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_irc_funs suite.\n");
/* BEGIN TEST EXEC */
	mu_run_test(t_irc_quit__message_provided__message_transmitted);
	mu_run_test(t_irc_quit__no_message_provided__msg_is_nick);
	
/* END TEST EXEC */
	if(tests_passed == tests_run)
		printf("End test_irc_funs suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End test_irc_funs suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

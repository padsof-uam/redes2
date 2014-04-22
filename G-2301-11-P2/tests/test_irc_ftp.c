#include "test_irc_ftp.h"
#include "testmacros.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* BEGIN TESTS */
int t_irc_ftp__transfer_complete__ok() {

	mu_fail("Not implemented");
	mu_end;
}
/* END TESTS */

int test_irc_ftp_suite(int *errors, int *success)
{
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_irc_ftp suite.\n");
	/* BEGIN TEST EXEC */
	mu_run_test(t_irc_ftp__transfer_complete__ok);
	/* END TEST EXEC */
	if (tests_passed == tests_run)
		printf("End test_irc_ftp suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End test_irc_ftp suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

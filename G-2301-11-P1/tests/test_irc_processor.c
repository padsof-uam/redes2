#include "test_irc_processor.h"
#include "irc_processor.h"
#include "testmacros.h"
#include <stdio.h>
#include <string.h>

/* BEGIN TESTS */
int t_irc_msgsep__length_limited__no_len_surpassing() {
	char str[] = "123456\r\n123";
	char* next = irc_msgsep(str, 3);

	mu_assert("the crlf was found.", !next);

	mu_end;
}
int t_irc_msgsep__null_str__return_null() {
	char* next = irc_msgsep(NULL, 3);

	mu_assert("retval was not null", !next);
	
	mu_end;
}
int t_irc_msgsep__with_crlf__separation_correct() {
	char str[] = "123456\r\n123";
	char* expected = str + 8;
	char* next = irc_msgsep(str, 40);

	mu_assert("the crlf was not found.", next == expected);
	mu_assert("separated message is not the same", !strcmp(str, "123456"));

	mu_end;
}
int t_irc_msgsep__no_crlf__notmodified() {
	char str[] = "123456123";
	char* original = strdup(str);
	char* next = irc_msgsep(str, 40);

	mu_assert("the crlf was found.", next == NULL);
	mu_assert("separated message is not the same", !strcmp(str, original));

	mu_end;
}

/* END TESTS */

int test_irc_processor_suite(int* errors, int* success) {
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_irc_processor suite.\n");
/* BEGIN TEST EXEC */
	mu_run_test(t_irc_msgsep__length_limited__no_len_surpassing);
	mu_run_test(t_irc_msgsep__null_str__return_null);
	mu_run_test(t_irc_msgsep__with_crlf__separation_correct);
	mu_run_test(t_irc_msgsep__no_crlf__notmodified);
	
/* END TEST EXEC */
	if(tests_passed == tests_run)
		printf("End test_irc_processor suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End test_irc_processor suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

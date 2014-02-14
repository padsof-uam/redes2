#include "listener.h"
#include "test_listener.h"
#include "testmacros.h"
#include <stdio.h>

/* BEGIN TESTS */
int t_server_openclose() {
	int handler = Server_open_socket(DEFAULT_PORT, DEFAULT_MAX_QUEUE);
	mu_assert("Server won't open", handler !=- ERR_SOCK);

	if(Server_close_communication(handler) != OK)
	{
		mu_sysfail("close error");
	}

	mu_end;
}

/* END TESTS */

int test_listener_suite(int* errors, int* success) {
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin listener suite.\n");
/* BEGIN TEST EXEC */
	mu_run_test(t_server_openclose);
	
/* END TEST EXEC */
	printf("End listener suite. %d/%d\n\n", tests_passed, tests_run);

	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

#include "tests.h"
#include "testmacros.h"
#include <stdio.h>



/* BEGIN TESTS */
int t_testA() {
	char a[] = "1";
	int b = 1;
	int result = atoi(a);

	mu_assert("Number is not equal", b == result);
	mu_end;
}

/* END TESTS */

int tests_suite(int* errors, int* success) {
	int tests_run;
	int tests_passed;

	printf("Begin tests suite.\n");
/* BEGIN TEST EXEC */
	mu_run_test(t_testA);
	
/* END TEST EXEC */
	printf("End tests suite. %d/%d\n", tests_passed, tests_run);

	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

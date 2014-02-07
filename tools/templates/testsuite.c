#include "_testsuite_.h"
#include "testmacros.h"
#include <stdio.h>



/* BEGIN TESTS */

/* END TESTS */

int _testsuite__suite(int* errors, int* success) {
	int tests_run;
	int tests_passed;

	printf("Begin _testsuite_ suite.\n");
/* BEGIN TEST EXEC */
	
/* END TEST EXEC */
	printf("End _testsuite_ suite. %d/%d\n\n", tests_passed, tests_run);

	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

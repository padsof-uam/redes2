#include "_testsuite_.h"
#include "testmacros.h"
#include "commparser.h"
#include <stdio.h>

/* BEGIN TESTS */
/* END TESTS */

int _testsuite__suite(int* errors, int* success) {
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin _testsuite_ suite.\n");
/* BEGIN TEST EXEC */
/* END TEST EXEC */
	if(tests_passed == tests_run)
		printf("End _testsuite_ suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End _testsuite_ suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

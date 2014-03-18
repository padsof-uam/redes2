#include "_testsuite_.h"
#include "testmacros.h"
#include "commparser.h"
#include <stdio.h>

const char* _cmds[] = { "cmd1", "cmd2", "cmd3"};

static int _test_func(void* data)
{
	return *(int*)data;
}

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

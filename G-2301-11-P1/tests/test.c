#include "test_commparser.h"
#include "daemon.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, const char** argv) {
	time_t t;
	int success = 0, error = 0, run = 0;
	time(&t);

	printf("Begin test run %s\n", ctime(&t));	
/* BEGIN TEST REGION */
	run += test_commparser_suite(&error, &success);
	run += daemon_suite(&error, &success);

/* END TEST REGION */
	time(&t);
	printf("End test run %s.\n", ctime(&t));
	printf("Run %d. %d success, %d errors.\n", run, success, error);

	return 0;
}

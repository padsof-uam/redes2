#include "list.h"
#include "test_list.h"
#include "testmacros.h"
#include <stdio.h>

#define MAX_LIST_ITEMS 200

/* BEGIN TESTS */
int t_list_complete_test() {
	int i, retval, count, item;
	list* l = list_new(int_duplicator, free);

	for(i = 0; i < MAX_LIST_ITEMS; i++)
	{
		retval = list_add(l, &i);
		mu_assert_eq(retval, OK, "add failed");
	}

	count = list_count(l);

	mu_assert_eq(count, MAX_LIST_ITEMS, "count invalid");

	for(i = 0; i < MAX_LIST_ITEMS; i++)
	{
		item = *(int*)list_at(l, i);
		mu_assert_eq(item, i, "item is not well-saved");
	}

	retval = list_insert(l, &i, MAX_LIST_ITEMS - 10);
	mu_assert_eq(retval, OK, "insert failed");
	item = *(int*)list_at(l, MAX_LIST_ITEMS - 10);
	mu_assert_eq(item, i, "incorrect item");

	list_clear(l);

	count = list_count(l);

	mu_assert_eq(count, 0, "count after clear invalid");

	list_destroy(l, NULL);

	mu_end;
}

/* END TESTS */

int test_list_suite(int* errors, int* success) {
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_list suite.\n");
/* BEGIN TEST EXEC */
	mu_run_test(t_list_complete_test);
	
/* END TEST EXEC */
	if(tests_passed == tests_run)
		printf("End test_list suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End test_list suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

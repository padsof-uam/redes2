#include "test_lfringbuf.h"
#include "testmacros.h"
#include "lfringbuf.h"
#include "sysutils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* BEGIN TESTS */
int t_lfringbuf_pop__empty_buffer_after_cycle__fails() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	int retval, i, val;

	if(!rb)
		mu_sysfail("lfringbuf_new");

	for(i = 0; i < 10; i++)
	{
		retval = lfringbuf_push(rb, &i);
		mu_assert_eq(retval, OK, "push: retval is not ok");
	}

	for(i = 0; i < 10; i++)
	{
		retval = lfringbuf_pop(rb, &val);
		mu_assert_eq(retval, OK, "pop: retval is not ok");
	}

	retval = lfringbuf_pop(rb, &val);
	mu_assert_eq(retval, ERR, "last pop: retval is not ok");

	lfringbuf_destroy(rb);
	mu_end;
}
int t_lfringbuf_push__full_buffer_after_cycle__fails() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	int retval, i, val;

	if(!rb)
		mu_sysfail("lfringbuf_new");

	for(i = 0; i < 10; i++)
	{
		retval = lfringbuf_push(rb, &i);
		mu_assert_eq(retval, OK, "push: retval is not ok");
	}

	for(i = 0; i < 10; i++)
	{
		retval = lfringbuf_pop(rb, &val);
		mu_assert_eq(retval, OK, "pop: retval is not ok");
	}

	for(i = 0; i < 10; i++)
	{
		retval = lfringbuf_push(rb, &i);
		mu_assert_eq(retval, OK, "push: retval is not ok");
	}

	retval = lfringbuf_push(rb, &val);
	mu_assert_eq(retval, ERR, "last push: retval is not ok");

	lfringbuf_destroy(rb);
	mu_end;
}

int t_lfringbuf_wait_for_items__empty_buffer_after_full__returns_timeout() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	int retval, i, val;

	if(!rb)
		mu_sysfail("lfringbuf_new");

	for(i = 0; i < 10; i++)
	{
		retval = lfringbuf_push(rb, &i);
		mu_assert_eq(retval, OK, "push: retval is not ok");
	}

	for(i = 0; i < 10; i++)
	{
		retval = lfringbuf_pop(rb, &val);
		mu_assert_eq(retval, OK, "pop: retval is not ok");
	}

	retval = lfringbuf_wait_for_items(rb, 100);

	if(retval != ERR_TIMEOUT)
		mu_sysfail("retval is not ERR_TIMEOUT");

	lfringbuf_destroy(rb);
	mu_end;
}
int t_lfringbuf_wait_for_items__empty_buffer_timeout__returns_timeout() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	int retval;

	if(!rb)
		mu_sysfail("lfringbuf_new");

	retval = lfringbuf_wait_for_items(rb, 100);

	if(retval != ERR_TIMEOUT)
		mu_sysfail("retval is not ERR_TIMEOUT");

	lfringbuf_destroy(rb);
	mu_end;
}
int t_lfringbuf_wait_for_items__items_in_buffer__returns_inmediately() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	int retval;
	int item = 2;

	if(!rb)
		mu_sysfail("lfringbuf_new");

	lfringbuf_push(rb, &item);
	retval = lfringbuf_wait_for_items(rb, 100);

	if(retval != OK)
		mu_sysfail("retval is not OK");

	lfringbuf_destroy(rb);
	mu_end;
}

static volatile short _thread_ended;

static void* _fun_wait(void* data)
{
	lfringbuf* rb = (lfringbuf*) data;
	
	lfringbuf_wait_for_items(rb, -1);

	_thread_ended = 1;
	return NULL;
}

int t_lfringbuf_wait_for_items__empty_buffer__waits() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	pthread_t th;
	int item = 2;

	if(!rb)
		mu_sysfail("lfringbuf_new");
	_thread_ended = 0;

	if(pthread_create(&th, NULL, _fun_wait, rb) != 0)
	{
		lfringbuf_destroy(rb);
		mu_sysfail("pthread creation failed.");
	}

	usleep(100 * 1000);
	mu_assert_eq(_thread_ended, 0, "the thread didn't wait.");

	lfringbuf_push(rb, &item);

	usleep(100 * 1000);	
	mu_assert_eq(_thread_ended, 1, "the thread is still waiting.");

	lfringbuf_destroy(rb);
	pthread_cancel_join(&th);
	mu_end;
}
int t_lfringbuf_pop__buffer_full__element_retrieved() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	int retval, i, val;

	if(!rb)
		mu_sysfail("lfringbuf_new");

	for(i = 0; i < 10; i++)
	{
		retval = lfringbuf_push(rb, &i);
		mu_assert_eq(retval, OK, "push: retval is not ok");
	}

	retval = lfringbuf_pop(rb, &val);
	mu_assert_eq(retval, OK, "pop: retval is not ok");
	mu_assert_eq(val, 0, "retrieved value");

	lfringbuf_destroy(rb);
	mu_end;
}
int t_lfringbuf_pop__items_in_buffer__element_retrieved() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	int retval, i, val;

	for(i = 0; i < 5; i++)
	{
		retval = lfringbuf_push(rb, &i);
		mu_assert_eq(retval, OK, "push: retval is not ok");
	}

	retval = lfringbuf_pop(rb, &val);
	mu_assert_eq(retval, OK, "pop: retval is not ok");
	mu_assert_eq(val, 0, "retrieved value");

	lfringbuf_destroy(rb);
	mu_end;
}
int t_lfringbuf_pop__empty_buffer__fails() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	int retval, val;

	if(!rb)
		mu_sysfail("lfringbuf_new");

	retval = lfringbuf_pop(rb, &val);
	mu_assert_eq(retval, ERR, "pop: retval is not error");

	lfringbuf_destroy(rb);
	mu_end;
}
int t_lfringbuf_push__buffer_full__fails() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	int retval, i, val = 10;

	if(!rb)
		mu_sysfail("lfringbuf_new");

	for(i = 0; i < 10; i++)
	{
		retval = lfringbuf_push(rb, &i);
		mu_assert_eq(retval, OK, "push: retval is not ok");
	}

	retval = lfringbuf_push(rb, &val);
	mu_assert_eq(retval, ERR, "push: retval is not ERR");

	lfringbuf_destroy(rb);
	mu_end;
}
int t_lfringbuf_push__items_in_buffer__element_pushed() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	int retval, i;
	
	if(!rb)
		mu_sysfail("lfringbuf_new");

	for(i = 0; i < 5; i++)
	{
		retval = lfringbuf_push(rb, &i);
		mu_assert_eq(retval, OK, "push: retval is not ok");
	}

	lfringbuf_destroy(rb);
	mu_end;
}
int t_lfringbuf_push__empty_buffer__element_pushed() {
	lfringbuf* rb = lfringbuf_new(10, sizeof(int));
	int retval, i = 6, val;

	retval = lfringbuf_push(rb, &i);
	mu_assert_eq(retval, OK, "push: retval is not ok");

	retval = lfringbuf_pop(rb, &val);
	mu_assert_eq(retval, OK, "pop: retval is not ok");
	mu_assert_eq(val, i, "value is not the same as was pushed");

	lfringbuf_destroy(rb);
	mu_end;
}
/* END TESTS */

int test_lfringbuf_suite(int *errors, int *success)
{
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_lfringbuf suite.\n");
	/* BEGIN TEST EXEC */
	mu_run_test(t_lfringbuf_pop__empty_buffer_after_cycle__fails);
	mu_run_test(t_lfringbuf_push__full_buffer_after_cycle__fails);
	mu_run_test(t_lfringbuf_wait_for_items__empty_buffer_after_full__returns_timeout);
	mu_run_test(t_lfringbuf_wait_for_items__empty_buffer_timeout__returns_timeout);
	mu_run_test(t_lfringbuf_wait_for_items__items_in_buffer__returns_inmediately);
	mu_run_test(t_lfringbuf_wait_for_items__empty_buffer__waits);
	mu_run_test(t_lfringbuf_pop__buffer_full__element_retrieved);
	mu_run_test(t_lfringbuf_pop__items_in_buffer__element_retrieved);
	mu_run_test(t_lfringbuf_pop__empty_buffer__fails);
	mu_run_test(t_lfringbuf_push__buffer_full__fails);
	mu_run_test(t_lfringbuf_push__items_in_buffer__element_pushed);
	mu_run_test(t_lfringbuf_push__empty_buffer__element_pushed);
	/* END TEST EXEC */
	if (tests_passed == tests_run)
		printf("End test_lfringbuf suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End test_lfringbuf suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

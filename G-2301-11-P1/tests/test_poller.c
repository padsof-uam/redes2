#include "test_poller.h"
#include "testmacros.h"
#include <stdio.h>
#include "poller.h"


/* BEGIN TESTS */
int t_pollfds_remove__moves_last() {
	int i;
	struct pollfds* pfds = pollfds_init(0);

	for(i = 0; i < 5; i++)
		pollfds_add(pfds, i);

	pollfds_remove(pfds, 2);

	mu_assert_eq(pfds->fds[2].fd, 4, "last message is not moved");
	mu_assert_eq(pfds->len, 4, "len was not modified");
	mu_end;
}
int t_pollfds_add__normal_add__adds_fds() {
	int flags = 12;
	struct pollfds* pfds = pollfds_init(flags);
	int fd = 5;
	int retval = pollfds_add(pfds, fd);

	mu_assert_eq(retval, OK, "operation failed");
	mu_assert_eq(pfds->len, 1, "len is not correct");
	mu_assert_eq(pfds->fds[0].fd, fd, "fd is not the same");
	mu_assert_eq(pfds->fds[0].events, flags, "flags are not correctly set");

	mu_end;
}
int t_pollfds_add__no_more_capacity__asks_more_memory() {
	int i, retval;
	struct pollfds* pfds = pollfds_init(0);

	for(i = 0; i < DEFAULT_PFDS_LEN; i++)
		pollfds_add(pfds, i);

	retval = pollfds_add(pfds, i);
	mu_assert_eq(retval, OK, "operation failed");
	mu_assert("capacity is not increased", pfds->capacity > DEFAULT_PFDS_LEN);
	mu_assert_eq(pfds->len, DEFAULT_PFDS_LEN + 1, "len is not correct");
	mu_assert_eq(pfds->fds[DEFAULT_PFDS_LEN].fd, i, "fd is not the same");

	mu_end;
}
int t_pollfds_setcapacity__capacity_is_greater__realloc_mem() {
	struct pollfds* pfds = pollfds_init(0);
	int retval = pollfds_setcapacity(pfds, DEFAULT_PFDS_LEN + 20);

	mu_assert_eq(retval, OK, "operation failed");
	mu_assert_eq(pfds->capacity, DEFAULT_PFDS_LEN + 20,	"capacity not modified");

	mu_end;
}
int t_pollfds_setcapacity__capacity_is_inferior__notchanged() {
	struct pollfds* pfds = pollfds_init(0);
	int retval = pollfds_setcapacity(pfds, DEFAULT_PFDS_LEN - 1);

	mu_assert_eq(retval, OK, "operation failed");
	mu_assert_eq(pfds->capacity, DEFAULT_PFDS_LEN,	"capacity modified");

	mu_end;
}

/* END TESTS */

int test_poller_suite(int* errors, int* success) {
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_poller suite.\n");
/* BEGIN TEST EXEC */
	mu_run_test(t_pollfds_remove__moves_last);
	mu_run_test(t_pollfds_add__normal_add__adds_fds);
	mu_run_test(t_pollfds_add__no_more_capacity__asks_more_memory);
	mu_run_test(t_pollfds_setcapacity__capacity_is_greater__realloc_mem);
	mu_run_test(t_pollfds_setcapacity__capacity_is_inferior__notchanged);
	
/* END TEST EXEC */
	printf("End test_poller suite. %d/%d\n\n", tests_passed, tests_run);

	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

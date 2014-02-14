#include "listener.h"
#include "test_listener.h"
#include "testmacros.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

/* BEGIN TESTS */
int t_Server_close_communication__close_noconnected_socket() {
	int sock = socket(AF_INET, SOCK_STREAM, TCP);
	int res = Server_close_communication(sock);
	mu_assert_eq(res, OK, "close failed");

	mu_end;
}

int t_Server_close_communication__close_connected_socket() {
	int sock[2];
	int res;
	if (socketpair(PF_LOCAL, SOCK_STREAM, 0, sock) == -1)
    {
        mu_sysfail("socketpair");
    }
    res = Server_close_communication(sock[0]);
    mu_assert_eq(res, OK, "close s1 failed");

    res = Server_close_communication(sock[1]);
    mu_assert_eq(res, OK, "close s2 failed");

	mu_end;
}

int t_server_open_socket__opensocket() {
	int handler = Server_open_socket(DEFAULT_PORT, DEFAULT_MAX_QUEUE);
	mu_assert("Server won't open", handler !=- ERR_SOCK);

	close(handler);
	mu_end;
}

/* END TESTS */

int test_listener_suite(int* errors, int* success) {
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin listener suite.\n");
/* BEGIN TEST EXEC */
	mu_run_test(t_Server_close_communication__close_noconnected_socket);
	mu_run_test(t_Server_close_communication__close_connected_socket);
	mu_run_test(t_server_open_socket__opensocket);
	
/* END TEST EXEC */
	printf("End listener suite. %d/%d\n\n", tests_passed, tests_run);

	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

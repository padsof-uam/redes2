#include "listener.h"
#include "test_listener.h"
#include "testmacros.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

/* BEGIN TESTS */
int t_Server_close_communication__close_noconnected_socket() {
	int sock = socket(AF_INET, SOCK_STREAM, TCP);
	int res = server_close_communication(sock);

	if(res != OK)
		mu_sysfail("close failed");

	mu_end;
}

int t_Server_close_communication__close_connected_socket() {
	int sock[2];
	int res;
	if (socketpair(PF_LOCAL, SOCK_STREAM, 0, sock) == -1)
        mu_sysfail("socketpair");

    res = server_close_communication(sock[0]);
    
	if(res != OK)
		mu_sysfail("close s1 failed");


    res = server_close_communication(sock[1]);
    
	if(res != OK)
		mu_sysfail("close s2 failed");


	mu_end;
}

int t_server_open_socket__opensocket() {
	int handler = server_open_socket(DEFAULT_PORT, DEFAULT_MAX_QUEUE);
	mu_assert("Server won't open", handler !=ERR_SOCK);

	close(handler);
	mu_end;
}

/* END TESTS */

int test_listener_suite(int* errors, int* success) {
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_listener suite.\n");
/* BEGIN TEST EXEC */
	mu_run_test(t_Server_close_communication__close_noconnected_socket);
	mu_run_test(t_Server_close_communication__close_connected_socket);
	mu_run_test(t_server_open_socket__opensocket);
	
/* END TEST EXEC */
	if(tests_passed == tests_run)
		printf("End test_listener suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End test_listener suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);

	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

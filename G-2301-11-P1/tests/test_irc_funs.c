#include "test_irc_funs.h"
#include "testmacros.h"
#include "irc_funs.h"
#include "irc_core.h"
#include "irc_processor.h"
#include "types.h"

#include <string.h>
#include <stdio.h>

static list* _process_message(cmd_action action, struct irc_globdata* gdata, int fd, char* message)
{
	struct irc_msgdata data;

	data.globdata = gdata;
	data.msg_tosend = list_new();
	data.msg = message;
	data.msgdata = malloc(sizeof(struct sockcomm_data));
	data.msgdata->fd = fd;
	strcpy(data.msgdata->data, message);
	data.msgdata->len = strlen(message);

	action(&data);

	free(data.msgdata);
	return data.msg_tosend;
}

/* BEGIN TESTS */
int t_irc_quit__message_provided__message_transmitted() {
	struct irc_globdata* irc = irc_init();
	struct ircchan* chan = irc_register_channel(irc, "testchan");
	list* output;
	struct sockcomm_data* msg;
	struct ircuser* user = irc_register_user(irc, 1);

	list_add(chan->users, user);
	list_add(chan->users, irc_register_user(irc, 2));
	list_add(user->channels, chan);
	
	irc_set_usernick(irc, 1, "pepe");

	output = _process_message(irc_quit, irc, 1, "QUIT Bye");

	mu_assert_eq(list_count(output), 1, "Incorrect number of generated messages.");
	msg = list_at(output, 0);

	mu_assert_streq(msg->data, ":pepe QUIT :Bye", "Bad message");
	mu_end;
}
int t_irc_quit__no_message_provided__msg_is_nick() {
	struct irc_globdata* irc = irc_init();
	struct ircchan* chan = irc_register_channel(irc, "testchan");
	list* output;
	struct sockcomm_data* msg;
	struct ircuser* user = irc_register_user(irc, 1);

	list_add(chan->users, user);
	list_add(chan->users, irc_register_user(irc, 2));
	list_add(user->channels, chan);

	irc_set_usernick(irc, 1, "pepe");

	output = _process_message(irc_quit, irc, 1, "QUIT");

	mu_assert_eq(list_count(output), 1, "Incorrect number of generated messages.");
	msg = list_at(output, 0);

	mu_assert_streq(msg->data, ":pepe QUIT :pepe", "Bad message");
	mu_end;
}

/* END TESTS */

int test_irc_funs_suite(int* errors, int* success) {
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_irc_funs suite.\n");
/* BEGIN TEST EXEC */
	mu_run_test(t_irc_quit__message_provided__message_transmitted);
	mu_run_test(t_irc_quit__no_message_provided__msg_is_nick);
	
/* END TEST EXEC */
	if(tests_passed == tests_run)
		printf("End test_irc_funs suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End test_irc_funs suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}

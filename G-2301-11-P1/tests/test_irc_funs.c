#include "test_irc_funs.h"
#include "testmacros.h"
#include "irc_funs.h"
#include "irc_core.h"
#include "irc_processor.h"
#include "types.h"
#include "irc_codes.h"
#include "strings.h"
#include "irc_testhelper.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

<<<<<<< HEAD
/** 
 * Usamos macros para facilitar la creación de tests aunque se pierda un poco 
 *  de claridad
 */
#define assert_generated(msgnum) mu_assert_eq(list_count(output), msgnum, "Incorrect number of generated messages.")
#define msgnum(num) ((struct sockcomm_data*) list_at(output, num))
#define irc_testend irc_destroy(irc); list_destroy(output, free); mu_end
#define assert_msgstr_eq(msg, str) mu_assert_streq(msg->data, str, "Message string doesn't correspond")
#define assert_numeric_reply(msg, expected_reply, params) do { if(_assert_numeric_reply(msg, expected_reply, params) != MU_PASSED) return MU_ERR; } while(0)
#define assert_dest(msg, dest) mu_assert_eq(msg->fd, dest->fd, "Destinatary is not the expected.")

/* Esto vale? */
#define assert_created_user_nick(nick) (dic_lookup(irc->nick_user_map, nick))
#define assert_created_user_id(id) (dic_lookup(irc->fd_user_map, &id))

static int _assert_numeric_reply(struct sockcomm_data *msg, int expected_reply, const char* additional_params)
{
	char servname[MAX_SERVER_NAME];
	int actual_reply;
	char param[MAX_IRC_MSG];
	char* param_start;
	char user[MAX_NICK_LEN];
	char* colon;
	char err_msg[MAX_IRC_MSG + 30];

	snprintf(err_msg, MAX_IRC_MSG + 30, "Error verifying response %s", msg->data);
	sscanf(msg->data, ":%s %d %s %s", servname, &actual_reply, user, param);
	param_start = strnstr(msg->data, param, MAX_IRC_MSG);

	colon = strchr(param_start, ':');

	if(colon)
	{
		colon--;
		*colon = '\0';
	}
=======
/* BEGIN TESTS */
int t_irc_topic__topic_change__topic_changed_and_broadcast() {
    struct irc_globdata *irc = irc_init();
    list* output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "paco");
    struct ircchan* chan =  _irc_create_chan(irc, "#testchan", 2, a, b);
>>>>>>> 959f3c1ec1f1f8a1cbf2a2edc3ab8961aecb46c0

    output = _process_message(irc_topic, irc, 1, "TOPIC #testchan :Tema de prueba");

    assert_generated(2);
    assert_dest(msgnum(0), a);
    assert_dest(msgnum(1), b);
    assert_msgstr_eq(msgnum(0), ":pepe TOPIC #testchan :Tema de prueba");
    assert_msgstr_eq(msgnum(1), ":pepe TOPIC #testchan :Tema de prueba");
    mu_assert_streq(chan->topic, "Tema de prueba", "Topic not set");

	irc_testend;
}
int t_irc_topic__op_needed__returns_needmoreprivileges() {
    struct irc_globdata *irc = irc_init();
    list* output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "paco");
    struct ircchan* chan =  _irc_create_chan(irc, "#testchan", 2, a, b);    
	
    chan->mode |= chan_topiclock;

    output = _process_message(irc_topic, irc, 1, "TOPIC #testchan :Tema de prueba");
    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_CHANOPRIVSNEEDED, "#testchan");
    irc_testend;
}
int t_irc_topic__topic_notset_request__returns_notopic() {
    struct irc_globdata *irc = irc_init();
    list* output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    _irc_create_chan(irc, "#testchan", 1, a);
    output = _process_message(irc_topic, irc, 1, "TOPIC #testchan");
    
    assert_generated(1);
    assert_numeric_reply(msgnum(0), RPL_NOTOPIC, "#testchan");
	irc_testend;
}
int t_irc_topic__topic_set_requested__returns_topic() {
    struct irc_globdata *irc = irc_init();
    list* output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    struct ircchan* chan =  _irc_create_chan(irc, "#testchan", 1, a);    
    strcpy(chan->topic, "Prueba");

    output = _process_message(irc_topic, irc, 1, "TOPIC #testchan");
    
    assert_generated(1);
    mu_assert("Message doesn't contain the topic", strstr(msgnum(0)->data, ":Prueba") != NULL);
    assert_numeric_reply(msgnum(0), RPL_TOPIC, "#testchan");
	
    irc_testend;
}
int t_irc_topic__no_params__returns_needmoreparams() {
    struct irc_globdata *irc = irc_init();
    list* output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "paco");
    _irc_create_chan(irc, "#testchan", 2, a, b);    
    
    output = _process_message(irc_topic, irc, 1, "TOPIC");
    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NEEDMOREPARAMS, "TOPIC");
    irc_testend;
}
<<<<<<< HEAD

/* BEGIN TESTS */
int t_irc_user_not_registered() {
	mu_fail("Not implemented");
	mu_end;
}
int t_irc_user_bad_params() {
    struct irc_globdata *irc = irc_init();
    list* output;
    char * msg;


    msg = strdup("USER ignore ignorar :Guillermo de Juan");
    output = _process_message(irc_user, irc, 1, msg);
    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NEEDMOREPARAMS, NULL);
    free(msg);
    
    msg = strdup("USER Juan ignorar :Guillermo de Juan");
    list_add(output,_process_message(irc_user, irc, 1, msg));
    assert_generated(2);
    assert_numeric_reply(msgnum(1), ERR_NEEDMOREPARAMS, NULL);    
    free(msg);

    msg = strdup("USER Juan :Guillermo de Juan");
    list_add(output,_process_message(irc_user, irc, 1, msg));
    assert_generated(3);
    //assert_numeric_reply(msgnum(2), ERR_NEEDMOREPARAMS, NULL);
    free(msg);

    irc_testend;
}

int t_irc_user() {
    struct irc_globdata *irc = irc_init();
    list* output;
    int fd = 1;

    char * msg = strdup("USER Ralph Ignore ign :comida de gato");

    struct ircuser *a = _irc_register_withnick(irc, fd, "Eloy");
    output = _process_message(irc_user, irc, fd, msg);

    struct ircuser *b = assert_created_user_id(fd);


    mu_assert_streq(b->name, a->name, "L");

    assert_generated(0);

    irc_testend;
}


int t_irc_nick_not_registered() {

    struct irc_globdata *irc = irc_init();
    list* output;

    output = _process_message(irc_nick, irc, 4, "NICK Eloy");
    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_ERRONEUSNICKNAME, NULL);
    irc_testend;
}
int t_irc_nick_collision() {
    struct irc_globdata *irc = irc_init();
    list* output;

    struct ircuser *a = _irc_register_withnick(irc, 4, "Eloy");
    output = _process_message(irc_nick, irc, 4, "NICK Eloy");
    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NICKCOLLISION, NULL);
	irc_testend;
}
int t_irc_nick() {

    struct irc_globdata *irc = irc_init();
    list* output;

    struct ircuser *a = _irc_register_withnick(irc, 4, "Eloy");
    output = _process_message(irc_nick, irc, 4, "NICK Juan");
    assert_generated(0);

    _irc_register_withnick(irc, 5, "Alfredo");
    output = _process_message(irc_nick, irc, 4, ":Alfredo NICK Barbacoa");
    assert_generated(0);

    assert_created_user_nick("Juan");
    assert_created_user_nick("Barbacoa");
    irc_testend;
}
=======
>>>>>>> 959f3c1ec1f1f8a1cbf2a2edc3ab8961aecb46c0
int t_irc_privmsg__send_to_channel__sent_to_users_in_channel()
{
    struct irc_globdata *irc = irc_init();
    list* output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "paco");
    _irc_register_withnick(irc, 3, "luis");

    _irc_create_chan(irc, "#testchan", 2, a, b);

    output = _process_message(irc_privmsg, irc, 3, "PRIVMSG #testchan :todos");

    assert_generated(2);
    assert_dest(msgnum(0), a);
    assert_dest(msgnum(1), b);
    assert_msgstr_eq(msgnum(0), ":luis PRIVMSG #testchan :todos");
    assert_msgstr_eq(msgnum(1), ":luis PRIVMSG #testchan :todos");

    irc_testend;
}

int t_irc_privmsg__send_to_user__user_receives_it()
{
    struct irc_globdata *irc = irc_init();
    list* output;

    _irc_register_withnick(irc, 0, "paco");
    _irc_register_withnick(irc, 1, "pepe");

    output = _process_message(irc_privmsg, irc, 0, "PRIVMSG pepe :hola :)");

    assert_generated(1);
    assert_msgstr_eq(msgnum(0), ":paco PRIVMSG pepe :hola :)");

    irc_testend;
}

int t_irc_privmsg__no_text_provided__err_notexttosend()
{
    struct irc_globdata *irc = irc_init();
    list* output;

    output = _process_message(irc_privmsg, irc, 0, "PRIVMSG pepe");

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NOTEXTTOSEND, NULL);

    irc_testend;
}

int t_irc_privmsg__dest_not_provied__err_norecipient()
{
    struct irc_globdata *irc = irc_init();
    list* output;

    output = _process_message(irc_privmsg, irc, 0, "PRIVMSG");

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NORECIPIENT, NULL);

    irc_testend;
}

int t_irc_privmsg__dest_doesnt_exist__err_no_such_nick()
{
    struct irc_globdata *irc = irc_init();
    list* output;

    output = _process_message(irc_privmsg, irc, 0, "PRIVMSG pepe: hola!!!");

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NOSUCHNICK, NULL);

    irc_testend;
}

int t_irc_privmsg__dest_away__replies_away()
{
    struct irc_globdata *irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 0, "pepe");
    list* output;
    _irc_register_withnick(irc, 1, "luis");

    user->is_away = 1;
    strcpy(user->away_msg, "Estoy programando redes.");

    output = _process_message(irc_privmsg, irc, 1, "PRIVMSG pepe :hola");

    assert_generated(1);
    assert_msgstr_eq(msgnum(0), ":redes-ircd 301 luis pepe :Estoy programando redes.");

    irc_testend;
}

int t_irc_quit__message_provided__message_transmitted()
{
    struct irc_globdata *irc = irc_init();
    struct ircchan *chan = irc_register_channel(irc, "testchan");
    list *output;
    struct ircuser *user = irc_register_user(irc, 1);

    list_add(chan->users, user);
    list_add(chan->users, irc_register_user(irc, 2));
    list_add(user->channels, chan);

    irc_set_usernick(irc, 1, "pepe");

    output = _process_message(irc_quit, irc, 1, "QUIT Bye");

    assert_generated(1);
    assert_msgstr_eq(msgnum(0), ":pepe QUIT :Bye");
    irc_testend;
}
int t_irc_quit__no_message_provided__msg_is_nick()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    struct ircuser *user = _irc_register_withnick(irc, 1, "pepe");
    _irc_create_chan(irc, "#testchan", 2, user, irc_register_user(irc, 2));

    output = _process_message(irc_quit, irc, 1, "QUIT");

    assert_generated(1);
    assert_msgstr_eq(msgnum(0), ":pepe QUIT :pepe");

    irc_testend;
}
/* END TESTS */

int test_irc_funs_suite(int *errors, int *success)
{
    int tests_run = 0;
    int tests_passed = 0;

    printf("Begin test_irc_funs suite.\n");
    /* BEGIN TEST EXEC */
<<<<<<< HEAD
	mu_run_test(t_irc_user_not_registered);
	mu_run_test(t_irc_user_bad_params);
    mu_run_test(t_irc_user);
	mu_run_test(t_irc_nick_not_registered);
	mu_run_test(t_irc_nick_collision);
	mu_run_test(t_irc_nick);
=======
	mu_run_test(t_irc_topic__topic_change__topic_changed_and_broadcast);
	mu_run_test(t_irc_topic__op_needed__returns_needmoreprivileges);
	mu_run_test(t_irc_topic__topic_notset_request__returns_notopic);
	mu_run_test(t_irc_topic__topic_set_requested__returns_topic);
	mu_run_test(t_irc_topic__no_params__returns_needmoreparams);
>>>>>>> 959f3c1ec1f1f8a1cbf2a2edc3ab8961aecb46c0
    mu_run_test(t_irc_privmsg__send_to_channel__sent_to_users_in_channel);
    mu_run_test(t_irc_privmsg__send_to_user__user_receives_it);
    mu_run_test(t_irc_privmsg__no_text_provided__err_notexttosend);
    mu_run_test(t_irc_privmsg__dest_not_provied__err_norecipient);
    mu_run_test(t_irc_privmsg__dest_doesnt_exist__err_no_such_nick);
    mu_run_test(t_irc_privmsg__dest_away__replies_away);
    mu_run_test(t_irc_quit__message_provided__message_transmitted);
    mu_run_test(t_irc_quit__no_message_provided__msg_is_nick);

    /* END TEST EXEC */
    if (tests_passed == tests_run)
        printf("End test_irc_funs suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
    else
        printf("End test_irc_funs suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


    *errors += (tests_run - tests_passed);
    *success += tests_passed;
    return tests_run;
}

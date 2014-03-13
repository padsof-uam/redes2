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

/* BEGIN TESTS */
int t_irc_join__all_together() {

	mu_fail("Not implemented");
	mu_end;
}

int t_irc_join__invited__not_inv() {

	mu_fail("Not implemented");
	mu_end;
}

int t_irc_join__1_invited_ok() {

	mu_fail("Not implemented");
	mu_end;
}

int t_irc_join__1_channel_pass__ok() {

	mu_fail("Not implemented");
	mu_end;
}

int t_irc_join__1_channel_pass__badpass() {

	mu_fail("Not implemented");
	mu_end;
}

int t_irc_join__more_channel_no_pass() {

	mu_fail("Not implemented");
	mu_end;
}

int t_irc_join__1_channel_no_pass() {

	mu_fail("Not implemented");
	mu_end;
}

int t_irc_join__bad_params() {

	mu_fail("Not implemented");
	mu_end;
}

int t_irc_mode__chan_provided__returns_chan_mode() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
    struct ircchan* chan = _irc_create_chan(irc, "#testchan", 1, user);
    char msg[] = "MODE #testchan";
    list* output;

    chan->mode = chan_invite | chan_moderated;

    output = _process_message(irc_mode, irc, 1, msg);
    assert_generated(1);
    assert_numeric_reply_text(msgnum(0), RPL_CHANNELMODEIS, "#testchan", "+im");

    irc_testend;
}
int t_irc_mode__user_provided__returns_user_mode() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* luis = _irc_register_withnick(irc, 2, "luis");
    char msg[] = "MODE luis";
    list* output;

    _irc_register_withnick(irc, 1, "pepe");

    luis->mode = user_op | user_rcvwallops;

    output = _process_message(irc_mode, irc, 1, msg);
    assert_generated(1);
    assert_numeric_reply_text(msgnum(0), RPL_UMODEIS, "luis", "+wo");

    irc_testend;
}
int t_irc_mode__different_user__err_match() {
    struct irc_globdata* irc = irc_init();
    char msg[] = "MODE luis +i";
    list* output;
    _irc_register_withnick(irc, 1, "pepe");

    output = _process_message(irc_mode, irc, 1, msg);

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_USERSDONTMATCH, NULL);

    irc_testend;
}
int t_irc_mode__user_minus_o_op__removes_op_privs() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
    char msg[] = "MODE pepe -o";
    list* output;

    user->mode = user_op;

    output = _process_message(irc_mode, irc, 1, msg);

    mu_assert_eq(user->mode, 0, "user is still op");

    irc_testend;
}
int t_irc_mode__user_plus_o__ignored() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
    char msg[] = "MODE pepe +o";
    list* output;

    user->mode = 0;

    output = _process_message(irc_mode, irc, 1, msg);

    mu_assert_eq(user->mode, 0, "user was changed to op");

    irc_testend;
}
int t_irc_mode__plus_k_no_op__err_privsneeded() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
    char msg[] = "MODE #testchan +k key";
    list* output;
    _irc_create_chan(irc, "#testchan", 1, user);

    output = _process_message(irc_mode, irc, 1, msg);

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_CHANOPRIVSNEEDED, NULL);

    irc_testend;
}
int t_irc_mode__minus_k__removes_password() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
    struct ircchan* chan = _irc_create_chan(irc, "#testchan", 1, user);
    char msg[] = "MODE #testchan -k";
    list* output;
    chan->has_password = 1;
    user->mode = user_op;

    output = _process_message(irc_mode, irc, 1, msg);

    assert_generated(1);
    mu_assert_eq(chan->has_password, 0, "password is still set");

    irc_testend;
}
int t_irc_mode__plus_k_pass_provided__sets_password() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
    struct ircchan* chan = _irc_create_chan(irc, "#testchan", 1, user);
    char msg[] = "MODE #testchan +k pass";
    list* output;

    user->mode = user_op;

    output = _process_message(irc_mode, irc, 1, msg);
    assert_generated(1);

    mu_assert("password not set", chan->has_password);
    mu_assert_streq(chan->password, "pass", "password is incorrect");

    irc_testend;
}
int t_irc_mode__plus_k_no_pass_provided__err_moreparams() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
    char msg[] = "MODE #testchan +k";
    list* output;
    _irc_create_chan(irc, "#testchan", 1, user);

    user->mode = user_op;

    output = _process_message(irc_mode, irc, 1, msg);

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NEEDMOREPARAMS, NULL);

    irc_testend;
}
int t_irc_mode__minus_l__removes_user_limit() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
    struct ircchan* chan = _irc_create_chan(irc, "#testchan", 1, user);
    char msg[] = "MODE #testchan -l";
    list* output;

    user->mode = user_op;
    chan->user_limit = 10;

    output = _process_message(irc_mode, irc, 1, msg);

    mu_assert_eq(chan->user_limit, -1, "limit is still set");

    irc_testend;
}
int t_irc_mode__plus_l__sets_user_limit() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
    struct ircchan* chan = _irc_create_chan(irc, "#testchan", 1, user);
    char msg[] = "MODE #testchan +l 100";
    list* output;

    user->mode = user_op;

    output = _process_message(irc_mode, irc, 1, msg);

    assert_generated(1);
    mu_assert_eq(chan->user_limit, 100, "limit is wrong");

    irc_testend;
}
int t_irc_mode__plus_o_noop__err_chanopprivsneeded() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
    char msg[] = "MODE #testchan +o pepe";
    list* output;
    _irc_create_chan(irc, "#testchan", 1, user);

    output = _process_message(irc_mode, irc, 1, msg);

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_CHANOPRIVSNEEDED, NULL);

    irc_testend;
}
int t_irc_mode__plus_o_op__gives_chanop_target() {
    struct irc_globdata* irc = irc_init();
    struct ircuser* user = _irc_register_withnick(irc, 1, "pepe");
    struct ircuser* op = _irc_register_withnick(irc, 2, "luis");
    struct ircchan* chan = _irc_create_chan(irc, "#testchan", 2, user, op);
    char msg[] = "MODE #testchan +o luis";
    list* output;

    user->mode = user_op;

    output = _process_message(irc_mode, irc, 1, msg);

    assert_generated(1);
    mu_assert("luis is not op", irc_is_channel_op(chan, op));

    irc_testend;
}
int t_irc_mode__no_target_provided__err_moreparams() {
    struct irc_globdata* irc = irc_init();
    char msg[] = "MODE";
    list* output;
    _irc_register_withnick(irc, 1, "pepe");

    output = _process_message(irc_mode, irc, 1, msg);

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NEEDMOREPARAMS, NULL);

    irc_testend;
}

int t_irc_kick__you_are_not_in_channel()
{

    struct irc_globdata *irc = irc_init();
    list *output;
    char str[] = "KICK #testchan Paco";
    struct ircuser *b = _irc_register_withnick(irc, 2, "Paco");
    _irc_create_chan(irc, "#testchan", 1, b);

    _irc_register_withnick(irc, 1, "Pepe");


    output = _process_message(irc_kick, irc, 1, str);

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NOTONCHANNEL, NULL);
    irc_testend;

}
int t_irc_kick__kicked()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char str[] = "KICK #testchan Paco";

    struct ircuser *a = _irc_register_withnick(irc, 1, "Pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "Paco");


    struct ircchan *channel = _irc_create_chan(irc, "#testchan", 2, a, b);
    irc_channel_addop(channel, a);

    output = _process_message(irc_kick, irc, 1, str);
    assert_generated(2);
    assert_msgstr_eq(msgnum(0), ":Pepe PART #testchan Paco");
    assert_msgstr_eq(msgnum(1), ":Pepe PART #testchan Paco");

    irc_testend;
}
int t_irc_kick__chan_op_priv_need()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char str[] = "KICK #testchan Paco";

    struct ircuser *a = _irc_register_withnick(irc, 1, "Pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "Paco");

    _irc_create_chan(irc, "#testchan", 2, a, b);

    output = _process_message(irc_kick, irc, 1, str);

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_CHANOPRIVSNEEDED, NULL);
    irc_testend;
}
int t_irc_kick__not_in_channel()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char str[] = "KICK #testchan Oscar";


    struct ircuser *a = _irc_register_withnick(irc, 1, "Pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "Paco");

    struct ircchan *channel = _irc_create_chan(irc, "#testchan", 2, a, b);
    irc_channel_addop(channel, a);

    output = _process_message(irc_kick, irc, 1, str);

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_USERNOTINCHANNEL , NULL);
    irc_testend;
}
int t_irc_kick__no_such_channel()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char str[] = "KICK #patata Paco";

    struct ircuser *a = _irc_register_withnick(irc, 1, "Pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "Paco");

    struct ircchan *channel = _irc_create_chan(irc, "#testchan", 2, a, b);

    irc_channel_addop(channel, a);

    output = _process_message(irc_kick, irc, 1, str);
    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NOSUCHCHANNEL , NULL);
    irc_testend;
}
int t_irc_oper__bad_password()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char str[] = "OPER Pepe bad_pass";
    _irc_register_withnick(irc, 1, "Pepe");

    dic_add(irc->oper_passwords, "Pepe", "password");

    output = _process_message(irc_oper, irc, 1, str);
    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_PASSWDMISMATCH, NULL);

    irc_testend;
}
int t_irc_oper__not_set()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char str[] = "OPER Pepe password";
    _irc_register_withnick(irc, 1, "Pepe");

    output = _process_message(irc_oper, irc, 1, str);
    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_PASSWDMISMATCH, NULL);

    irc_testend;
}
int t_irc_oper__set()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char str[] = "OPER Pepe password";
    _irc_register_withnick(irc, 1, "Pepe");

    dic_add(irc->oper_passwords, "Pepe", "password");
    output = _process_message(irc_oper, irc, 1, str);
    assert_generated(1);
    assert_numeric_reply(msgnum(0), RPL_YOUREOPER, NULL);

    irc_testend;
}
int t__irc_kill__killed()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char str[] = "KILL paco :por tolai";

    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    _irc_register_withnick(irc, 2, "paco");

    a->mode =  a->mode | user_op;


    output = _process_message(irc_kill, irc, 1, str );
    assert_generated(1);

    assert_msgstr_eq(msgnum(0), ":pepe KILL paco :por tolai");

    irc_testend;
}
int t__irc_kill__more_params()
{

    struct irc_globdata *irc = irc_init();
    list *output;

    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    _irc_register_withnick(irc, 2, "paco");

    a->mode =  a->mode | user_op;

    output = _process_message(irc_kill, irc, 1, "KILL oscar");
    assert_generated(1);

    assert_numeric_reply(msgnum(0), ERR_NEEDMOREPARAMS, NULL);
    irc_testend;
}
int t__irc_kill__no_such_nick()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char str[] = "KILL oscar :por tolai";

    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    _irc_register_withnick(irc, 2, "paco");

    a->mode =  a->mode | user_op;


    output = _process_message(irc_kill, irc, 1, str);
    assert_generated(1);

    assert_numeric_reply(msgnum(0), ERR_NOSUCHNICK, NULL);
    irc_testend;
}

int t__irc_kill__no_privileges()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char str[] = "KILL paco :por tolai";

    _irc_register_withnick(irc, 1, "pepe");
    _irc_register_withnick(irc, 2, "paco");

    output = _process_message(irc_kill, irc, 1, str );

    assert_generated(1);

    assert_numeric_reply(msgnum(0), ERR_NOPRIVILEGES, NULL);
    irc_testend;
}

int t_irc_topic__topic_change__topic_changed_and_broadcast()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "paco");
    struct ircchan *chan =  _irc_create_chan(irc, "#testchan", 2, a, b);
    char msg[] = "TOPIC #testchan :Tema de prueba";

    output = _process_message(irc_topic, irc, 1, msg);

    assert_generated(2);
    assert_dest(msgnum(0), a);
    assert_dest(msgnum(1), b);
    assert_msgstr_eq(msgnum(0), ":pepe TOPIC #testchan :Tema de prueba");
    assert_msgstr_eq(msgnum(1), ":pepe TOPIC #testchan :Tema de prueba");
    mu_assert_streq(chan->topic, "Tema de prueba", "Topic not set");

    irc_testend;
}
int t_irc_topic__op_needed__returns_needmoreprivileges()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "paco");
    struct ircchan *chan =  _irc_create_chan(irc, "#testchan", 2, a, b);
    char msg[] = "TOPIC #testchan :Tema de prueba";
    chan->mode |= chan_topiclock;

    output = _process_message(irc_topic, irc, 1, msg);
    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_CHANOPRIVSNEEDED, "#testchan");
    irc_testend;
}
int t_irc_topic__topic_notset_request__returns_notopic()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    _irc_create_chan(irc, "#testchan", 1, a);
    output = _process_message(irc_topic, irc, 1, "TOPIC #testchan");

    assert_generated(1);
    assert_numeric_reply(msgnum(0), RPL_NOTOPIC, "#testchan");
    irc_testend;
}
int t_irc_topic__topic_set_requested__returns_topic()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    struct ircchan *chan =  _irc_create_chan(irc, "#testchan", 1, a);
    strcpy(chan->topic, "Prueba");

    output = _process_message(irc_topic, irc, 1, "TOPIC #testchan");

    assert_generated(1);
    mu_assert("Message doesn't contain the topic", strstr(msgnum(0)->data, ":Prueba") != NULL);
    assert_numeric_reply(msgnum(0), RPL_TOPIC, "#testchan");

    irc_testend;
}
int t_irc_topic__no_params__returns_needmoreparams()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "paco");
    _irc_create_chan(irc, "#testchan", 2, a, b);

    output = _process_message(irc_topic, irc, 1, "TOPIC");
    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NEEDMOREPARAMS, "TOPIC");
    irc_testend;
}

int t_irc_user_bad_params()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char msg[] = "USER ignore ignorar :Guillermo de Juan";


    output = _process_message(irc_user, irc, 1, msg);
    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NEEDMOREPARAMS, NULL);

    irc_testend;
}

int t_irc_user()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    int fd = 1;
    char msg[] = "USER Ralph Ignore ign :comida de gato";
    struct ircuser *a = _irc_register_withnick(irc, fd, "Ralph");
    output = _process_message(irc_user, irc, fd, msg);

    mu_assert_streq(a->name, "comida de gato", "Name not set");

    assert_generated(0);

    irc_testend;
}

int t_irc_nick_collision()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    _irc_register_withnick(irc, 4, "pedro");
    _irc_register_withnick(irc, 5, "asdf");
    output = _process_message(irc_nick, irc, 5 , "NICK pedro");

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NICKCOLLISION, NULL);

    irc_testend;
}

int t_irc_nick__set_nick__set()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    struct ircuser *a = irc_register_user(irc, 1);

    output = _process_message(irc_nick, irc, 1, "NICK Juan");

    assert_generated(1);
    mu_assert_streq(a->nick, "Juan", "Nick was not set");

    irc_testend;
}

int t_irc_nick__change_nick__set_and_broadcast()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "paco");
    struct ircuser *b = _irc_register_withnick(irc, 2, "pepe");
    _irc_create_chan(irc, "#testchan", 2, a, b);

    output = _process_message(irc_nick, irc, 1, "NICK Juan");

    assert_generated(2);
    assert_msgstr_eq(msgnum(0), ":paco NICK Juan");
    mu_assert_streq(a->nick, "Juan", "Nick was not set");

    irc_testend;
}

int t_irc_privmsg__send_to_channel__sent_to_users_in_channel()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    struct ircuser *a = _irc_register_withnick(irc, 1, "pepe");
    struct ircuser *b = _irc_register_withnick(irc, 2, "paco");
    char msg[] = "PRIVMSG #testchan :todos";
    _irc_register_withnick(irc, 3, "luis");

    _irc_create_chan(irc, "#testchan", 2, a, b);

    output = _process_message(irc_privmsg, irc, 3, msg);

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
    list *output;
    char msg[] = "PRIVMSG pepe :hola :)";

    _irc_register_withnick(irc, 0, "paco");
    _irc_register_withnick(irc, 1, "pepe");

    output = _process_message(irc_privmsg, irc, 0, msg);

    assert_generated(1);
    assert_msgstr_eq(msgnum(0), ":paco PRIVMSG pepe :hola :)");

    irc_testend;
}

int t_irc_privmsg__no_text_provided__err_notexttosend()
{
    struct irc_globdata *irc = irc_init();
    list *output;

    output = _process_message(irc_privmsg, irc, 0, "PRIVMSG pepe");

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NOTEXTTOSEND, NULL);

    irc_testend;
}

int t_irc_privmsg__dest_not_provied__err_norecipient()
{
    struct irc_globdata *irc = irc_init();
    list *output;

    output = _process_message(irc_privmsg, irc, 0, "PRIVMSG");

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NORECIPIENT, NULL);

    irc_testend;
}

int t_irc_privmsg__dest_doesnt_exist__err_no_such_nick()
{
    struct irc_globdata *irc = irc_init();
    list *output;
    char msg[] =  "PRIVMSG pepe: hola!!!";
    output = _process_message(irc_privmsg, irc, 0, msg);

    assert_generated(1);
    assert_numeric_reply(msgnum(0), ERR_NOSUCHNICK, NULL);

    irc_testend;
}

int t_irc_privmsg__dest_away__replies_away()
{
    struct irc_globdata *irc = irc_init();
    struct ircuser *user = _irc_register_withnick(irc, 0, "pepe");
    list *output;
    char msg[] = "PRIVMSG pepe :hola";
    _irc_register_withnick(irc, 1, "luis");

    user->is_away = 1;
    strcpy(user->away_msg, "Estoy programando redes.");

    output = _process_message(irc_privmsg, irc, 1, msg);

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

    assert_generated(2);
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

    assert_generated(2);
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
	mu_run_test(t_irc_join__all_together);
	mu_run_test(t_irc_join__invited__not_inv);
	mu_run_test(t_irc_join__1_invited_ok);
	mu_run_test(t_irc_join__1_channel_pass__ok);
	mu_run_test(t_irc_join__1_channel_pass__badpass);
	mu_run_test(t_irc_join__more_channel_no_pass);
	mu_run_test(t_irc_join__1_channel_no_pass);
	mu_run_test(t_irc_join__bad_params);
	mu_run_test(t_irc_mode__user_provided__returns_user_mode);
	mu_run_test(t_irc_mode__different_user__err_match);
	mu_run_test(t_irc_mode__user_minus_o_op__removes_op_privs);
	mu_run_test(t_irc_mode__user_plus_o__ignored);
	mu_run_test(t_irc_mode__plus_k_no_op__err_privsneeded);
	mu_run_test(t_irc_mode__minus_k__removes_password);
	mu_run_test(t_irc_mode__plus_k_pass_provided__sets_password);
	mu_run_test(t_irc_mode__plus_k_no_pass_provided__err_moreparams);
	mu_run_test(t_irc_mode__minus_l__removes_user_limit);
	mu_run_test(t_irc_mode__plus_l__sets_user_limit);
	mu_run_test(t_irc_mode__plus_o_noop__err_chanopprivsneeded);
	mu_run_test(t_irc_mode__plus_o_op__gives_chanop_target);
	mu_run_test(t_irc_mode__no_target_provided__err_moreparams);
	mu_run_test(t_irc_user_bad_params);
    mu_run_test(t_irc_kick__you_are_not_in_channel);
    mu_run_test(t_irc_kick__kicked);
    mu_run_test(t_irc_kick__chan_op_priv_need);
    mu_run_test(t_irc_kick__not_in_channel);
    mu_run_test(t_irc_kick__no_such_channel);
    mu_run_test(t_irc_oper__bad_password);
    mu_run_test(t_irc_oper__not_set);
    mu_run_test(t_irc_oper__set);
    mu_run_test(t__irc_kill__killed);
    mu_run_test(t__irc_kill__more_params);
    mu_run_test(t__irc_kill__no_such_nick);
    mu_run_test(t__irc_kill__no_privileges);
    mu_run_test(t_irc_user_bad_params);
    mu_run_test(t_irc_user);
    mu_run_test(t_irc_nick_collision);
    mu_run_test(t_irc_nick__set_nick__set);
    mu_run_test(t_irc_nick__change_nick__set_and_broadcast);
    mu_run_test(t_irc_topic__topic_change__topic_changed_and_broadcast);
    mu_run_test(t_irc_topic__op_needed__returns_needmoreprivileges);
    mu_run_test(t_irc_topic__topic_notset_request__returns_notopic);
    mu_run_test(t_irc_topic__topic_set_requested__returns_topic);
    mu_run_test(t_irc_topic__no_params__returns_needmoreparams);
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

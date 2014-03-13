#ifndef IRC_TESTHELPER_H
#define IRC_TESTHELPER_H

#include "strings.h"
#include "irc_processor.h"
#include "types.h"
#include "irc_core.h"

/** 
 * Usamos macros para facilitar la creación de tests aunque se pierda un poco 
 *  de claridad
 */
#define assert_generated(msgnum) mu_assert_eq(list_count(output), msgnum, "Incorrect number of generated messages.")
#define msgnum(num) ((struct sockcomm_data*) list_at(output, num))
#define irc_testend irc_destroy(irc); list_destroy(output, free); mu_end
#define assert_msgstr_eq(msg, str) mu_assert_streq(_remove_trailing_crlf(msg->data), str, "Message string doesn't correspond")
#define assert_numeric_reply(msg, expected_reply, params) do { if(_assert_numeric_reply(msg, expected_reply, params) != MU_PASSED) return MU_ERR; } while(0)
#define assert_dest(msg, dest) mu_assert_eq(msg->fd, dest->fd, "Destinatary is not the expected.")

int _assert_numeric_reply(struct sockcomm_data *msg, int expected_reply, const char* additional_params);

list *_process_message(cmd_action action, struct irc_globdata *gdata, int fd, char *message);

struct ircchan* _irc_create_chan(struct irc_globdata* irc, const char* name, int usernum, ...);
struct ircuser* _irc_register_withnick(struct irc_globdata* irc, int id, const char* nick);
char* _remove_trailing_crlf(char* str);
#endif

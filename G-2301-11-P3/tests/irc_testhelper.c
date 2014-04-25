#include "irc_testhelper.h"
#include "testmacros.h"
#include "irc_processor.h"
#include "types.h"
#include "irc_core.h"
#include "strings.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


int _assert_numeric_reply_text(struct sockcomm_data *msg, int expected_reply, const char* additional_params, const char* text)
{
	char servname[MAX_SERVER_NAME];
	int actual_reply;
	char param[MAX_IRC_MSG];
	char* param_start;
	char user[MAX_NICK_LEN];
	char* colon;
	char err_msg[MAX_IRC_MSG + 30];

	snprintf(err_msg, MAX_IRC_MSG + 30, "Error verifying response %s", _remove_trailing_crlf(msg->data));
	sscanf(msg->data, ":%s %d %s %s", servname, &actual_reply, user, param);
	param_start = strnstr(msg->data, param, MAX_IRC_MSG);

	colon = strchr(param_start, ':');

	if(!colon)
	{
		if(text)
			mu_fail(err_msg);
	}
	else
	{
		colon--;
		*colon = '\0';
		colon += 2;
	}

	mu_assert_eq(actual_reply, expected_reply, err_msg);

	if(additional_params)
		mu_assert_streq(param_start, additional_params, err_msg);

	if(text)
		mu_assert_streq(_remove_trailing_crlf(colon), text, err_msg);

    return MU_PASSED;
}

int _assert_numeric_reply(struct sockcomm_data *msg, int expected_reply, const char* additional_params)
{
	return _assert_numeric_reply_text(msg, expected_reply, additional_params, NULL);
}

list *_process_message(cmd_action action, struct irc_globdata *gdata, int fd, char *message)
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

struct ircchan* _irc_create_chan(struct irc_globdata* irc, const char* name, int usernum, ...)
{
	int i;
	va_list ap;
	struct ircuser* user;
	struct ircchan* chan = irc_register_channel(irc, name);

	va_start(ap, usernum);
	for(i = 0; i < usernum; i++)
	{
		user = va_arg(ap, struct ircuser*);
		irc_channel_adduser(irc, chan, user, NULL);
	}

	va_end(ap);

	return chan;
}


struct ircuser* _irc_register_withnick(struct irc_globdata* irc, int id, const char* nick)
{
	struct ircuser* user = irc_register_user(irc, id);
	irc_set_usernick(irc, id, nick);
	return user;
}

char* _remove_trailing_crlf(char* str)
{
	char* crlf = strnstr(str, "\r\n", MAX_IRC_MSG);

	if(crlf != NULL)
		*crlf = '\0';

	return str;
}

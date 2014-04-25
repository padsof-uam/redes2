#include "G-2301-11-P3-commparser.h"
#include <string.h>

static int _is_wildcard(const char* str)
{
	return str != NULL && strlen(str) == 1 && str[0] == '*';
}

int parse_command(const char* str, const char** commands, int len)
{
	int i;

	if(!str)
		return -1;

	for(i = 0; i < len; i++)
		if(_is_wildcard(commands[i]) || !strncasecmp(str, commands[i], strlen(commands[i])))
			return i;

	if (!strcmp(commands[len-1],"*"))
		return len-1;
	
	return -1;
}

int parse_exec_command(const char* str, const char** commands, const cmd_action* actions, int len, void* data)
{
	int cmd;
	cmd_action func;
	cmd = parse_command(str, commands, len);

	if(cmd == -1)
		return -1;

	func = actions[cmd];

	if(!func)
		return -1;

	return func(data);
}


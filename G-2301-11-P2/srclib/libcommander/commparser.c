#include "commparser.h"
#include <string.h>

int parse_command(const char* str, const char** commands, int len)
{
	int i;

	if(!str)
		return -1;

	for(i = 0; i < len; i++)
		if(!strncasecmp(str, commands[i], strlen(commands[i])))
			return i;
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


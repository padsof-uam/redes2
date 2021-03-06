#ifndef COMMPARSER_H
#define COMMPARSER_H

typedef int (*cmd_action)(void*);

int parse_command(const char* str, const char** commands, int len);
int parse_exec_command(const char* str, const char** commands, const cmd_action* actions, int len, void* data);

#endif


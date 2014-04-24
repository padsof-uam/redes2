#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <stdlib.h>

typedef enum {
	arg_required = 1,
	arg_req_param = 2,
	arg_opt_param = 4
} arg_type;

struct arginfo {
	char shortopt;
	const char* longopt;
	arg_type type;
	const char* help;
	const char* argname;
	short present;
	const char* param;
};

int argp_parse(int argc, const char** argv, struct arginfo* info, size_t arglen);
void argp_usage(struct arginfo* info, size_t arglen);

#endif

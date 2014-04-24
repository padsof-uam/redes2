#include "argparser.h"
#include "termcolor.h"

#include <stdio.h>
#include <string.h>

extern const char* _argp_progname;

#define opt_noflagparam(opt) ((opt)->shortopt == 0 && (opt)->longopt==NULL)

static int _insert_noflag_param(const char *param, struct arginfo *info, size_t arglen)
{
    int i;
    for (i = 0; i < arglen; i++)
    {
        if (info[i].longopt == NULL && info[i].shortopt == 0 && !info[i].present)
            break;
    }

    if (i == arglen)
        return -1;

    info[i].present = 1;
    info[i].param = param;

    return i;
}

static void _unrecognized_arg_exit(const char* arg, struct arginfo* info, size_t arglen)
{
	printf("Unrecognized option: %s\n", arg);
                argp_usage(info, arglen);
                exit(EXIT_FAILURE);	
}

static void _no_param_present_exit(const char* arg, struct arginfo* info, size_t arglen)
{
	printf("Option %s needs a parameter.\n", arg);
                argp_usage(info, arglen);
                exit(EXIT_FAILURE);		
}

static struct arginfo* _find_shortopt(const char* opt, struct arginfo* info, size_t arglen)
{
	for(int i = 0; i < arglen; i++)
	{
		if(opt[1] == info[i].shortopt)
			return info + i;
	}

	return NULL;
}

static struct arginfo* _find_longopt(const char* opt, struct arginfo* info, size_t arglen)
{
	for(int i = 0; i < arglen; i++)
	{
		if(info[i].longopt != NULL && !strncmp(opt + 2, info[i].longopt, strlen(opt) - 2))
			return info + i;
	}

	return NULL;
}

int argp_parse(int argc, const char **argv, struct arginfo *info, size_t arglen)
{
    int i;
    const char *arg;
    short optend = 0;
    struct arginfo* opt;
    int parsed_params = 0;

    for (i = 0; i < arglen; i++)
    {
        info[i].param = NULL;
        info[i].present = 0;
    }

    for (i = 1; i < argc; i++)
    {
        arg = argv[i];

        if(!strcmp("-h", arg) || !strcmp("--help", arg))
        {
            argp_usage(info, arglen);
            exit(EXIT_SUCCESS);
        }

        if (arg[0] != '-' || optend)
        {
            if (_insert_noflag_param(arg, info, arglen) == -1)
            	_unrecognized_arg_exit(arg, info, arglen);

            parsed_params++;
        }
        else
        {
        	if(arg[1] != '-') // -shortflag
        	{
	        	opt = _find_shortopt(arg, info, arglen);
	        }
	        else if(arg[2] == '\0') // -- (end of options list)
	        {
	        	optend = 1;
	        	continue;
	        }
	        else // --longflag
	        {
	        	opt = _find_longopt(arg, info, arglen);
	        }

        	if(opt == NULL)
        		_unrecognized_arg_exit(arg, info, arglen);

        	opt->present = 1;

        	if(opt->type & arg_req_param)
        	{
        		i++;

        		if(i == argc)
        			_no_param_present_exit(arg, info, arglen);

        		opt->param = argv[i];
        	}
        	else if(opt->type & arg_opt_param)
        	{
        		if(i + 1 < argc && argv[i + 1][0] != '-')
        		{
        			i++;
        			opt->param = argv[i];
        		}
        	}

        	parsed_params++;
        }
    }

    for(i = 0; i < arglen; i++)
    {
    	if((info[i].type & arg_required) && !info[i].present)
    	{
    		if(info[i].longopt != NULL)
    			arg = info[i].longopt;
    		else if(info[i].argname != NULL)
    			arg = info[i].argname;
    		else 
    			arg = "";

    		printf("Required argument %s is not provided.\n", arg);
    		argp_usage(info, arglen);
    		exit(EXIT_FAILURE);
    	}
    }

    return parsed_params;
}

void argp_usage(struct arginfo *info, size_t arglen)
{
    struct arginfo *arg;
    size_t i;

    printf("Usage: %s [options]", _argp_progname);

    for(i = 0; i < arglen; i++)
    {
    	if(info[i].longopt == NULL && info[i].shortopt == 0)
    	{
    		if(info[i].type & arg_required)
    			printf(" %s", info[i].argname);
    		else
    			printf(" [%s]", info[i].argname);
    	}
    }

    printf("\n\n");

    for (i = 0; i < arglen; i++)
    {
        arg = info + i;
        printf("\t");

        if (arg->longopt != NULL)
        {
            printf("--%s", arg->longopt);
            if (arg->shortopt != 0)
                printf(", ");
            else
                printf(" ");
        }

        if (arg->shortopt != 0)
            printf("-%c ", arg->shortopt);

        if (arg->type & arg_opt_param || 
            (!(arg->type & arg_required) && opt_noflagparam(arg)))
        {
            if (arg->argname)
                printf("[%s]", arg->argname);
            else
                printf("[optarg]");
        }

        if (arg->type & arg_req_param || 
            ((arg->type & arg_required) && opt_noflagparam(arg)))
        {
            if (arg->argname)
                printf("<%s>", arg->argname);
            else
                printf("<arg>");
        }

        if (arg->type & arg_required)
            printf("\t"TBOLD "Required." TRESET);

        printf("\n");

        if (arg->help != NULL)
            printf("\t\t%s\n", arg->help);
    }

    printf("\t--help,-h\n\t\tPrint this help message.\n\n");
}

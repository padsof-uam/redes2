#include "log.h"

#include <stdio.h>

FILE *_out = NULL;
int _level = LOG_INFO;

void slog_set_output(FILE *f)
{
    _out = f;
}

void slog_set_output_syslog()
{
    _out = NULL;
}

void slog_set_level(int level)
{
    _level = level;
}

void slog_debug(const char* format, ...)
{
    /* This function disappears from code when compiled without debug flag */
    #ifdef DEBUG
    va_list ap;
    va_start(ap, format);

    vslog(LOG_DEBUG, format, ap);
    va_end(ap);
    #endif
}

void vslog(int level, const char* format, va_list ap)
{
    if(level > _level)
        return;


    if (_out != NULL)
    {
        vfprintf(_out, format, ap);
        fprintf(_out, "\n");
    }
    else
    {
        vsyslog(level, format, ap);
    }    
}

void slog(int level, const char *str, ...)
{
    va_list ap;

    va_start(ap, str);
    vslog(level, str, ap);
    va_end(ap);
}


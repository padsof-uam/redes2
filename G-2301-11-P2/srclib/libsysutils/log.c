#include "log.h"

#include <stdio.h>
#include <pthread.h>
#include <time.h>

FILE *_out = NULL;
int _level = LOG_INFO;
pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

static const char *_log_level_str(int level)
{
    switch (level)
    {
    case LOG_EMERG:
        return "[EMERG]";
    case LOG_ALERT:
        return "[ALERT]";
    case LOG_CRIT:
        return "[CRIT]";
    case LOG_ERR:
        return "[ERR]";
    case LOG_WARNING:
        return "[WARNING]";
    case LOG_NOTICE:
        return "[NOTICE]";
    case LOG_INFO:
        return "[INFO]";
    case LOG_DEBUG:
        return "[DEBUG]";
    default:
        return "[UNKNOWN]";
    }
}

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

void slog_debug(const char *format, ...)
{
    /* This function disappears from code when compiled without debug flag */
#ifdef DEBUG
    va_list ap;
    va_start(ap, format);

    vslog(LOG_DEBUG, format, ap);
    va_end(ap);
#endif
}

void vslog(int level, const char *format, va_list ap)
{
    char time_buf[100];
    time_t rawtime;
    struct tm *timeinfo;


    if (level > _level)
        return;


    if (_out != NULL)
    {
        time (&rawtime);
        timeinfo = localtime (&rawtime);
        strftime(time_buf, 100, "%h %d, %T", timeinfo);

        pthread_mutex_lock(&_mutex);
        {
            fprintf(_out, "%s %s: ", time_buf, _log_level_str(level));
            vfprintf(_out, format, ap);
            fprintf(_out, "\n");
        }
        pthread_mutex_unlock(&_mutex);
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


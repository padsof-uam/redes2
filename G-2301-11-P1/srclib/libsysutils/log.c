#include "log.h"
#include <syslog.h>

#include <stdio.h>
#include <stdarg.h>

void slog(int level, const char* str, ...)
{
	va_list ap;
	va_start(ap, str);
#ifdef NODAEMON
	vfprintf(stderr, str, ap);
	fprintf(stderr, "\n");
#else
	vsyslog(level, str, ap);
#endif
	va_end(ap);
}


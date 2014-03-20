#include <syslog.h>

#ifndef SYSLOG_H
#define SYSLOG_H

void slog(int level, const char* str, ...);

#endif

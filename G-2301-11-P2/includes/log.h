#ifndef LOG_H
#define LOG_H

#include <sys/syslog.h>
#include <stdarg.h>
#include <stdio.h>

void slog_set_output(FILE *f);
void slog_set_output_syslog();
void slog_set_level(int level);
void slog(int log_type, const char* format, ...);
void slog_debug(const char* format, ...);
void vslog(int level, const char* format, va_list ap);
#endif


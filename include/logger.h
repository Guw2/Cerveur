#ifndef CERVEUR_LOGGER_H
#define CERVEUR_LOGGER_H

#include <stdbool.h>

typedef enum{
	LOG_LEVEL_DEBUG = 0,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR
} LogLevel;

void logger_set_level(LogLevel level);
void logger_set_colors(bool enabled);

void logger_debug(const char *fmt, ...);
void logger_info(const char *fmt, ...);
void logger_warn(const char *fmt, ...);
void logger_error(const char *fmt, ...);

#endif

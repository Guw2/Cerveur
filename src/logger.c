#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

static LogLevel current_level = LOG_LEVEL_INFO;
static bool colors_enabled = true;

static const char *level_to_string(LogLevel level){
    switch (level) {
        case LOG_LEVEL_DEBUG:
            return "DEBUG";
        case LOG_LEVEL_INFO:
            return "INFO";
        case LOG_LEVEL_WARN:
            return "WARN";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

static const char *level_to_color(LogLevel level){

    if(!colors_enabled){
        return "";
    }

    switch (level) {
        case LOG_LEVEL_DEBUG:
            return "\033[90m";
        case LOG_LEVEL_INFO:
            return "\033[32m";
        case LOG_LEVEL_WARN:
            return "\033[33m";
        case LOG_LEVEL_ERROR:
            return "\033[31m";
        default:
            return "";
    }
}

static void logger_log(LogLevel level, const char *fmt, va_list args){
    if(level < current_level){
        return;
    }

    const char *color = level_to_color(level);
    const char *reset = colors_enabled ? "\033[0m" : "";

    fprintf(stderr, "%s[%-5s]%s ", color, level_to_string(level), reset);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

void logger_set_level(LogLevel level) {
    current_level = level;
}

void logger_set_colors(bool enabled) {
    colors_enabled = enabled;
}

void logger_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logger_log(LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
}

void logger_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logger_log(LOG_LEVEL_INFO, fmt, args);
    va_end(args);
}

void logger_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logger_log(LOG_LEVEL_WARN, fmt, args);
    va_end(args);
}

void logger_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logger_log(LOG_LEVEL_ERROR, fmt, args);
    va_end(args);
}
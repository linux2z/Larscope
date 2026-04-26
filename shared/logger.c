/*
 * Larscope — Logger Implementation
 * Outputs to stderr + optional log file.
 */
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

static ls_log_level_t g_min_level = LS_LOG_INFO;
static FILE          *g_log_file  = NULL;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char *level_str[] = {
    "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"
};

static const char *level_color[] = {
    "\033[36m",   /* DEBUG: cyan */
    "\033[32m",   /* INFO:  green */
    "\033[33m",   /* WARN:  yellow */
    "\033[31m",   /* ERROR: red */
    "\033[35m",   /* FATAL: magenta */
};

int ls_logger_init(ls_log_level_t level, const char *log_file) {
    g_min_level = level;
    if (log_file) {
        g_log_file = fopen(log_file, "a");
        if (!g_log_file) {
            fprintf(stderr, "[LOGGER] Failed to open log file: %s\n", log_file);
            return -1;
        }
    }
    return 0;
}

void ls_logger_destroy(void) {
    pthread_mutex_lock(&g_log_mutex);
    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
    pthread_mutex_unlock(&g_log_mutex);
}

void ls_logger_set_level(ls_log_level_t level) {
    g_min_level = level;
}

void ls_log(ls_log_level_t level, const char *module, const char *fmt, ...) {
    if (level < g_min_level) return;

    /* Timestamp */
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tm_buf;
    localtime_r(&ts.tv_sec, &tm_buf);

    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &tm_buf);

    /* Format user message */
    char msg_buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, ap);
    va_end(ap);

    pthread_mutex_lock(&g_log_mutex);

    /* stderr with color */
    fprintf(stderr, "%s%s.%03ld [%s] %-12s\033[0m %s\n",
            level_color[level],
            time_buf, ts.tv_nsec / 1000000,
            level_str[level],
            module ? module : "system",
            msg_buf);

    /* Log file without color */
    if (g_log_file) {
        fprintf(g_log_file, "%s.%03ld [%s] %-12s %s\n",
                time_buf, ts.tv_nsec / 1000000,
                level_str[level],
                module ? module : "system",
                msg_buf);
        fflush(g_log_file);
    }

    pthread_mutex_unlock(&g_log_mutex);

    if (level == LS_LOG_FATAL) {
        abort();
    }
}

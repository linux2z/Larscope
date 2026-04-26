/*
 * Larscope — Logger
 * Per-module logging with level filtering.
 */
#ifndef LS_LOGGER_H
#define LS_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LS_LOG_DEBUG = 0,
    LS_LOG_INFO,
    LS_LOG_WARN,
    LS_LOG_ERROR,
    LS_LOG_FATAL
} ls_log_level_t;

/**
 * Initialize logger. Optionally log to a file.
 * @param level    Minimum log level to output.
 * @param log_file Path to log file, or NULL for stdout/syslog only.
 */
int  ls_logger_init(ls_log_level_t level, const char *log_file);
void ls_logger_destroy(void);

/**
 * Set minimum log level at runtime.
 */
void ls_logger_set_level(ls_log_level_t level);

/**
 * Log a message.
 * @param level  Log level.
 * @param module Module name (e.g. "camera", "gpio_input").
 * @param fmt    printf-style format string.
 */
void ls_log(ls_log_level_t level, const char *module, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

#ifdef __cplusplus
}
#endif

#endif /* LS_LOGGER_H */

// include/logger.h
#ifndef LOGGER_H
#define LOGGER_H

typedef enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR } log_level_t;

// Initialize logger with file path
int logger_init(const char *log_file);

// Log message with level
void logger_log(log_level_t level, const char *format, ...);

// Close logger
void logger_close(void);

#endif

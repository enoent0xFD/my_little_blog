#include "../include/logger.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char *level_strings[] = {"DEBUG", "INFO", "WARN", "ERROR"};

int logger_init(const char *filepath) {
  log_file = fopen(filepath, "a");
  return log_file != NULL;
}

void logger_log(log_level_t level, const char *format, ...) {
  if (!log_file)
    return;

  time_t now;
  time(&now);
  char timestamp[32];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

  pthread_mutex_lock(&log_mutex);

  fprintf(log_file, "[%s] [%s] ", timestamp, level_strings[level]);

  va_list args;
  va_start(args, format);
  vfprintf(log_file, format, args);
  va_end(args);

  fprintf(log_file, "\n");
  fflush(log_file);

  pthread_mutex_unlock(&log_mutex);
}

void logger_close(void) {
  if (log_file) {
    fclose(log_file);
    log_file = NULL;
  }
}

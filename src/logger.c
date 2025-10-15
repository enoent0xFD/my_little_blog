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
  if (filepath == NULL || filepath[0] == '\0') {
    log_file = NULL;
    return 1;
  }

  FILE *file = fopen(filepath, "a");
  if (!file) {
    log_file = NULL;
    return 0;
  }

  log_file = file;
  return 1;
}

void logger_log(log_level_t level, const char *format, ...) {
  time_t now;
  time(&now);
  char timestamp[32];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

  pthread_mutex_lock(&log_mutex);

  va_list args;
  va_start(args, format);

  if (log_file && log_file != stdout) {
    va_list file_args;
    va_copy(file_args, args);
    fprintf(log_file, "[%s] [%s] ", timestamp, level_strings[level]);
    vfprintf(log_file, format, file_args);
    fprintf(log_file, "\n");
    fflush(log_file);
    va_end(file_args);
  }

  va_list stdout_args;
  va_copy(stdout_args, args);
  fprintf(stdout, "[%s] [%s] ", timestamp, level_strings[level]);
  vfprintf(stdout, format, stdout_args);
  fprintf(stdout, "\n");
  fflush(stdout);
  va_end(stdout_args);

  va_end(args);

  pthread_mutex_unlock(&log_mutex);
}

void logger_close(void) {
  if (log_file && log_file != stdout && log_file != stderr) {
    fclose(log_file);
  }
  log_file = NULL;
}

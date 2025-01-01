// src/security.c
#include "../include/security.h"
#include "../include/logger.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Rate limiting implementation
struct rate_limit_entry {
  char ip[46]; // IPv6 max length
  time_t window_start;
  int request_count;
};

static struct rate_limit_entry rate_limits[MAX_IPS];
static int rate_limit_count = 0;

bool is_path_safe(const char *path) {
  // Only check for obvious path traversal attempts
  if (strstr(path, "..") != NULL) {
    logger_log(LOG_WARN, "Path traversal attempt detected: %s", path);
    return false;
  }

  // Allow normal characters commonly used in URLs
  const char *c = path;
  while (*c) {
    if (!isalnum(*c) && *c != '/' && *c != '-' && *c != '_' && *c != '.' &&
        *c != ' ' && *c != '+') {
      logger_log(LOG_WARN, "Invalid character in path: %c", *c);
      return false;
    }
    c++;
  }

  return true;
}

bool check_rate_limit(const char *ip) {
  time_t now = time(NULL);

  // Clean old entries
  for (int i = 0; i < rate_limit_count; i++) {
    if (now - rate_limits[i].window_start >= RATE_LIMIT_WINDOW) {
      if (i < rate_limit_count - 1) {
        memmove(&rate_limits[i], &rate_limits[i + 1],
                sizeof(struct rate_limit_entry) * (rate_limit_count - i - 1));
      }
      rate_limit_count--;
      i--;
    }
  }

  // Find or create entry for IP
  int index = -1;
  for (int i = 0; i < rate_limit_count; i++) {
    if (strcmp(rate_limits[i].ip, ip) == 0) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    if (rate_limit_count >= MAX_IPS) {
      logger_log(LOG_WARN, "Rate limit entries full, rejecting: %s", ip);
      return false;
    }
    index = rate_limit_count++;
    strncpy(rate_limits[index].ip, ip, sizeof(rate_limits[index].ip) - 1);
    rate_limits[index].window_start = now;
    rate_limits[index].request_count = 1;
    return true;
  }

  // Check and update existing entry
  if (rate_limits[index].request_count >= MAX_REQUESTS) {
    logger_log(LOG_WARN, "Rate limit exceeded for IP: %s", ip);
    return false;
  }

  rate_limits[index].request_count++;
  return true;
}

bool is_request_valid(const char *method, const char *path,
                      size_t content_length) {
  // Only allow GET requests for now
  if (strcmp(method, "GET") != 0) {
    logger_log(LOG_WARN, "Invalid method: %s", method);
    return false;
  }

  // Check path length
  if (strlen(path) > 255) {
    logger_log(LOG_WARN, "Path too long: %s", path);
    return false;
  }

  // Check content length
  if (content_length > 1024 * 1024) { // 1MB limit
    logger_log(LOG_WARN, "Content length too large: %zu", content_length);
    return false;
  }

  return true;
}

char *sanitize_path(const char *original_path) {
  char *safe_path = malloc(strlen(original_path) + 1);
  char *write_ptr = safe_path;

  // Remove consecutive slashes and normalize path
  const char *read_ptr = original_path;
  bool last_was_slash = false;

  while (*read_ptr) {
    if (*read_ptr == '/') {
      if (!last_was_slash) {
        *write_ptr++ = '/';
      }
      last_was_slash = true;
    } else {
      *write_ptr++ = *read_ptr;
      last_was_slash = false;
    }
    read_ptr++;
  }

  *write_ptr = '\0';
  return safe_path;
}

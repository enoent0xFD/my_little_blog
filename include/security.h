// include/security.h
#ifndef SECURITY_H
#define SECURITY_H

#include <stdbool.h>
#include <time.h>

// Rate limiting structure
#define MAX_IPS 1000
#define RATE_LIMIT_WINDOW 60 // seconds
#define MAX_REQUESTS 100     // requests per window

// Path validation
bool is_path_safe(const char *path);

// Rate limiting
bool check_rate_limit(const char *ip);

// Request validation
bool is_request_valid(const char *method, const char *path,
                      size_t content_length);

// Sanitize file path
char *sanitize_path(const char *original_path);

#endif

#include "../include/http.h"
#include <stdio.h>
#include <string.h>

void parse_http_request(char *request_line, struct http_request *req) {
  sscanf(request_line, "%s %s %s", req->method, req->path, req->version);
}

const char *get_content_type(const char *path) {
  const char *ext = strrchr(path, '.');
  if (ext) {
    if (strcmp(ext, ".html") == 0) {
      return "text/html";
    } else if (strcmp(ext, ".css") == 0) {
      return "text/css";
    } else if (strcmp(ext, ".js") == 0) {
      return "application/javascript";
    } else if (strcmp(ext, ".json") == 0) {
      return "application/json";
    }
  }
  return "text/plain";
}

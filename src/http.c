#include "../include/http.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
}// In src/http.c
void send_404(int client_fd) {
    const char* body = "404 Not Found";
    char headers[512];
    snprintf(headers, sizeof(headers),
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %lu\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            strlen(body), body);
    
    write(client_fd, headers, strlen(headers));
}

void send_500(int client_fd) {
    const char* body = "500 Internal Server Error";
    char headers[512];
    snprintf(headers, sizeof(headers),
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %lu\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            strlen(body), body);
    
    write(client_fd, headers, strlen(headers));
}

void send_response(int client_fd, int status_code, const char* status_text, 
                  const char* content_type, const char* body) {
    char headers[512];
    snprintf(headers, sizeof(headers),
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %lu\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            status_code, status_text, content_type, strlen(body), body);
    
    write(client_fd, headers, strlen(headers));
}

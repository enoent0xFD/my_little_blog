#include <stdbool.h>  // Add this for bool type

#ifndef HTTP_H
#define HTTP_H

struct http_request {
    char method[16];
    char path[256];
    char version[16];
};

bool parse_http_request(char* request_line, struct http_request* req);
const char* get_content_type(const char* path);
// In include/http.h
// Add these function declarations:
void send_404(int client_fd);
void send_500(int client_fd);
void send_response(int client_fd, int status_code, const char* status_text, 
                  const char* content_type, const char* body);
#endif

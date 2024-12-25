#ifndef HTTP_H
#define HTTP_H

struct http_request {
    char method[16];
    char path[256];
    char version[16];
};

void parse_http_request(char* request_line, struct http_request* req);
const char* get_content_type(const char* path);

#endif

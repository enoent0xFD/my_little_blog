// src/error_pages.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/error_pages.h"

const char* ERROR_TEMPLATE = 
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "    <meta charset=\"UTF-8\">\n"
    "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    "    <title>Error %d - Abstraction Layer</title>\n"
    "    <link rel=\"stylesheet\" href=\"/styles.css\">\n"
    "</head>\n"
    "<body>\n"
    "    <header>\n"
    "        <h1 class=\"site-title\">Abstraction Layer</h1>\n"
    "    </header>\n"
    "    <div class=\"content error-content\">\n"
    "        <h2>Error %d</h2>\n"
    "        <p>%s</p>\n"
    "        <p><a href=\"/\">Return to homepage</a></p>\n"
    "    </div>\n"
    "    <div class=\"status-bar\" id=\"status-bar\">\n"
    "        Status: Error | Powered by C\n"
    "    </div>\n"
    "</body>\n"
    "</html>";

void send_error_page(int client_fd, int status_code, const char* message) {
    char body[4096];
    snprintf(body, sizeof(body), ERROR_TEMPLATE, 
             status_code, status_code, message);

    char headers[512];
    const char* status_text = 
        status_code == 404 ? "Not Found" :
        status_code == 400 ? "Bad Request" :
        status_code == 500 ? "Internal Server Error" : "Error";

    snprintf(headers, sizeof(headers),
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %lu\r\n"
            "Connection: close\r\n"
            "\r\n",
            status_code, status_text, strlen(body));

    write(client_fd, headers, strlen(headers));
    write(client_fd, body, strlen(body));
}

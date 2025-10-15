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
    "    <title>Error %d · Filip Mihalic</title>\n"
    "    <script src=\"https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4\"></script>\n"
    "    <style>:root { color-scheme: dark; }</style>\n"
    "</head>\n"
    "<body class=\"bg-slate-950 text-slate-200 font-sans min-h-screen\">\n"
    "  <div class=\"max-w-2xl mx-auto px-6 py-20 flex flex-col gap-14\">\n"
    "    <header class=\"flex flex-col gap-4\">\n"
    "      <a href=\"/\" class=\"text-3xl font-semibold text-slate-100\">Filip Mihalic</a>\n"
    "      <p class=\"text-slate-500 text-sm\">Something went sideways. Let's get you back on track.</p>\n"
    "    </header>\n"
    "    <main class=\"rounded-2xl border border-slate-800/80 bg-slate-900/40 p-10\">\n"
    "      <p class=\"text-xs uppercase tracking-[0.3em] text-slate-500\">Error</p>\n"
    "      <h1 class=\"mt-4 text-4xl font-semibold text-slate-100\">%d</h1>\n"
    "      <p class=\"mt-6 text-slate-300\">%s</p>\n"
    "      <a href=\"/\" class=\"mt-8 inline-flex items-center text-sm text-blue-300 hover:text-blue-200\">Return to the homepage</a>\n"
    "    </main>\n"
    "  </div>\n"
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

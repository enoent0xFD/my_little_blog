#include "../include/server.h"
#include "../include/config.h"
#include "../include/error_pages.h"
#include "../include/http.h"
#include "../include/logger.h"
#include "../include/post.h"
#include "../include/security.h"
#include "../include/stats.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *ABOUT_TEMPLATE =
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "  <meta charset=\"UTF-8\">\n"
    "  <meta name=\"viewport\" content=\"width=device-width, "
    "initial-scale=1.0\">\n"
    "  <title>About · Filip Mihalic</title>\n"
    "  <meta name=\"description\" content=\"About Filip Mihalic, a "
    "systems-focused software developer based in Belgrade, Serbia.\">\n"
    "  <link rel=\"icon\" type=\"image/x-icon\" href=\"/favicon.ico\">\n"
    "  <link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" "
    "href=\"/favicon-32x32.png\">\n"
    "  <link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" "
    "href=\"/favicon-16x16.png\">\n"
    "  <link rel=\"apple-touch-icon\" sizes=\"180x180\" "
    "href=\"/apple-touch-icon.png\">\n"
    "  <script "
    "src=\"https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4\"></script>\n"
    "  <style>\n"
    "    :root { color-scheme: dark; }\n"
    "    .markdown :is(p, ul, ol, blockquote, pre) { margin-top: 1.5rem; "
    "margin-bottom: 1.5rem; }\n"
    "    .markdown a { color: #60a5fa; text-decoration: none; }\n"
    "    .markdown a:hover { text-decoration: underline; }\n"
    "  </style>\n"
    "</head>\n"
    "<body class=\"bg-slate-950 text-slate-200 font-sans\">\n"
    "  <div class=\"min-h-screen\">\n"
    "    <header class=\"border-b border-slate-800/80 bg-slate-950/80 "
    "backdrop-blur\">\n"
    "      <div class=\"max-w-3xl mx-auto flex flex-col gap-6 px-6 py-12\">\n"
    "        <div class=\"flex flex-col gap-6 md:flex-row md:items-center "
    "md:justify-between\">\n"
    "          <a href=\"/\" class=\"text-3xl font-semibold "
    "text-slate-100\">Filip Mihalic</a>\n"
    "          <nav class=\"flex items-center gap-6 text-sm\">\n"
    "            <a href=\"/\" class=\"text-slate-400 "
    "hover:text-slate-200\">Blog</a>\n"
    "            <a href=\"/about\" class=\"text-slate-100 "
    "font-medium\">About</a>\n"
    "          </nav>\n"
    "        </div>\n"
    "        <div class=\"flex flex-col gap-6 md:flex-row md:items-center "
    "md:gap-10\">\n"
    "          <img src=\"/images/filip.webp\" alt=\"Portrait of Filip "
    "Mihalic\" class=\"h-28 w-28 rounded-full border border-slate-800 "
    "object-cover shadow-lg\" loading=\"lazy\">\n"
    "          <p class=\"max-w-2xl text-base text-slate-400\">Personal notes, "
    "lessons, and observations.</p>\n"
    "        </div>\n"
    "      </div>\n"
    "    </header>\n"
    "    <main class=\"max-w-3xl mx-auto px-6 py-12\">\n"
    "      <section class=\"flex flex-col gap-6\">\n"
    "        <h1 class=\"text-4xl font-semibold text-slate-50\">Filip "
    "Mihalic</h1>\n"
    "        <div class=\"markdown text-lg leading-relaxed text-slate-200\">\n"
    "          <p>Hey! I'm Filip (he/him). I'm a software developer based in "
    "Belgrade, Serbia.</p>\n"
    "          <p>For the last 10+ years, I've been working my way down the "
    "software stack&mdash;starting with LAMP stacks and jQuery, moving through "
    "the web with Node.js and TypeScript, and now betting on Rust and diving "
    "deeper into systems.</p>\n"
    "          <p>I've spent most of my career wrestling with software of all "
    "kinds and sizes&mdash;monoliths, microservices, distributed monoliths, "
    "\"microservice death stars,\" and strangler figs that strangled "
    "themselves. I've also worked on legacy systems that were difficult to "
    "maintain and update.</p>\n"
    "          <p>After a decade with high-level languages, I've realized we "
    "take many concepts for granted, and that software development is becoming "
    "more and more complex, while at the same time the quality of shipped "
    "software is declining.</p>\n"
    "          <p>These days, I'm writing down my thoughts and "
    "observations&mdash;documenting my journey as I learn new things and ship "
    "software. From time to time, I'll share opinions on the topics I'm "
    "currently working on.</p>\n"
    "          <p>I hope you connect with me on <a "
    "href=\"https://www.linkedin.com/in/filip-mihalic\" target=\"_blank\" "
    "rel=\"noopener\">LinkedIn</a>, <a href=\"https://github.com/enoent0xFD\" "
    "target=\"_blank\" rel=\"noopener\">GitHub</a>, or over <a "
    "href=\"mailto:filipm@hey.com\">email</a>.</p>\n"
    "          <hr class=\"my-10 border-slate-800/80\">\n"
    "          <hr class=\"border-slate-800/80\">\n"
    "        </div>\n"
    "      </section>\n"
    "    </main>\n"
    "    <footer id=\"status-bar\" class=\"border-t border-slate-800/80 "
    "bg-slate-950/80 px-6 py-4 text-sm text-slate-500 max-w-3xl mx-auto "
    "w-full\">\n"
    "      Status: Loading...\n"
    "    </footer>\n"
    "  </div>\n"
    "  <script>\n"
    "    function updateStats() {\n"
    "      fetch('/api/stats')\n"
    "        .then(response => response.json())\n"
    "        .then(stats => {\n"
    "          document.getElementById('status-bar').textContent = `Time: "
    "${stats.uptime} | Memory: ${stats.memory} | ${stats.os}`;\n"
    "        })\n"
    "        .catch(() => {});\n"
    "    }\n"
    "    updateStats();\n"
    "  </script>\n"
    "</body>\n"
    "</html>";

static void handle_about_page(int client_fd);

static volatile int keep_running = 1;

void serve_static_file(int client_fd, const char *filepath) {
  int fd = open(filepath, O_RDONLY);
  if (fd == -1) {
    logger_log(LOG_INFO, "File not found: %s", filepath);
    send_error_page(client_fd, 404, "File not found");
    return;
  }

  struct stat file_stat;
  if (fstat(fd, &file_stat) != 0) {
    logger_log(LOG_ERROR, "Failed to get file stats: %s", filepath);
    close(fd);
    send_error_page(client_fd, 500, "Internal Server Error");
    return;
  }

  // Send headers
  char headers[512];
  snprintf(headers, sizeof(headers),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: %s\r\n"
           "Content-Length: %lld\r\n"
           "Connection: close\r\n"
           "\r\n",
           get_content_type(filepath), (long long)file_stat.st_size);

  write(client_fd, headers, strlen(headers));

  // Send file content
  char file_buffer[8192];
  ssize_t bytes_read;
  while ((bytes_read = read(fd, file_buffer, sizeof(file_buffer))) > 0) {
    write(client_fd, file_buffer, bytes_read);
  }

  close(fd);
}

void handle_signal(int signal) {
  logger_log(LOG_INFO, "Received signal %d, shutting down...", signal);
  keep_running = 0;
}
void handle_health_check(int client_fd) {
  const char *response = "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/plain\r\n"
                         "Content-Length: 2\r\n"
                         "Connection: close\r\n"
                         "\r\n"
                         "OK";

  write(client_fd, response, strlen(response));
  logger_log(LOG_DEBUG, "Health check request handled");
}
#define BUFFER_SIZE 8192
void handle_stats_request(int client_fd) {
  struct system_stats stats = get_system_stats();

  char json_response[512];
  snprintf(json_response, sizeof(json_response),
           "{\"uptime\":\"%s\",\"memory\":\"%s\",\"os\":\"%s\"}", stats.uptime,
           stats.memory, stats.os_info);

  char headers[256];
  snprintf(headers, sizeof(headers),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: %lu\r\n"
           "Connection: close\r\n"
           "\r\n",
           strlen(json_response));

  write(client_fd, headers, strlen(headers));
  write(client_fd, json_response, strlen(json_response));
}

void handle_client(int client_fd, struct server_config *config) {
  char buffer[BUFFER_SIZE] = {0};
  ssize_t bytes_read = 0;
  char *end_of_headers = NULL;

  // Read the request headers
  while ((bytes_read = read(client_fd, buffer + bytes_read,
                            BUFFER_SIZE - bytes_read)) > 0) {
    if ((end_of_headers = strstr(buffer, "\r\n\r\n"))) {
      break;
    }

    // Only check size limit for headers
    if (bytes_read >= BUFFER_SIZE - 1) {
      logger_log(LOG_WARN, "Request headers too large");
      send_error_page(client_fd, 413, "Request headers too large");
      return;
    }
  }

  if (bytes_read <= 0) {
    logger_log(LOG_WARN, "Failed to read request");
    return;
  }

  // Parse the request line
  struct http_request req;
  if (!parse_http_request(buffer, &req)) {
    logger_log(LOG_WARN, "Malformed request: %.*s",
               (int)strcspn(buffer, "\r\n"), buffer);
    send_error_page(client_fd, 400, "Malformed request");
    return;
  }

  // Log the request
  logger_log(LOG_INFO, "Received request: %s %s %s", req.method, req.path,
             req.version);

  // Route handling
  if (strcmp(req.path, "/health") == 0) {
    handle_health_check(client_fd);
  } else if (strcmp(req.path, "/") == 0) {
    handle_index_page(client_fd, config);
  } else if (strcmp(req.path, "/api/stats") == 0) { // Add this condition
    handle_stats_request(client_fd);
  } else if (strcmp(req.path, "/about") == 0) {
    handle_about_page(client_fd);
  } else if (strncmp(req.path, "/post/", 6) == 0) {
    char *clean_path = sanitize_path(req.path + 6);
    if (clean_path && is_path_safe(clean_path)) {
      handle_markdown_post(client_fd, clean_path, config);
      free(clean_path);
    } else {
      send_error_page(client_fd, 400, "Invalid path");
    }
  } else {
    // Handle static files
    char *clean_path = sanitize_path(req.path);
    if (clean_path && is_path_safe(clean_path)) {
      char filepath[512];
      snprintf(filepath, sizeof(filepath), "%s%s", config->static_dir,
               clean_path);
      serve_static_file(client_fd, filepath);
      free(clean_path);
    } else {
      send_error_page(client_fd, 400, "Invalid path");
    }
  }
}

static void handle_about_page(int client_fd) {
  size_t body_length = strlen(ABOUT_TEMPLATE);

  char headers[256];
  snprintf(headers, sizeof(headers),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: %zu\r\n"
           "Connection: close\r\n"
           "\r\n",
           body_length);

  write(client_fd, headers, strlen(headers));
  write(client_fd, ABOUT_TEMPLATE, body_length);
}

// In start_server function in server.c
int start_server(struct server_config *config) {
  int server_fd;
  struct sockaddr_in address;

  logger_log(LOG_INFO, "Creating socket...");
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    logger_log(LOG_ERROR, "Socket creation failed: %s", strerror(errno));
    return EXIT_FAILURE;
  }

  // Set socket options
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    logger_log(LOG_ERROR, "Setsockopt failed: %s", strerror(errno));
    close(server_fd);
    return EXIT_FAILURE;
  }

  // Configure address
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY; // Explicitly bind to all interfaces
  address.sin_port = htons(config->port);

  logger_log(LOG_INFO, "Attempting to bind to 0.0.0.0:%d...", config->port);
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    logger_log(LOG_ERROR, "Bind failed: %s", strerror(errno));
    close(server_fd);
    return EXIT_FAILURE;
  }
  logger_log(LOG_INFO, "Successfully bound to 0.0.0.0:%d", config->port);

  logger_log(LOG_INFO, "Starting to listen...");
  if (listen(server_fd, 10) < 0) {
    logger_log(LOG_ERROR, "Listen failed: %s", strerror(errno));
    close(server_fd);
    return EXIT_FAILURE;
  }

  logger_log(LOG_INFO, "Server is ready to accept connections");

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
      logger_log(LOG_ERROR, "Accept failed: %s", strerror(errno));
      continue;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    logger_log(LOG_INFO, "New connection from %s", client_ip);

    handle_client(client_fd, config);
    close(client_fd);
  }

  close(server_fd);
  return EXIT_SUCCESS;
}

int main() {
  struct server_config config = load_config("config.json");
  logger_init(NULL);

  logger_log(LOG_INFO, "=== Server Starting ===");
  logger_log(LOG_INFO, "Configuration:");
  logger_log(LOG_INFO, "  Port: %d", config.port);
  logger_log(LOG_INFO, "  Static Dir: %s", config.static_dir);
  logger_log(LOG_INFO, "  Blog Dir: %s", config.blog_dir);

  // Verify directories exist
  struct stat st;
  if (stat(config.static_dir, &st) != 0) {
    logger_log(LOG_ERROR, "Static directory does not exist: %s",
               config.static_dir);
    return EXIT_FAILURE;
  }
  if (stat(config.blog_dir, &st) != 0) {
    logger_log(LOG_ERROR, "Blog directory does not exist: %s", config.blog_dir);
    return EXIT_FAILURE;
  }

  logger_log(LOG_INFO, "Directories verified, starting server...");
  return start_server(&config);
}

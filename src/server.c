#include "../include/server.h"
#include "../include/config.h"
#include "../include/error_pages.h"
#include "../include/http.h"
#include "../include/logger.h"
#include "../include/post.h"
#include "../include/security.h"
#include "../include/stats.h"
#include "../include/template.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

// about page now uses templates/about.html exclusively

static void handle_about_page(int client_fd, struct server_config *config);

static volatile int keep_running = 1;

static int parse_page_param(const char *query) {
  if (!query || *query == '\0') {
    return 1;
  }

  char buffer[256];
  strncpy(buffer, query, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  char *saveptr;
  char *token = strtok_r(buffer, "&", &saveptr);
  while (token) {
    if (strncmp(token, "page=", 5) == 0) {
      const char *value = token + 5;
      if (*value == '\0') {
        break;
      }

      char *endptr;
      long parsed = strtol(value, &endptr, 10);
      if (endptr != value && *endptr == '\0' && parsed > 0 &&
          parsed <= INT_MAX) {
        return (int)parsed;
      }
      break;
    }
    token = strtok_r(NULL, "&", &saveptr);
  }

  return 1;
}

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

  char path_only[sizeof(req.path)];
  char query[sizeof(req.path)];
  query[0] = '\0';

  const char *query_start = strchr(req.path, '?');
  if (query_start) {
    size_t base_len = (size_t)(query_start - req.path);
    if (base_len >= sizeof(path_only)) {
      base_len = sizeof(path_only) - 1;
    }
    memcpy(path_only, req.path, base_len);
    path_only[base_len] = '\0';

    size_t query_len = strlen(query_start + 1);
    if (query_len >= sizeof(query)) {
      query_len = sizeof(query) - 1;
    }
    memcpy(query, query_start + 1, query_len);
    query[query_len] = '\0';
  } else {
    strncpy(path_only, req.path, sizeof(path_only) - 1);
    path_only[sizeof(path_only) - 1] = '\0';
  }

  // Route handling
  if (strcmp(path_only, "/health") == 0) {
    handle_health_check(client_fd);
  } else if (strcmp(path_only, "/") == 0) {
    handle_index_page(client_fd, config);
  } else if (strcmp(path_only, "/blog") == 0) {
    int page = parse_page_param(query);
    handle_blog_page(client_fd, config, page);
  } else if (strcmp(path_only, "/api/stats") == 0) { // Add this condition
    handle_stats_request(client_fd);
  } else if (strcmp(path_only, "/about") == 0) {
    handle_about_page(client_fd, config);
  } else if (strncmp(path_only, "/post/", 6) == 0) {
    char *clean_path = sanitize_path(path_only + 6);
    if (clean_path && is_path_safe(clean_path)) {
      handle_markdown_post(client_fd, clean_path, config);
      free(clean_path);
    } else {
      send_error_page(client_fd, 400, "Invalid path");
    }
  } else {
    // Handle static files
    char *clean_path = sanitize_path(path_only);
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

static void handle_about_page(int client_fd, struct server_config *config) {
  char *rendered = NULL;
  char path[512];
  snprintf(path, sizeof(path), "%s/about.html", config->templates_dir);
  if (render_template_file(path, NULL, 0, &rendered) != 0 || !rendered) {
    send_error_page(client_fd, 500, "Failed to render about page");
    return;
  }

  size_t body_length = strlen(rendered);
  char headers[256];
  snprintf(headers, sizeof(headers),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: %zu\r\n"
           "Connection: close\r\n"
           "\r\n",
           body_length);

  write(client_fd, headers, strlen(headers));
  write(client_fd, rendered, body_length);
  free_rendered_template(rendered);
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

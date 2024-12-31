#include "../include/server.h"
#include "../include/http.h"
#include "../include/post.h"
#include "../include/stats.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
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

  // Read the HTTP request
  while ((bytes_read = read(client_fd, buffer + bytes_read,
                            BUFFER_SIZE - bytes_read)) > 0) {
    if ((end_of_headers = strstr(buffer, "\r\n\r\n")))
      break;
  }

  struct http_request req;
  parse_http_request(buffer, &req);

  printf("Received request: %s %s %s\n", req.method, req.path, req.version);

  // Check if this is a markodown post request
  if (strncmp(req.path, "/post/", 6) == 0) {
    printf("Handling markdown post request for path: %s\n", req.path);
    printf("Config blog_dir: %s\n", config->blog_dir);
    printf("Will look for file: %s%s.md\n", config->blog_dir, req.path + 6);
    handle_markdown_post(client_fd, req.path + 6,
                         config); // Skip "/post/" prefix
    return;
  }
  if (strcmp(req.path, "/api/stats") == 0) {
    handle_stats_request(client_fd);
    return;
  }
  // Construct file path
  char filepath[512];
  snprintf(filepath, sizeof(filepath), "%s%s", config->static_dir, req.path);

  // Handle root path
  if (strcmp(req.path, "/") == 0) {
    handle_index_page(client_fd, config);
    return;
  }

  // Open and serve the file
  int fd = open(filepath, O_RDONLY);
  if (fd == -1) {
    char *not_found = "HTTP/1.1 404 Not Found\r\n"
                      "Content-Type: text/plain\r\n"
                      "Content-Length: 9\r\n"
                      "Connection: close\r\n"
                      "\r\n"
                      "Not Found";
    write(client_fd, not_found, strlen(not_found));
  } else {
    struct stat file_stat;
    if (fstat(fd, &file_stat) == 0) {
      char headers[512];
      snprintf(headers, sizeof(headers),
               "HTTP/1.1 200 OK\r\n"
               "Content-Type: %s\r\n"
               "Content-Length: %ld\r\n"
               "Connection: close\r\n"
               "\r\n",
               get_content_type(filepath), file_stat.st_size);

      // Send headers
      size_t headers_len = strlen(headers);
      size_t headers_written = 0;
      while (headers_written < headers_len) {
        ssize_t written = write(client_fd, headers + headers_written,
                                headers_len - headers_written);
        if (written <= 0)
          break;
        headers_written += written;
      }

      // Send file content
      char file_buffer[4096];
      while ((bytes_read = read(fd, file_buffer, sizeof(file_buffer))) > 0) {
        size_t bytes_written = 0;
        while (bytes_written < bytes_read) {
          ssize_t written = write(client_fd, file_buffer + bytes_written,
                                  bytes_read - bytes_written);
          if (written <= 0)
            goto cleanup;
          bytes_written += written;
        }
      }
    }
  cleanup:
    close(fd);
  }
}

int start_server(struct server_config *config) {
  int server_fd;
  struct sockaddr_in address;

  // Create socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    return EXIT_FAILURE;
  }

  // Configure socket options
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    return EXIT_FAILURE;
  }

  // Setup address structure
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(config->host);
  address.sin_port = htons(config->port);

  // Bind
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    return EXIT_FAILURE;
  }

  // Listen
  if (listen(server_fd, 10) < 0) {
    perror("listen failed");
    return EXIT_FAILURE;
  }

  printf("Server listening on %s:%d\n", config->host, config->port);

  // Accept connections
  while (1) {
    int client_fd;
    int addrlen = sizeof(address);

    if ((client_fd = accept(server_fd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen)) < 0) {
      perror("accept failed");
      continue;
    }

    handle_client(client_fd, config);
    close(client_fd);
  }

  return EXIT_SUCCESS;
}

int main() {
  struct server_config config = load_config("config.json");
  return start_server(&config);
}

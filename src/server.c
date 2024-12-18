#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
// structure to hold http request
struct http_request {
  char method[8];
  char path[256];
  char version[16];
};

// parse icnoming http request
void parse_http_request(char *buffer, struct http_request *request) {
  sscanf(buffer, "%s %s %s", request->method, request->path, request->version);
}

int main() {
  int server_fd, client_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};

  // Create socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Configure socket options - allows us to reuse the address
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }

  // Configure address structure
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Bind socket to the port
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(server_fd, 10) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d\n", PORT);

  // Accept and handle incoming connections
  while (1) {
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen)) < 0) {
      perror("accept failed");
      continue;
    }

    // Read incoming data
    size_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);

    // Parse the incoming HTTP request
    struct http_request request;

    parse_http_request(buffer, &request);
    printf("Received request:%s %s %s\n", request.path, request.method,
           request.version);
    // Send a basic response
    // Proper HTTP/1.1 response with headers
    time_t now = time(NULL);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    char response_body[256 * 2];
    snprintf(response_body, sizeof(response_body),
             "{\"message\":\"Hello from C server!\","
             "\"timestamp\":\"%s\","
             "\"request\":{\"method\":\"%s\",\"path\":\"%s\"}}\n",
             time_str, request.method, request.path);

    char http_response[512 * 2];
    snprintf(http_response, sizeof(http_response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %lu\r\n"
             "\r\n"
             "%s",
             strlen(response_body), response_body);
    write(client_fd, http_response, strlen(http_response));

    close(client_fd);
    memset(buffer, 0, BUFFER_SIZE);
  }

  return 0;
}

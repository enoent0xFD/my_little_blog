#include "../include/config.h"
#include "../lib/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct server_config load_config(const char *filename) {
  struct server_config config = {.port = 8080, .posts_per_page = 10};
  strcpy(config.host, "127.0.0.1");
  strcpy(config.static_dir, "./static");
  strcpy(config.blog_dir, "./content");
  strcpy(config.templates_dir, "./templates");

  FILE *fp = fopen(filename, "r");
  if (!fp) {
    printf("Warning: Could not open config file, using defaults\n");
    return config;
  }

  // Read the file
  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *json_str = malloc(file_size + 1);
  fread(json_str, 1, file_size, fp);
  json_str[file_size] = '\0';
  fclose(fp);

  cJSON *json = cJSON_Parse(json_str);
  if (!json) {
    printf("Error parsing config file\n");
    free(json_str);
    return config;
  }

  // Parse server settings
  cJSON *server = cJSON_GetObjectItem(json, "server");
  if (server) {
    cJSON *port = cJSON_GetObjectItem(server, "port");
    if (port && port->valueint)
      config.port = port->valueint;

    cJSON *host = cJSON_GetObjectItem(server, "host");
    if (host && host->valuestring)
      strncpy(config.host, host->valuestring, sizeof(config.host) - 1);

    cJSON *static_dir = cJSON_GetObjectItem(server, "static_dir");
    if (static_dir && static_dir->valuestring)
      strncpy(config.static_dir, static_dir->valuestring,
              sizeof(config.static_dir) - 1);

    cJSON *blog_dir = cJSON_GetObjectItem(server, "blog_dir");
    if (blog_dir && blog_dir->valuestring)
      strncpy(config.blog_dir, blog_dir->valuestring,
              sizeof(config.blog_dir) - 1);

    cJSON *templates_dir = cJSON_GetObjectItem(server, "templates_dir");
    if (templates_dir && templates_dir->valuestring)
      strncpy(config.templates_dir, templates_dir->valuestring,
              sizeof(config.templates_dir) - 1);
  }

  // Parse blog settings
  cJSON *blog = cJSON_GetObjectItem(json, "blog");
  if (blog) {
    cJSON *title = cJSON_GetObjectItem(blog, "title");
    if (title && title->valuestring)
      strncpy(config.blog_title, title->valuestring,
              sizeof(config.blog_title) - 1);

    cJSON *description = cJSON_GetObjectItem(blog, "description");
    if (description && description->valuestring)
      strncpy(config.blog_description, description->valuestring,
              sizeof(config.blog_description) - 1);

    cJSON *author = cJSON_GetObjectItem(blog, "author");
    if (author && author->valuestring)
      strncpy(config.blog_author, author->valuestring,
              sizeof(config.blog_author) - 1);

    cJSON *posts_per_page = cJSON_GetObjectItem(blog, "posts_per_page");
    if (posts_per_page && posts_per_page->valueint)
      config.posts_per_page = posts_per_page->valueint;
  }

  cJSON_Delete(json);
  free(json_str);
  return config;
}

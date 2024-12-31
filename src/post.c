// src/post.c
#include "../include/post.h"
#include "../include/http.h"
#include "../include/markdown.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FRONTMATTER_DELIM "---"
const char *POST_TEMPLATE =
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "    <meta charset=\"UTF-8\">\n"
    "    <meta name=\"viewport\" content=\"width=device-width, "
    "initial-scale=1.0\">\n"
    "    <title>%s - The Abstract Layer</title>\n"
    "    <link rel=\"stylesheet\" href=\"/styles.css\">\n"
    "</head>\n"
    "<body>\n"
    "    <header>\n"
    "        <h1>The Abstract Layer</h1>\n"
    "    </header>\n"
    "    <div class=\"content\">\n"
    "        <div class=\"post\">\n"
    "            <h1>%s</h1>\n"
    "            <span class=\"post-date\">%s</span>\n"
    "            %s\n"
    "        </div>\n"
    "    </div>\n"
    "    <div class=\"status-bar\" id=\"status-bar\">\n"
    "        Status: Loading... |\n"
    "    </div>\n"
    "    <script>\n"
    "    function updateStats() {\n"
    "        fetch('/api/stats')\n"
    "            .then(response => response.json())\n"
    "            .then(stats => {\n"
    "                document.getElementById('status-bar').textContent = \n"
    "                    `Time: ${stats.uptime} | Memory used: ${stats.memory} "
    "| "
    "OS: ${stats.os}`;\n"
    "            })\n"
    "            .catch(error => console.error('Error fetching stats:', "
    "error));\n"
    "    }\n"
    "    updateStats();\n"
    "    </script>\n"
    "</body>\n"
    "</html>";

// src/post.c - add this template
const char *INDEX_TEMPLATE =
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "    <meta charset=\"UTF-8\">\n"
    "    <meta name=\"viewport\" content=\"width=device-width, "
    "initial-scale=1.0\">\n"
    "    <title>Abstraction Layer</title>\n"
    "    <link rel=\"stylesheet\" href=\"/styles.css\">\n"
    "</head>\n"
    "<body>\n"
    "    <header>\n"
    "        <h1 class=\"site-title\">Abstraction Layer</h1>\n"
    "        <div class=\"intro\">\n"
    "            <p>\n"
    "                I like to think I'm smart enough to understand the things I think about. "
    "            </p>\n"
    "            <p class=\"github-link\">\n"
    "                <a href=\"https://github.com/yourusername\" "
    "target=\"_blank\">\n"
    "                    <svg height=\"20\" width=\"20\" viewBox=\"0 0 16 16\" "
    "fill=\"var(--text-color)\">\n"
    "                        <path d=\"M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 "
    "5.47 7.59.4.07.55-.17.55-.38 "
    "0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-"
    ".28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 "
    "2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 "
    "0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 "
    "1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 "
    "1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 "
    "3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 "
    "8.013 0 0016 8c0-4.42-3.58-8-8-8z\"/>\n"
    "                    </svg>\n"
    "                    GitHub\n"
    "                </a>\n"
    "            </p>\n"
    "        </div>\n"
    "    </header>\n"
    "    <div class=\"content\">\n"
    "        <h2 class=\"posts-heading\">Posts</h2>\n"
    "        %s\n" // Post list will go here
    "    </div>\n"
    "    <div class=\"status-bar\" id=\"status-bar\">\n"
    "        Status: Loading...\n"
    "    </div>\n"
    "    <script>\n"
    "    function updateStats() {\n"
    "        fetch('/api/stats')\n"
    "            .then(response => response.json())\n"
    "            .then(stats => {\n"
    "                document.getElementById('status-bar').textContent = \n"
    "                    `Time: ${stats.uptime} | Memory: ${stats.memory} | "
    "${stats.os}`;\n"
    "            })\n"
    "            .catch(error => console.error('Error fetching stats:', "
    "error));\n"
    "    }\n"
    "    updateStats();\n"
    "    </script>\n"
    "</body>\n"
    "</html>";

void handle_index_page(int client_fd, struct server_config *config) {
  struct blog_index *index = build_post_index(config->blog_dir);

  // Build post list HTML
  char *posts_html = malloc(4096 * 16); // Allocate plenty of space for posts
  posts_html[0] = '\0';

  for (int i = 0; i < index->post_count; i++) {
    char post_html[4096];
    snprintf(post_html, sizeof(post_html),
             "<div class=\"post-preview\">\n"
             "    <span class=\"post-date\">%s</span>\n"
             "    <h2><a href=\"/post/%s\">%s</a></h2>\n"
             "    <p>%s</p>\n"
             "</div>\n",
             index->posts[i].metadata.date, index->posts[i].filename,
             index->posts[i].metadata.title, index->posts[i].metadata.preview);
    strcat(posts_html, post_html);
  }

  // Format full page
  char *full_html = malloc(strlen(INDEX_TEMPLATE) + strlen(posts_html) + 1);
  sprintf(full_html, INDEX_TEMPLATE, posts_html);

  // Send response
  char headers[512];
  snprintf(headers, sizeof(headers),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: %lu\r\n"
           "Connection: close\r\n"
           "\r\n",
           strlen(full_html));

  write(client_fd, headers, strlen(headers));
  write(client_fd, full_html, strlen(full_html));

  // Clean up
  free(posts_html);
  free(full_html);
  free_post_index(index);
}

int parse_post_metadata(const char *content, struct post_metadata *metadata) {
  // Initialize metadata with empty strings
  memset(metadata, 0, sizeof(struct post_metadata));

  // Check if content starts with frontmatter delimiter
  if (strncmp(content, FRONTMATTER_DELIM, strlen(FRONTMATTER_DELIM)) != 0) {
    return 0;
  }

  // Find end of frontmatter
  const char *end =
      strstr(content + strlen(FRONTMATTER_DELIM), FRONTMATTER_DELIM);
  if (!end) {
    return 0;
  }

  // Copy frontmatter section for parsing
  size_t fm_length = end - (content + strlen(FRONTMATTER_DELIM));
  char *frontmatter = malloc(fm_length + 1);
  strncpy(frontmatter, content + strlen(FRONTMATTER_DELIM), fm_length);
  frontmatter[fm_length] = '\0';

  // Parse each line
  char *line = strtok(frontmatter, "\n");
  while (line) {
    char *value = strchr(line, ':');
    if (value) {
      *value = '\0'; // Split at colon
      value++;       // Move past colon

      // Skip initial whitespace
      while (*value == ' ')
        value++;

      // Remove trailing whitespace
      char *end = value + strlen(value) - 1;
      while (end > value && (*end == ' ' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
      }

      if (strcmp(line, "title") == 0) {
        strncpy(metadata->title, value, sizeof(metadata->title) - 1);
      } else if (strcmp(line, "date") == 0) {
        strncpy(metadata->date, value, sizeof(metadata->date) - 1);
      } else if (strcmp(line, "preview") == 0) {
        strncpy(metadata->preview, value, sizeof(metadata->preview) - 1);
      }
    }
    line = strtok(NULL, "\n");
  }

  free(frontmatter);
  return 1;
}

void handle_markdown_post(int client_fd, const char *path,
                          struct server_config *config) {
  char filepath[512];
  snprintf(filepath, sizeof(filepath), "%s/%s.md", config->blog_dir, path);

  FILE *file = fopen(filepath, "r");
  if (!file) {
    send_404(client_fd);
    return;
  }

  // Read entire file content
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *content = malloc(file_size + 1);
  fread(content, 1, file_size, file);
  content[file_size] = '\0';
  fclose(file);

  // Parse metadata
  struct post_metadata metadata;
  if (!parse_post_metadata(content, &metadata)) {
    // Set default values if no metadata
    strncpy(metadata.title, "Untitled Post", sizeof(metadata.title));
    strncpy(metadata.date, "Unknown Date", sizeof(metadata.date));
    strncpy(metadata.preview, "", sizeof(metadata.preview));
  }

  // Find start of actual content (after frontmatter)
  const char *content_start = strstr(content, FRONTMATTER_DELIM);
  if (content_start) {
    content_start =
        strstr(content_start + strlen(FRONTMATTER_DELIM), FRONTMATTER_DELIM);
    if (content_start) {
      content_start += strlen(FRONTMATTER_DELIM);
    }
  }

  // Convert markdown to HTML
  char *html_content =
      markdown_to_html(content_start ? content_start : content);
  if (!html_content) {
    free(content);
    send_500(client_fd);
    return;
  }

  // Format the full HTML using our template
  char *full_html = malloc(strlen(html_content) + 4096);
  sprintf(full_html, POST_TEMPLATE,
          metadata.title, // page title
          metadata.title, // post title
          metadata.date,  // post date
          html_content);  // post content

  // Send response
  char headers[512];
  snprintf(headers, sizeof(headers),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: %lu\r\n"
           "Connection: close\r\n"
           "\r\n",
           strlen(full_html));

  write(client_fd, headers, strlen(headers));
  write(client_fd, full_html, strlen(full_html));

  // Clean up
  free(content);
  free(html_content);
  free(full_html);
}

struct blog_index *build_post_index(const char *content_dir) {
  DIR *dir;
  struct dirent *entry;
  struct blog_index *index = malloc(sizeof(struct blog_index));
  index->post_count = 0;

  dir = opendir(content_dir);
  if (!dir) {
    printf("Failed to open content directory: %s\n", content_dir);
    return index;
  }

  while ((entry = readdir(dir)) != NULL && index->post_count < MAX_POSTS) {
    // Skip . and .. directories
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    // Check if file ends with .md
    size_t name_len = strlen(entry->d_name);
    if (name_len > 3 && strcmp(entry->d_name + name_len - 3, ".md") == 0) {
      char filepath[512];
      snprintf(filepath, sizeof(filepath), "%s/%s", content_dir, entry->d_name);

      // Read file content
      FILE *file = fopen(filepath, "r");
      if (file) {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *content = malloc(file_size + 1);
        fread(content, 1, file_size, file);
        content[file_size] = '\0';
        fclose(file);

        // Store filename without .md extension
        strncpy(index->posts[index->post_count].filename, entry->d_name,
                name_len - 3);
        index->posts[index->post_count].filename[name_len - 3] = '\0';

        // Parse metadata
        parse_post_metadata(content, &index->posts[index->post_count].metadata);
        free(content);

        index->post_count++;
      }
    }
  }

  closedir(dir);

  // Sort posts by date (newest first)
  for (int i = 0; i < index->post_count - 1; i++) {
    for (int j = i + 1; j < index->post_count; j++) {
      if (strcmp(index->posts[i].metadata.date, index->posts[j].metadata.date) <
          0) {
        struct blog_post temp = index->posts[i];
        index->posts[i] = index->posts[j];
        index->posts[j] = temp;
      }
    }
  }

  return index;
}

void free_post_index(struct blog_index *index) { free(index); }

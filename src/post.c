// src/post.c
#include "../include/post.h"
#include "../include/http.h"
#include "../include/error_pages.h"
#include "../include/markdown.h"
#include "../include/template.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Templates are file-based. See templates/post.html and templates/index.html
#define FRONTMATTER_DELIM "---"

// INDEX_TEMPLATE removed; replaced by templates/index.html

static char *build_pagination_html(int page, int total_pages) {
  char *markup = malloc(768);
  if (!markup) {
    return NULL;
  }
  markup[0] = '\0';

  if (total_pages <= 1) {
    return markup;
  }

  if (page > 1 && page < total_pages) {
    snprintf(markup, 768,
             "<div class=\"mt-12 flex items-center justify-between\">\n"
             "  <a href=\"/blog?page=%d\" class=\"text-sm font-medium text-slate-300 hover:text-slate-100\">Newer posts</a>\n"
             "  <a href=\"/blog?page=%d\" class=\"inline-flex items-center gap-2 rounded-full border border-slate-700 px-6 py-3 text-sm font-medium text-slate-200 transition hover:border-slate-500\">Load more...</a>\n"
             "</div>\n",
             page - 1, page + 1);
  } else if (page > 1) {
    snprintf(markup, 768,
             "<div class=\"mt-12 flex justify-start\">\n"
             "  <a href=\"/blog?page=%d\" class=\"text-sm font-medium text-slate-300 hover:text-slate-100\">Newer posts</a>\n"
             "</div>\n",
             page - 1);
  } else {
    snprintf(markup, 768,
             "<div class=\"mt-12 flex justify-center\">\n"
             "  <a href=\"/blog?page=%d\" class=\"inline-flex items-center gap-2 rounded-full border border-slate-700 px-6 py-3 text-sm font-medium text-slate-200 transition hover:border-slate-500\">Load more...</a>\n"
             "</div>\n",
             page + 1);
  }

  return markup;
}

static void render_blog_page(int client_fd, struct server_config *config,
                             int page) {
  int posts_per_page =
      (config->posts_per_page > 0) ? config->posts_per_page : 10;

  struct blog_index *index = build_post_index(config->blog_dir);
  if (!index) {
    send_error_page(client_fd, 500, "Failed to load blog index");
    return;
  }

  int total_posts = index->post_count;

  if (total_posts == 0 && page > 1) {
    free_post_index(index);
    send_error_page(client_fd, 404, "Page not found");
    return;
  }

  if (page < 1) {
    page = 1;
  }

  int total_pages =
      (total_posts == 0) ? 1 : (total_posts + posts_per_page - 1) / posts_per_page;

  if (total_posts > 0 && page > total_pages) {
    free_post_index(index);
    send_error_page(client_fd, 404, "Page not found");
    return;
  }

  int start_index = (page - 1) * posts_per_page;
  int end_index = start_index + posts_per_page;
  if (end_index > total_posts) {
    end_index = total_posts;
  }

  size_t posts_html_capacity =
      (size_t)posts_per_page * 2048 + 1024; // generous buffer
  if (posts_html_capacity < 2048) {
    posts_html_capacity = 2048;
  }

  char *posts_html = malloc(posts_html_capacity);
  if (!posts_html) {
    free_post_index(index);
    send_error_page(client_fd, 500, "Failed to render posts");
    return;
  }

  posts_html[0] = '\0';
  size_t offset = 0;

  if (total_posts == 0) {
    snprintf(posts_html, posts_html_capacity,
             "<p class=\"text-sm text-slate-500\">Nothing here yet&mdash;new "
             "writing will land soon.</p>\n");
  } else {
    const char *POST_CARD_TEMPLATE =
        "<article class=\"group rounded-2xl border border-slate-800/80 bg-slate-900/40 px-6 py-6 transition-colors hover:border-slate-700\">\n"
        "  <div class=\"flex items-center gap-3 text-xs uppercase tracking-[0.3em] text-slate-500\">\n"
        "    <span>%s</span>\n"
        "  </div>\n"
        "  <h3 class=\"mt-4 text-2xl font-semibold text-slate-100 group-hover:text-white\">\n"
        "    <a href=\"/post/%s\">%s</a>\n"
        "  </h3>\n"
        "  <p class=\"mt-3 text-slate-400\">%s</p>\n"
        "</article>\n";

    for (int i = start_index; i < end_index; i++) {
      const struct blog_post *post = &index->posts[i];
      int needed = snprintf(
          posts_html + offset, posts_html_capacity - offset,
          POST_CARD_TEMPLATE, post->metadata.date, post->filename,
          post->metadata.title, post->metadata.preview);

      if (needed < 0) {
        continue;
      }

      if ((size_t)needed >= posts_html_capacity - offset) {
        size_t new_capacity = posts_html_capacity + needed + 1024;
        char *resized = realloc(posts_html, new_capacity);
        if (!resized) {
          free(posts_html);
          free_post_index(index);
          send_error_page(client_fd, 500, "Failed to render posts");
          return;
        }
        posts_html = resized;
        posts_html_capacity = new_capacity;
        needed = snprintf(posts_html + offset, posts_html_capacity - offset,
                          POST_CARD_TEMPLATE, post->metadata.date,
                          post->filename, post->metadata.title,
                          post->metadata.preview);

        if (needed < 0 || (size_t)needed >= posts_html_capacity - offset) {
          free(posts_html);
          free_post_index(index);
          send_error_page(client_fd, 500, "Failed to render posts");
          return;
        }
      }

      offset += (size_t)needed;
    }
  }

  char *pagination_html = build_pagination_html(page, total_pages);
  if (!pagination_html) {
    free(posts_html);
    free_post_index(index);
    send_error_page(client_fd, 500, "Failed to render pagination");
    return;
  }

  struct template_kv kvs[2];
  kvs[0].key = "POSTS";
  kvs[0].value = posts_html;
  kvs[0].is_raw = 1;
  kvs[1].key = "PAGINATION";
  kvs[1].value = pagination_html;
  kvs[1].is_raw = 1;

  char *full_html = NULL;
  char index_tpl_path[512];
  snprintf(index_tpl_path, sizeof(index_tpl_path), "%s/index.html",
           config->templates_dir);
  if (render_template_file(index_tpl_path, kvs, 2, &full_html) != 0 ||
      !full_html) {
    free(posts_html);
    free(pagination_html);
    free_post_index(index);
    send_error_page(client_fd, 500, "Failed to render page");
    return;
  }

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

  free(posts_html);
  free(pagination_html);
  free(full_html);
  free_post_index(index);
}

void handle_index_page(int client_fd, struct server_config *config) {
  render_blog_page(client_fd, config, 1);
}

void handle_blog_page(int client_fd, struct server_config *config, int page) {
  render_blog_page(client_fd, config, page);
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

  // Render with file-based template
  struct template_kv pkvs[4];
  pkvs[0].key = "TITLE";
  pkvs[0].value = metadata.title;
  pkvs[0].is_raw = 0;
  pkvs[1].key = "DATE";
  pkvs[1].value = metadata.date;
  pkvs[1].is_raw = 0;
  pkvs[2].key = "POST_TITLE";
  pkvs[2].value = metadata.title;
  pkvs[2].is_raw = 0;
  pkvs[3].key = "CONTENT";
  pkvs[3].value = html_content;
  pkvs[3].is_raw = 1;

  char *full_html = NULL;
  char post_tpl_path[512];
  snprintf(post_tpl_path, sizeof(post_tpl_path), "%s/post.html",
           config->templates_dir);
  if (render_template_file(post_tpl_path, pkvs, 4, &full_html) != 0 ||
      !full_html) {
    free(content);
    free(html_content);
    send_500(client_fd);
    return;
  }

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

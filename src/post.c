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
    "  <meta charset=\"UTF-8\">\n"
    "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    "  <title>%s · Filip Mihalic</title>\n"
    "  <meta name=\"description\" content=\"Writing about systems programming, resilient software, and learning in public.\">\n"
    "  <meta name=\"keywords\" content=\"Filip Mihalic, systems programming, Rust, C, software engineering\">\n"
    "  <meta property=\"og:title\" content=\"Filip Mihalic\">\n"
    "  <meta property=\"og:description\" content=\"Writing about systems programming, resilient software, and learning in public.\">\n"
    "  <meta property=\"og:type\" content=\"website\">\n"
    "  <meta property=\"og:url\" content=\"https://filipmihalic.com\">\n"
    "  <meta name=\"twitter:card\" content=\"summary\">\n"
    "  <meta name=\"twitter:title\" content=\"Filip Mihalic\">\n"
    "  <meta name=\"twitter:description\" content=\"Writing about systems programming, resilient software, and learning in public.\">\n"
    "  <link rel=\"icon\" type=\"image/x-icon\" href=\"/favicon.ico\">\n"
    "  <link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"/favicon-32x32.png\">\n"
    "  <link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"/favicon-16x16.png\">\n"
    "  <link rel=\"apple-touch-icon\" sizes=\"180x180\" href=\"/apple-touch-icon.png\">\n"
    "  <script src=\"https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4\"></script>\n"
    "  <link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/gh/highlightjs/cdn-release@11.11.1/build/styles/tokyo-night-dark.min.css\">\n"
    "  <script defer src=\"https://cdn.jsdelivr.net/gh/highlightjs/cdn-release@11.11.1/build/highlight.min.js\"></script>\n"
    "  <style>\n"
    "    :root { color-scheme: dark; }\n"
    "    .markdown :is(p, ul, ol, blockquote, pre) { margin-top: 1.5rem; margin-bottom: 1.5rem; }\n"
    "    .markdown h2 { margin-top: 3rem; font-size: 1.5rem; font-weight: 600; color: #e2e8f0; }\n"
    "    .markdown h3 { margin-top: 2.5rem; font-size: 1.25rem; font-weight: 600; color: #cbd5f5; }\n"
    "    .markdown a { color: #60a5fa; text-decoration: none; }\n"
    "    .markdown a:hover { text-decoration: underline; }\n"
    "    .markdown code { background-color: rgba(148, 163, 184, 0.15); padding: 0.2rem 0.4rem; border-radius: 0.375rem; font-size: 0.875rem; }\n"
    "    .markdown pre { background-color: rgba(15, 23, 42, 0.8); padding: 1.5rem; border-radius: 0.75rem; overflow-x: auto; }\n"
    "    .markdown pre code { background: transparent; padding: 0; }\n"
    "  </style>\n"
    "</head>\n"
    "<body class=\"bg-slate-950 text-slate-200 font-sans\">\n"
    "  <div class=\"min-h-screen\">\n"
    "    <header class=\"border-b border-slate-800/80 bg-slate-950/80 backdrop-blur\">\n"
    "      <div class=\"max-w-3xl mx-auto flex flex-col gap-6 px-6 py-10\">\n"
    "        <div class=\"flex flex-col gap-6 md:flex-row md:items-center md:justify-between\">\n"
    "          <a href=\"/\" class=\"text-2xl font-semibold text-slate-100\">Filip Mihalic</a>\n"
    "          <nav class=\"flex items-center gap-6 text-sm\">\n"
    "            <a href=\"/\" class=\"text-slate-100 font-medium\">Blog</a>\n"
    "            <a href=\"/about\" class=\"text-slate-400 hover:text-slate-200\">About</a>\n"
    "          </nav>\n"
    "        </div>\n"
    "        <p class=\"text-sm text-slate-400\">Systems, software, and the craft of building things that last.</p>\n"
    "      </div>\n"
    "    </header>\n"
    "    <main class=\"max-w-3xl mx-auto px-6 py-12\">\n"
    "      <article class=\"flex flex-col gap-6\">\n"
    "        <div class=\"flex items-center gap-3 text-sm text-slate-400\">\n"
    "          <span class=\"inline-flex items-center gap-2\">\n"
    "            <span class=\"h-px w-10 bg-slate-700\"></span>\n"
    "            <span class=\"tracking-wide uppercase\">%s</span>\n"
    "          </span>\n"
    "        </div>\n"
    "        <h1 class=\"text-4xl font-semibold text-slate-50\">%s</h1>\n"
    "        <div class=\"markdown leading-relaxed text-slate-200\">\n"
    "          %s\n"
    "        </div>\n"
    "      </article>\n"
    "    </main>\n"
    "    <footer id=\"status-bar\" class=\"border-t border-slate-800/80 bg-slate-950/80 px-6 py-4 text-sm text-slate-500 max-w-3xl mx-auto w-full\">\n"
    "      Status: Loading...\n"
    "    </footer>\n"
    "  </div>\n"
    "  <script>\n"
    "    function updateStats() {\n"
    "      fetch('/api/stats')\n"
    "        .then(response => response.json())\n"
    "        .then(stats => {\n"
    "          document.getElementById('status-bar').textContent = `Time: ${stats.uptime} | Memory used: ${stats.memory} | ${stats.os}`;\n"
    "        })\n"
    "        .catch(() => {});\n"
    "    }\n"
    "    document.addEventListener('DOMContentLoaded', function () {\n"
    "      if (window.hljs && window.hljs.highlightAll) {\n"
    "        window.hljs.highlightAll();\n"
    "      }\n"
    "    });\n"
    "    updateStats();\n"
    "  </script>\n"
    "</body>\n"
    "</html>";

// src/post.c - add this template
const char *INDEX_TEMPLATE =
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "  <meta charset=\"UTF-8\">\n"
    "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    "  <title>Filip Mihalic · Writing on systems and software</title>\n"
    "  <meta name=\"description\" content=\"Systems programming, resilient software, and lessons from working down the stack.\">\n"
    "  <meta name=\"keywords\" content=\"Filip Mihalic, systems programming, Rust, C, software engineering\">\n"
    "  <meta property=\"og:title\" content=\"Filip Mihalic\">\n"
    "  <meta property=\"og:description\" content=\"Systems programming, resilient software, and lessons from working down the stack.\">\n"
    "  <meta property=\"og:type\" content=\"website\">\n"
    "  <meta property=\"og:url\" content=\"https://filipmihalic.com\">\n"
    "  <meta name=\"twitter:card\" content=\"summary\">\n"
    "  <meta name=\"twitter:title\" content=\"Filip Mihalic\">\n"
    "  <meta name=\"twitter:description\" content=\"Systems programming, resilient software, and lessons from working down the stack.\">\n"
    "  <link rel=\"icon\" type=\"image/x-icon\" href=\"/favicon.ico\">\n"
    "  <link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"/favicon-32x32.png\">\n"
    "  <link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"/favicon-16x16.png\">\n"
    "  <link rel=\"apple-touch-icon\" sizes=\"180x180\" href=\"/apple-touch-icon.png\">\n"
    "  <script src=\"https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4\"></script>\n"
    "  <style>\n"
    "    :root { color-scheme: dark; }\n"
    "  </style>\n"
    "</head>\n"
    "<body class=\"bg-slate-950 text-slate-200 font-sans\">\n"
    "  <div class=\"min-h-screen\">\n"
    "    <header class=\"border-b border-slate-800/80 bg-slate-950/80 backdrop-blur\">\n"
    "      <div class=\"max-w-3xl mx-auto flex flex-col gap-6 px-6 py-12\">\n"
    "        <div class=\"flex flex-col gap-6 md:flex-row md:items-center md:justify-between\">\n"
    "          <a href=\"/\" class=\"text-3xl font-semibold text-slate-100\">Filip Mihalic</a>\n"
    "          <nav class=\"flex items-center gap-6 text-sm\">\n"
    "            <a href=\"/\" class=\"text-slate-100 font-medium\">Blog</a>\n"
    "            <a href=\"/about\" class=\"text-slate-400 hover:text-slate-200\">About</a>\n"
    "          </nav>\n"
    "        </div>\n"
    "        <div class=\"flex flex-col gap-6 md:flex-row md:items-center md:gap-10\">\n"
    "          <img src=\"/images/filip.webp\" alt=\"Portrait of Filip Mihalic\" class=\"h-28 w-28 rounded-full border border-slate-800 object-cover shadow-lg\" loading=\"lazy\">\n"
    "          <p class=\"max-w-2xl text-base text-slate-400\">Working down the stack&mdash;from web apps to networking internals&mdash;and sharing the lessons from building dependable software in Rust, C, and beyond.</p>\n"
    "        </div>\n"
    "      </div>\n"
    "    </header>\n"
    "    <main class=\"max-w-3xl mx-auto px-6 py-12\">\n"
    "      <section class=\"flex flex-col gap-4\">\n"
    "        <h2 class=\"text-sm uppercase tracking-[0.3em] text-slate-500\">Latest Writing</h2>\n"
    "        <div class=\"w-16 h-px bg-slate-700\"></div>\n"
    "      </section>\n"
    "      <div class=\"mt-10 grid gap-8\">\n"
    "        %s\n"
    "      </div>\n"
    "    </main>\n"
    "    <footer id=\"status-bar\" class=\"border-t border-slate-800/80 bg-slate-950/80 px-6 py-4 text-sm text-slate-500 max-w-3xl mx-auto w-full\">\n"
    "      Status: Loading...\n"
    "    </footer>\n"
    "  </div>\n"
    "  <script>\n"
    "    function updateStats() {\n"
    "      fetch('/api/stats')\n"
    "        .then(response => response.json())\n"
    "        .then(stats => {\n"
    "          document.getElementById('status-bar').textContent = `Time: ${stats.uptime} | Memory: ${stats.memory} | ${stats.os}`;\n"
    "        })\n"
    "        .catch(() => {});\n"
    "    }\n"
    "    updateStats();\n"
    "  </script>\n"
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
             "<article class=\"group rounded-2xl border border-slate-800/80 bg-slate-900/40 px-6 py-6 transition-colors hover:border-slate-700\">\n"
             "  <div class=\"flex items-center gap-3 text-xs uppercase tracking-[0.3em] text-slate-500\">\n"
             "    <span>%s</span>\n"
             "  </div>\n"
             "  <h3 class=\"mt-4 text-2xl font-semibold text-slate-100 group-hover:text-white\">\n"
             "    <a href=\"/post/%s\">%s</a>\n"
             "  </h3>\n"
             "  <p class=\"mt-3 text-slate-400\">%s</p>\n"
             "</article>\n",
             index->posts[i].metadata.date, index->posts[i].filename,
             index->posts[i].metadata.title, index->posts[i].metadata.preview);
    strcat(posts_html, post_html);
  }

  if (index->post_count == 0) {
    strcpy(posts_html,
           "<p class=\"text-sm text-slate-500\">Nothing here yet&mdash;new "
           "writing will land soon.</p>\n");
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
          metadata.date,  // post date
          metadata.title, // post title
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

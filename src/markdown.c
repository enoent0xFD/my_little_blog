#include "../include/markdown.h"
#include "../lib/md4c/md4c-html.h"
#include "../lib/md4c/md4c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// callback for md4c-html to write output
static void write_output(const MD_CHAR *text, MD_SIZE size, void *userdata) {
  char **output = (char **)userdata;
  size_t len = *output ? strlen(*output) : 0;
  *output = realloc(*output, len + size + 1);
  memcpy(*output + len, text, size);
  (*output)[len + size] = '\0';
}

char *markdown_to_html(const char *markdown_content) {
  char *html_output = NULL;

  // Configure HTML renderer options
  unsigned parser_flags = MD_FLAG_COLLAPSEWHITESPACE | MD_FLAG_TABLES |
                          MD_FLAG_STRIKETHROUGH |
                          MD_FLAG_PERMISSIVEEMAILAUTOLINKS;
  unsigned renderer_flags = MD_HTML_FLAG_SKIP_UTF8_BOM;

  // Convert markdown to HTML
  int result = md_html(markdown_content, strlen(markdown_content), write_output,
                       &html_output, parser_flags, renderer_flags);

  if (result != 0) {
    free(html_output);
    return NULL;
  }

  return html_output;
}

char *load_markdown_file(const char *filepath) {
  FILE *file = fopen(filepath, "r");
  if (!file) {
    return NULL;
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // Read file content
  char *content = malloc(size + 1);
  fread(content, 1, size, file);
  content[size] = '\0';

  fclose(file);
  char *html = markdown_to_html(content);
  free(content);
  return html;
}
void free_markdown_html(char *html) { free(html); }

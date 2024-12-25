#ifndef MARKDOWN_H
#define MARKDOWN_H

//convet markdown to html
char *markdown_to_html(const char *markdown_content);
//read and parse markdown file
char *load_markdown_file(const char *filepath);
//free generated html
void free_markdown_html(char *html);

#endif

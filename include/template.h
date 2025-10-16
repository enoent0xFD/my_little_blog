// Minimal template rendering helper
#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <stddef.h>

struct template_kv {
  const char *key;   // key name without braces, e.g. "TITLE"
  const char *value; // replacement value (UTF-8 string)
  int is_raw;        // if non-zero, do not HTML-escape (used for HTML fragments)
};

// Renders a template file located at `filepath` into an allocated buffer in `*out`.
// Supports keys in two forms:
//  - {{KEY}}   -> escaped replacement
//  - {{{KEY}}} -> raw replacement (no escaping)
// Returns 0 on success, non-zero on error.
int render_template_file(const char *filepath, const struct template_kv *vars,
                         size_t nvars, char **out);

// Utility to free the rendered buffer (alias to free for clarity).
void free_rendered_template(char *buf);

#endif // TEMPLATE_H


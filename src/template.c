// Minimal file-based template renderer
#include "../include/template.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

// Basic source cache: caches template file contents by path and mtime
struct tpl_cache_entry {
  char path[512];
  char *data;
  size_t len;
  time_t mtime;
  int in_use;
};

#define TPL_CACHE_CAP 32
static struct tpl_cache_entry g_tpl_cache[TPL_CACHE_CAP];

static int stat_mtime(const char *path, time_t *out) {
  struct stat st;
  if (stat(path, &st) != 0) return -1;
  if (out) *out = st.st_mtime;
  return 0;
}

static char *read_file_all_uncached(const char *path, size_t *out_len) {
  FILE *f = fopen(path, "rb");
  if (!f) return NULL;
  if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
  long sz = ftell(f);
  if (sz < 0) { fclose(f); return NULL; }
  if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
  char *buf = (char *)malloc((size_t)sz + 1);
  if (!buf) { fclose(f); return NULL; }
  size_t n = fread(buf, 1, (size_t)sz, f);
  fclose(f);
  buf[n] = '\0';
  if (out_len) *out_len = n;
  return buf;
}

static char *read_file_all(const char *path, size_t *out_len) {
  time_t mtime = 0;
  if (stat_mtime(path, &mtime) != 0) return NULL;

  int free_slot = -1;
  for (int i = 0; i < TPL_CACHE_CAP; ++i) {
    if (g_tpl_cache[i].in_use) {
      if (strncmp(g_tpl_cache[i].path, path, sizeof(g_tpl_cache[i].path)) == 0) {
        if (g_tpl_cache[i].mtime == mtime && g_tpl_cache[i].data) {
          if (out_len) *out_len = g_tpl_cache[i].len;
          char *copy = (char *)malloc(g_tpl_cache[i].len + 1);
          if (!copy) return NULL;
          memcpy(copy, g_tpl_cache[i].data, g_tpl_cache[i].len + 1);
          return copy;
        }
        // refresh stale entry
        free(g_tpl_cache[i].data);
        size_t nlen = 0;
        char *ndata = read_file_all_uncached(path, &nlen);
        if (!ndata) return NULL;
        g_tpl_cache[i].data = ndata;
        g_tpl_cache[i].len = nlen;
        g_tpl_cache[i].mtime = mtime;
        if (out_len) *out_len = nlen;
        char *copy = (char *)malloc(nlen + 1);
        if (!copy) return NULL;
        memcpy(copy, ndata, nlen + 1);
        return copy;
      }
    } else if (free_slot == -1) {
      free_slot = i;
    }
  }

  int slot = (free_slot != -1) ? free_slot : 0;
  if (free_slot == -1 && g_tpl_cache[slot].in_use) {
    free(g_tpl_cache[slot].data);
    memset(&g_tpl_cache[slot], 0, sizeof(g_tpl_cache[slot]));
  }

  size_t nlen = 0;
  char *ndata = read_file_all_uncached(path, &nlen);
  if (!ndata) return NULL;
  strncpy(g_tpl_cache[slot].path, path, sizeof(g_tpl_cache[slot].path) - 1);
  g_tpl_cache[slot].path[sizeof(g_tpl_cache[slot].path) - 1] = '\0';
  g_tpl_cache[slot].data = ndata;
  g_tpl_cache[slot].len = nlen;
  g_tpl_cache[slot].mtime = mtime;
  g_tpl_cache[slot].in_use = 1;
  if (out_len) *out_len = nlen;
  char *copy = (char *)malloc(nlen + 1);
  if (!copy) return NULL;
  memcpy(copy, ndata, nlen + 1);
  return copy;
}

static char *html_escape(const char *s) {
  if (!s)
    return strdup("");
  size_t extra = 0;
  for (const char *p = s; *p; ++p) {
    switch (*p) {
    case '&':
      extra += 4; // & -> &amp; (5-1)
      break;
    case '<':
    case '>':
      extra += 3; // < -> &lt; or > -> &gt;
      break;
    case '"':
      extra += 5; // " -> &quot;
      break;
    case '\'':
      extra += 5; // ' -> &#39;
      break;
    default:
      break;
    }
  }
  size_t len = strlen(s);
  char *out = (char *)malloc(len + extra + 1);
  if (!out)
    return NULL;
  char *w = out;
  for (const char *p = s; *p; ++p) {
    switch (*p) {
    case '&':
      memcpy(w, "&amp;", 5);
      w += 5;
      break;
    case '<':
      memcpy(w, "&lt;", 4);
      w += 4;
      break;
    case '>':
      memcpy(w, "&gt;", 4);
      w += 4;
      break;
    case '"':
      memcpy(w, "&quot;", 6);
      w += 6;
      break;
    case '\'':
      memcpy(w, "&#39;", 5);
      w += 5;
      break;
    default:
      *w++ = *p;
      break;
    }
  }
  *w = '\0';
  return out;
}

static const struct template_kv *find_kv(const struct template_kv *vars,
                                         size_t nvars, const char *key,
                                         size_t key_len) {
  for (size_t i = 0; i < nvars; ++i) {
    if (strlen(vars[i].key) == key_len &&
        strncmp(vars[i].key, key, key_len) == 0) {
      return &vars[i];
    }
  }
  return NULL;
}

int render_template_file(const char *filepath, const struct template_kv *vars,
                         size_t nvars, char **out) {
  if (!out)
    return -1;
  *out = NULL;

  size_t tlen = 0;
  char *tpl = read_file_all(filepath, &tlen);
  if (!tpl)
    return -1;

  // Reserve output buffer (start with ~1.5x of template)
  size_t cap = tlen + 1024;
  char *buf = (char *)malloc(cap);
  if (!buf) {
    free(tpl);
    return -1;
  }
  size_t off = 0;

  for (size_t i = 0; i < tlen;) {
    if (tpl[i] == '{' && i + 1 < tlen && tpl[i + 1] == '{') {
      int triple = 0;
      size_t j = i + 2;
      if (j < tlen && tpl[j] == '{') {
        triple = 1;
        j++;
      }
      size_t key_start = j;
      while (j < tlen) {
        if (tpl[j] == '}' && j + 1 < tlen && tpl[j + 1] == '}') {
          if (triple) {
            if (j + 2 < tlen && tpl[j + 2] == '}') {
              // found {{{KEY}}}
              size_t key_len = j - key_start;
              const struct template_kv *kv =
                  find_kv(vars, nvars, tpl + key_start, key_len);
              const char *val = kv ? kv->value : "";
              // triple braces always raw, do not escape
              size_t need = strlen(val);
              if (off + need + 1 > cap) {
                cap = (off + need + 1) * 2;
                char *res = (char *)realloc(buf, cap);
                if (!res) {
                  free(buf);
                  free(tpl);
                  return -1;
                }
                buf = res;
              }
              memcpy(buf + off, val, need);
              off += need;
              i = j + 3; // skip }}}
              goto continue_outer;
            }
          } else {
            // found {{KEY}}
            size_t key_len = j - key_start;
            const struct template_kv *kv =
                find_kv(vars, nvars, tpl + key_start, key_len);
            const char *val = kv ? kv->value : "";
            char *emit = (kv && kv->is_raw) ? (char *)val : html_escape(val);
            size_t need = strlen(emit);
            if (off + need + 1 > cap) {
              cap = (off + need + 1) * 2;
              char *res = (char *)realloc(buf, cap);
              if (!res) {
                if (!(kv && kv->is_raw))
                  free(emit);
                free(buf);
                free(tpl);
                return -1;
              }
              buf = res;
            }
            memcpy(buf + off, emit, need);
            off += need;
            if (!(kv && kv->is_raw))
              free(emit);
            i = j + 2; // skip }}
            goto continue_outer;
          }
        }
        j++;
      }
      // If we get here, no closing braces; treat literally
    }

    // Copy one char
    if (off + 2 > cap) {
      cap *= 2;
      char *res = (char *)realloc(buf, cap);
      if (!res) {
        free(buf);
        free(tpl);
        return -1;
      }
      buf = res;
    }
    buf[off++] = tpl[i++];

  continue_outer:
    (void)0;
  }

  buf[off] = '\0';
  free(tpl);
  *out = buf;
  return 0;
}

void free_rendered_template(char *buf) { free(buf); }

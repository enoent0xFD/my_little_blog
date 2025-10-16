#ifndef CONFIG_H
#define CONFIG_H

struct server_config {
    int port;
    char host[256];
    char static_dir[256];
    char blog_dir[256];
    char templates_dir[256];
    char blog_title[256];
    char blog_description[512];
    char blog_author[256];
    int posts_per_page;
};

struct server_config load_config(const char* filename);

#endif

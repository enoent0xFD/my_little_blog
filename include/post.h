#ifndef POST_H
#define POST_H

#include "config.h"

struct post_metadata {
    char title[256];
    char date[64];
    char preview[512];
};

#define MAX_POSTS 100

struct blog_post {
    char filename[256];
    struct post_metadata metadata;
};

struct blog_index {
    struct blog_post posts[MAX_POSTS];
    int post_count;
};

// Function declarations
int parse_post_metadata(const char* content, struct post_metadata* metadata);
void handle_markdown_post(int client_fd, const char* path, struct server_config* config);
struct blog_index* build_post_index(const char* content_dir);
void free_post_index(struct blog_index* index);
void handle_index_page(int client_fd, struct server_config* config);

#endif

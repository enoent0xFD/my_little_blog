#ifndef SERVER_H
#define SERVER_H

#include "config.h"

int start_server(struct server_config* config);
void handle_client(int client_fd, struct server_config* config);

#endif

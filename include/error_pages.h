// include/error_pages.h
#ifndef ERROR_PAGES_H
#define ERROR_PAGES_H

// Function to send styled error pages
void send_error_page(int client_fd, int status_code, const char* message);

#endif

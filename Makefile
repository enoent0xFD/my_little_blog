CC=gcc
CFLAGS=-I./include -Wall -Wextra
SRC_DIR=src
BUILD_DIR=build

all: $(BUILD_DIR)/blog_server

$(BUILD_DIR)/blog_server: $(SRC_DIR)/server.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean

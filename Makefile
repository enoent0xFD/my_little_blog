CC=gcc
CFLAGS=-I./include -I./lib/md4c -Wall -Wextra
SRC_DIR=src
LIB_DIR=lib
BUILD_DIR=build
MD4C_DIR=$(BUILD_DIR)/md4c

# Define source files explicitly to ensure order
SRCS=$(wildcard $(SRC_DIR)/*.c)
MD4C_SRCS=lib/md4c/entity.c lib/md4c/md4c.c lib/md4c/md4c-html.c
LIBS=$(wildcard $(LIB_DIR)/*.c)
OBJS=$(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
MD4C_OBJS=$(MD4C_SRCS:$(LIB_DIR)/%.c=$(BUILD_DIR)/%.o)
LIB_OBJS=$(LIBS:$(LIB_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR)/blog_server

$(BUILD_DIR)/blog_server: $(OBJS) $(MD4C_OBJS) $(LIB_OBJS)
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean

CC=gcc
CFLAGS=-I./include -Wall -Wextra
SRC_DIR=src
BUILD_DIR=build
LIB_DIR=lib

SRCS=$(wildcard $(SRC_DIR)/*.c)
LIBS=$(wildcard $(LIB_DIR)/*.c)
OBJS=$(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
LIB_OBJS=$(LIBS:$(LIB_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR)/blog_server

$(BUILD_DIR)/blog_server: $(OBJS) $(LIB_OBJS)
	$(CC) $(OBJS) $(LIB_OBJS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean

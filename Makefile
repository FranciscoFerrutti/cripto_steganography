# Compiler settings
CC := gcc
CFLAGS := -std=c11 -pedantic -pedantic-errors -pthread -g -Wall -Wextra -D_POSIX_C_SOURCE=200809L -Werror -fsanitize=address -Iinclude
LDFLAGS := -lcrypto

# Directories
SRC_DIR := src
BUILD_DIR := build
INCLUDE_DIR := include

# Sources and Objects
LIB_SRCS := $(SRC_DIR)/core/embed.c $(SRC_DIR)/core/extract.c $(SRC_DIR)/core/parse_args.c $(SRC_DIR)/utils/bitmap.c $(SRC_DIR)/utils/encryption.c
LIB_OBJS := $(LIB_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
MAIN_SRC := $(SRC_DIR)/main.c
MAIN_OBJ := $(BUILD_DIR)/main.o
VPATH := $(SRC_DIR)/core $(SRC_DIR)/utils $(SRC_DIR)

# Output executable
STEGOBMP_CLI := stegobmp

.PHONY: all clean

# Default target
all: $(STEGOBMP_CLI)

# Link objects to create the executable
$(STEGOBMP_CLI): $(LIB_OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $(LIB_OBJS) $(MAIN_OBJ) -o $@ $(LDFLAGS)

# Compile each source file to an object file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)   # Create necessary directories for object files
	$(CC) -c $(CFLAGS) $< -o $@

# Special target for main.o
$(BUILD_DIR)/main.o: $(MAIN_SRC)
	@mkdir -p $(dir $@)   # Create necessary directories for main.o
	$(CC) -c $(CFLAGS) $< -o $@

# Clean build files
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR) $(STEGOBMP_CLI)

# Compiler settings
CC := gcc
CFLAGS := -std=c11 -pedantic -pedantic-errors -pthread -g -Wall -Wextra -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -Werror -fsanitize=address -Iinclude
LDFLAGS := -lcrypto

# Colors
GREEN := \033[0;32m
YELLOW := \033[0;33m
BLUE := \033[0;34m
RED := \033[0;31m
NC := \033[0m # No Color


# Directories
SRC_DIR := src
BUILD_DIR := build
INCLUDE_DIR := include

# Sources and Objects
LIB_SRCS := $(wildcard $(SRC_DIR)/*/*.c) $(wildcard $(SRC_DIR)/core/*/*.c) 
LIB_OBJS := $(LIB_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
MAIN_SRC := $(SRC_DIR)/main.c
MAIN_OBJ := $(BUILD_DIR)/main.o
VPATH := $(SRC_DIR)/core $(SRC_DIR)/utils $(SRC_DIR)

# Output executable
STEGOBMP_CLI := stegobmp

.PHONY: all clean valgrind
# Default target
all: $(STEGOBMP_CLI)
	@echo  "$(GREEN)Build successful!$(NC)"

# Link objects to create the executable
$(STEGOBMP_CLI): $(LIB_OBJS) $(MAIN_OBJ)
	@echo  "$(BLUE)Linking objects to create the executable$(NC)"
	@$(CC) $(CFLAGS) $(LIB_OBJS) $(MAIN_OBJ) -o $@ $(LDFLAGS)
	@echo  "$(GREEN)Executable $(STEGOBMP_CLI) created successfully!$(NC)"

# Compile each source file to an object file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)   
	@echo  "$(YELLOW)Compiling $< $(NC)"
	@$(CC) -c $(CFLAGS) $< -o $@

# Special target for main.o
$(BUILD_DIR)/main.o: $(MAIN_SRC)
	@mkdir -p $(dir $@)   
	@echo  "$(YELLOW)Compiling main source file$(NC)"
	@$(CC) -c $(CFLAGS) $< -o $@

# Clean build files
clean:
	@echo  "$(BLUE)Cleaning build directory$(NC)"
	@rm -rf $(BUILD_DIR) $(STEGOBMP_CLI)
	@echo  "$(GREEN)Clean complete!$(NC)"

# Run valgrind on the executable
valgrind: $(STEGOBMP_CLI)
	@echo "$(BLUE)Running Valgrind for memory leak detection...$(NC)"
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./$(STEGOBMP_CLI) -i test.bmp -o out.bmp -f -b 1 -s "Hello, World!"
	@echo "$(GREEN)Valgrind completed! Check valgrind-out.txt for details.$(NC)"
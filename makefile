# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -Iinclude

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = tests

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
TEST_FILES = $(wildcard $(TEST_DIR)/*.c)

# Object files
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TEST_OBJ_FILES = $(TEST_FILES:$(TEST_DIR)/%.c=$(BUILD_DIR)/test_%.o)

# Executables
EXEC = $(BUILD_DIR)/main
TEST_EXEC = $(BUILD_DIR)/test_exec

# OS detection
ifeq ($(OS),Windows_NT)
    MKDIR = if not exist $(subst /,\,$(BUILD_DIR)) mkdir $(subst /,\,$(BUILD_DIR))
    RM = del /Q /F $(subst /,\,$(BUILD_DIR))\\*
    EXEC_EXT = .exe
else
    MKDIR = mkdir -p $(BUILD_DIR)
    RM = rm -rf $(BUILD_DIR)/*
    EXEC_EXT =
endif

# Default target
all: $(EXEC)$(EXEC_EXT)

# Linking main executable
$(EXEC)$(EXEC_EXT): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $@

# Linking test executable
$(TEST_EXEC)$(EXEC_EXT): $(OBJ_FILES) $(TEST_OBJ_FILES)
	$(CC) $(OBJ_FILES) $(TEST_OBJ_FILES) -o $@

# Compiling source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(MKDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compiling test files
$(BUILD_DIR)/test_%.o: $(TEST_DIR)/%.c
	$(MKDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Test target
test: $(TEST_EXEC)$(EXEC_EXT)

# Clean target
clean:
	$(RM)

# PHONY targets
.PHONY: all clean test

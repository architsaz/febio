# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic -std=c99 -O -g -fsanitize=address \
         -fsanitize=undefined -fstack-protector -Wshadow -Wpointer-arith \
         -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wconversion \
         -Wformat=2 -I/usr/include -Iinclude

LDFLAGS = -L/usr/lib -lz -lm -fsanitize=address -fsanitize=undefined

# Directories
SRC_DIR = src
SHARED_DIR = $(SRC_DIR)/shared
INC_DIR = include
BUILD_DIR = build
TEST_DIR = test

# Project-specific directories and files
FEB_SRC_FILES = $(wildcard $(SRC_DIR)/feb/*.c)
PPA_SRC_FILES = $(wildcard $(SRC_DIR)/ppa/*.c)
SHARED_SRC_FILES = $(wildcard $(SHARED_DIR)/*.c)
TEST_FILES = $(wildcard $(TEST_DIR)/*.c)


# Object files for each project
FEB_OBJ_FILES = $(FEB_SRC_FILES:$(SRC_DIR)/feb/%.c=$(BUILD_DIR)/feb_%.o) \
                $(SHARED_SRC_FILES:$(SHARED_DIR)/%.c=$(BUILD_DIR)/shared_%.o)

PPA_OBJ_FILES = $(PPA_SRC_FILES:$(SRC_DIR)/ppa/%.c=$(BUILD_DIR)/ppa_%.o) \
                $(SHARED_SRC_FILES:$(SHARED_DIR)/%.c=$(BUILD_DIR)/shared_%.o)

TEST_OBJ_FILES = $(TEST_FILES:$(TEST_DIR)/%.c=$(BUILD_DIR)/test_%.o) \
                 $(SHARED_SRC_FILES:$(SHARED_DIR)/%.c=$(BUILD_DIR)/shared_%.o)

# Executables for each project
FEB_EXEC = $(BUILD_DIR)/febmkr_exec
PPA_EXEC = $(BUILD_DIR)/ppa_exec
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
all: feb ppa

# Project-specific targets
feb: $(FEB_EXEC)$(EXEC_EXT)

ppa: $(PPA_EXEC)$(EXEC_EXT)

# Linking executables for each project
$(FEB_EXEC)$(EXEC_EXT): $(FEB_OBJ_FILES)
	$(CC) $(FEB_OBJ_FILES) $(LDFLAGS) -o $@

$(PPA_EXEC)$(EXEC_EXT): $(PPA_OBJ_FILES)
	$(CC) $(PPA_OBJ_FILES) $(LDFLAGS) -o $@

# Linking test executable
$(TEST_EXEC)$(EXEC_EXT): $(TEST_OBJ_FILES)
	$(CC) $(TEST_OBJ_FILES) $(LDFLAGS) -o $@

# Compiling source files for each project
$(BUILD_DIR)/feb_%.o: $(SRC_DIR)/feb/%.c
	$(MKDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ppa_%.o: $(SRC_DIR)/ppa/%.c
	$(MKDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compiling shared files
$(BUILD_DIR)/shared_%.o: $(SHARED_DIR)/%.c
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
.PHONY: all clean test feb ppa

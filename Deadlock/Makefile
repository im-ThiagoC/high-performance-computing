# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -D_WIN32_WINNT=0x0600

# Libraries (e.g., pthread)
LIBS = -pthread -

# Source files
#SRCS = main.c
SRCS += producer_consumers.c
SRCS += hungry_philosophers.c

# Target executable
TARGET = deadlock_example.exe

# Build directory
BUILD_DIR = output

# Object files in build directory
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

# Default target
all: $(BUILD_DIR)/$(TARGET)

# Ensure build directory exists
$(BUILD_DIR):
	if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

# Link object files to create executable
$(BUILD_DIR)/$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(OBJS) -o $@ $(LIBS)

# Compile source files to object files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rmdir /S /Q $(BUILD_DIR) 2>nul

# Run the program
run: $(BUILD_DIR)/$(TARGET)
	.\$(BUILD_DIR)\$(TARGET)

.PHONY: all clean run
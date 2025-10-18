# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -D_WIN32_WINNT=0x0600 -fopenmp

# Libraries (e.g., pthread)
LIBS = -pthread

# Source files
#SRCS = main.c
GAME_OF_LIFE 	= game_of_life.c

# Omp version
GAME_OF_LIFE_OMP 	= game_of_life_omp.c

# Target executable
TARGETS = $(BUILD_DIR)/game_of_life.exe 			\
          $(BUILD_DIR)/game_of_life_omp.exe

# Build directory
BUILD_DIR = output

# Object files in build directory
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

# Default target
all: $(TARGETS)

# Ensure build directory exists
$(BUILD_DIR):
	if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

# Link object files to create executable
$(BUILD_DIR)/$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(OBJS) -o $@ $(LIBS)

# Compile source files to object files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build rules for each executable
$(BUILD_DIR)/game_of_life.exe: $(GAME_OF_LIFE) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

$(BUILD_DIR)/game_of_life_omp.exe: $(GAME_OF_LIFE_OMP) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

# Clean build artifacts
clean:
	rmdir /S /Q $(BUILD_DIR) 2>nul

# Run the program
run: $(BUILD_DIR)/$(TARGET)
	.\$(BUILD_DIR)\$(TARGET)

.PHONY: all clean run
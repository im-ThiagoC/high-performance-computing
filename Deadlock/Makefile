# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -D_WIN32_WINNT=0x0600 -fopenmp

# Libraries (e.g., pthread)
LIBS = -pthread

# Source files
#SRCS = main.c
PRODUCER_CONSUMERS 	= producer_consumers.c
HUNGRY_PHILOSOPHERS = hungry_philosophers.c

# Omp version
PRODUCER_CONSUMERS_OMP 	= producer_consumers_omp.c
HUNGRY_PHILOSOPHERS_OMP = hungry_philosophers_omp.c

# Target executable
TARGETS = $(BUILD_DIR)/producer_consumers.exe 			\
          $(BUILD_DIR)/hungry_philosophers.exe			\
          $(BUILD_DIR)/producer_consumers_omp.exe 	\
          $(BUILD_DIR)/hungry_philosophers_omp.exe

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
$(BUILD_DIR)/producer_consumers.exe: $(PRODUCER_CONSUMERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

$(BUILD_DIR)/hungry_philosophers.exe: $(HUNGRY_PHILOSOPHERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

$(BUILD_DIR)/producer_consumers_omp.exe: $(PRODUCER_CONSUMERS_OMP) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

$(BUILD_DIR)/hungry_philosophers_omp.exe: $(HUNGRY_PHILOSOPHERS_OMP) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

# Clean build artifacts
clean:
	rmdir /S /Q $(BUILD_DIR) 2>nul

# Run the program
run: $(BUILD_DIR)/$(TARGET)
	.\$(BUILD_DIR)\$(TARGET)

.PHONY: all clean run
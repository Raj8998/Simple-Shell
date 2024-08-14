# Makefile for compiling shell.c

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wformat-security -g

# Source files
SRCS = shell.c

# Header files
HEADERS = runShellCommands.h tokenizer.h handlePipeCommands.h

# Executable name
TARGET = shell

# Default rule
all: $(TARGET)

# Rule to compile the executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean rule to remove compiled files
clean:
	rm -f $(TARGET) *.o

# PHONY targets
.PHONY: all clean

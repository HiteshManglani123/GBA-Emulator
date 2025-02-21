# Compiler
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g   # Enable warnings, optimization, and debugging

# List of source files
SRCS = gba.c setup.c
OBJS = $(SRCS:.c=.o)  # Convert .c files to .o files automatically

# Output binary
TARGET = gba_emulator

# Default rule: compile everything
all: $(TARGET)

# Link the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Compile individual .c files to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)


CC = gcc
CFLAGS = -Wall -Wextra

# Target executable
TARGET = mysh

# Source files
SRCS = main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Header files
HDRS =  main.h

# Default target
all: $(TARGET)

# Compile source files into object files
%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files into the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Clean up object files and the target executable
clean:
	rm -f $(OBJS) $(TARGET)

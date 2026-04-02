# 1. Variables - Makes it easy to change compilers or flags later
CC = gcc
CFLAGS = -I./include -Wall -Wextra -g
TARGET = myshell

# 2. List of Source Files and Object Files
# This says: take every .c file and imagine it as a .o file
SRCS = src/main.c src/helpers.c src/parser.c src/executor.c src/builtins.c src/redirection.c
OBJS = $(SRCS:.c=.o)

# 3. The Default Rule (The "Goal")
# This runs when you just type 'make'
all: $(TARGET)

# 4. Linking Stage
# Links all .o files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# 5. Compilation Stage
# This is a "Pattern Rule". It says: 
# "To create any .o file, look for a .c file with the same name."
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 6. Cleanup Rule
# Run 'make clean' to start fresh
clean:
	rm -f $(OBJS) $(TARGET)

# 7. Phony Targets
# Tells make that 'all' and 'clean' aren't actual files
.PHONY: all clean

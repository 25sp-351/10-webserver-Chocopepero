# Compiler
CC = clang

# Compiler flags
CFLAGS = -Wall -g -I. -g3 -O0

# Target executable
EXEC = server
PORT ?= 8080

# Source files
SRCS = main.c tcp_server.c vec.c request.c response.c router.c
OBJS = $(SRCS:.c=.o)

# Default Target: Compile Program
all: $(EXEC)

# Compile Executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

# Compile Object Files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run Test
test: $(EXEC)
	@echo "Running tests on port $(PORT)..."
	@chmod +x server_test.sh
	@PORT=$(PORT) ./server_test.sh

# Clean Build Files
clean:
	rm -f $(OBJS) $(EXEC) server_output.log

# Phony Targets
.PHONY: all clean test
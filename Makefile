# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g  # Fixed typo (-Mextra → -Wextra)
LDFLAGS = -lpthread -lssl -lcrypto

# Targets
TARGETS = client server

# Client sources
CLIENT_SRC = client.c auth.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
CLIENT_DEPS = auth.h email.h  # Added email.h

# Server sources
SERVER_SRC = server.c email.c auth.c  # Added email.c and auth.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_DEPS = email.h auth.h  # Added headers

# Default target
all: $(TARGETS)

# Client build rule
client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Server build rule
server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Pattern rule for object files
%.o: %.c $(SERVER_DEPS) $(CLIENT_DEPS)  # Now tracks all headers
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean build artifacts
clean:
	rm -f $(TARGETS) *.o

# Run targets
run-client: client
	./client

run-server: server
	./server

.PHONY: all clean run-client run-server  # Fixed typo (.PHOW → .PHONY)

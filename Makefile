# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread -lssl -lcrypto  # For pthread and OpenSSL

# Targets
TARGETS = client server

# Client source and dependencies
CLIENT_SRC = client.c auth.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
CLIENT_DEPS = auth.h

# Server source
SERVER_SRC = server.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)

all: $(TARGETS)

client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(CLIENT_DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGETS) *.o

run-client: client
	./client

run-server: server
	./server

.PHONY: all clean run-client run-server

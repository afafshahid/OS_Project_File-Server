# Makefile for Mail Server components

CC = gcc
CFLAGS = -Wall -pthread
LDFLAGS = -lrt -pthread -lcrypto

# Target executables
all: socket_server socket_client email_writer

# Using the correct source file names
socket_server:
	$(CC) $(CFLAGS) -o socket_server server.c email.c auth.c $(LDFLAGS)

socket_client:
	$(CC) $(CFLAGS) -o socket_client client.c email.c auth.c $(LDFLAGS)

email_writer:
	$(CC) $(CFLAGS) -o email_writer emailwriter.c email.c auth.c $(LDFLAGS)

clean:
	rm -f socket_server socket_client email_writer *.o

# Run individual components
run-server: socket_server
	./socket_server

run-client: socket_client
	./socket_client

run-writer: email_writer
	./email_writer

# Run all components in separate terminals
run: all
	@echo "Starting all components in separate terminals..."
	@gnome-terminal -- bash -c "./socket_server; exec bash" &
	@sleep 1
	@gnome-terminal -- bash -c "./email_writer; exec bash" &
	@sleep 1
	@gnome-terminal -- bash -c "./socket_client; exec bash" &

.PHONY: all clean run run-server run-client run-writer

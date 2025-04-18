#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "email.h"

#define PORT 2727
#define BACKLOG 5
#define MAX_USERNAME_LEN 50
#define MAX_EMAILS 10

void *handle_client(void *arg) 
{
    int client_socket = *(int *)arg;
    free(arg);  // Free the dynamically allocated socket
    char buffer[1024] = {0};
    char username[MAX_USERNAME_LEN] = {0};
    Email email;
    
    // 1. First receive the username from client
    int bytes_read = read(client_socket, username, sizeof(username));
    if (bytes_read <= 0) 
    {
        printf("Failed to receive username or client disconnected\n");
        close(client_socket);
        pthread_exit(NULL);
    }
    username[strcspn(username, "\n")] = '\0'; // Remove newline if present
    
    // 2. Send welcome message
    char welcome[150];
    snprintf(welcome, sizeof(welcome), "MAIL SERVER: Welcome %s!\n", username);
    send(client_socket, welcome, strlen(welcome), 0);

    printf("Client %s connected\n", username);

    while (1) 
    {
	memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(client_socket, buffer, sizeof(buffer));
	if (bytes_read <= 0) 
	{  // Check if connection closed or error
            printf("Client disconnected\n");
            break;
        }

        if (strncmp(buffer, "SEND_EMAIL", 10) == 0) 
	{
            // Receive email data
            read(client_socket, &email, sizeof(Email));
            send_email(email);  // Your function
            write(client_socket, "EMAIL_STORED", 12);
        }

        else if (strncmp(buffer, "FETCH_INBOX", 11) == 0) 
	{
           Email emails[MAX_EMAILS];
           int count = fetch_emails("Inbox", emails, username);  // Pass username
    
            // Send only emails where receiver = username
            int user_email_count = 0;
            Email user_emails[MAX_EMAILS];
            for (int i = 0; i < count; i++) 
	    {
             if (strcmp(emails[i].receiver, username) == 0) 
	     {  // Filter
              user_emails[user_email_count++] = emails[i];
             }
            }
    
           // Send filtered emails
           write(client_socket, &user_email_count, sizeof(int));
           for (int i = 0; i < user_email_count; i++) 
	   {
            write(client_socket, &user_emails[i], sizeof(Email));
           }
        }
        
	else if (strncmp(buffer, "exit", 4) == 0) 
	{
            break;
        }
           
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main() 
{
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // 1. Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) 
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Listen
    if (listen(server_fd, BACKLOG) < 0) 
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // 4. Accept loop using threads
    while (1) 
    {
        int *client_socket = malloc(sizeof(int));
        *client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (*client_socket < 0) 
        {
            perror("Accept failed");
            free(client_socket);
            continue;
        }

        printf("New connection from %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, client_socket) != 0) 
        {
            perror("Thread creation failed");
            close(*client_socket);
            free(client_socket);
        }

        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}

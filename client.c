#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <openssl/sha.h>
#include "auth.h"
#include "email.h"


#define PORT 2727
#define SERVER_IP "127.0.0.1"

void send_emails(int sock) 
{
    Email email;
    printf("From: ");
    fgets(email.sender, sizeof(email.sender), stdin);
    printf("To: ");
    fgets(email.receiver, sizeof(email.receiver), stdin);
    printf("Subject: ");
    fgets(email.subject, sizeof(email.subject), stdin);
    printf("Body: ");
    fgets(email.body, sizeof(email.body), stdin);

    // Clean inputs
    email.sender[strcspn(email.sender, "\n")] = '\0';
    email.receiver[strcspn(email.receiver, "\n")] = '\0';
    email.subject[strcspn(email.subject, "\n")] = '\0';
    email.body[strcspn(email.body, "\n")] = '\0';
    strcpy(email.folder, "Inbox");

    // Send to server
    write(sock, "SEND_EMAIL", 10);
    write(sock, &email, sizeof(Email));

    char response[20];
    read(sock, response, sizeof(response));
    printf("Server: %s\n", response);
}

void fetch_inbox(int sock) 
{
    // Send fetch command
    if (write(sock, "FETCH_INBOX", 11) < 0) {
        perror("Failed to send fetch command");
        return;
    }

    // Get count of emails
    int count;
    if (read(sock, &count, sizeof(int)) != sizeof(int)) {
        perror("Failed to read email count");
        return;
    }

    if (count == 0) {
        printf("\nYour inbox is empty.\n");
        return;
    }

    printf("\n--- Your Inbox (%d emails) ---\n", count);
    
    // Receive and display each email
    for (int i = 0; i < count; i++) {
        Email email;
        ssize_t bytes_read = read(sock, &email, sizeof(Email));
        
        if (bytes_read != sizeof(Email)) {
            printf("Error: Incomplete email received\n");
            break;
        }

        printf("\nEmail %d:\n", i+1);
        printf("From: %s\n", email.sender);
        printf("Subject: %s\n", email.subject);
        printf("Body: %s\n", email.body);
        printf("----------------------------");
    }
}

int main() 
{
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[1024] = {0};
    char username[50], password[50];
    int option;

    // 1. Create socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) 
    {
        perror("Socket creation failed");
        exit(1);
    }

    // 2. Initialize server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    printf("1) Login \n2) Create Account \nChoose Option: ");
    scanf("%d", &option);
    getchar(); // Consume the newline character left by scanf

    if (option == 1 || option == 2) 
    {
        printf("Enter username: ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = '\0'; // Remove newline

        printf("Enter password: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = '\0'; // Remove newline

        if (option == 2) 
        {
            create_user(username, password);
            printf("User Registered!\n");
        }

        if (verify_user(username, password)) 
        {
            printf("Login successful!\n");

            // 3. Connect to server
            if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
            {
                perror("Connection failed");
                close(client_fd);
                exit(1);
            }
            printf("Connected to Mail Server\n");

            // Send username to server first
            send(client_fd, username, strlen(username), 0);

            // 4. Receive Welcome message
            if (read(client_fd, buffer, sizeof(buffer)) < 0) 
            {
                perror("Read failed");
                close(client_fd);
                exit(1);
            }
            printf("Server: %s", buffer);

            // 5. Main communication loop
            while (1) 
            {
                  printf("\n1. Send Email\n2. Fetch Inbox\n3. Exit\n> ");
                  int choice;
                  scanf("%d", &choice);
                  getchar(); // Clear newline
        
                if (choice == 1) send_emails(client_fd);
                else if (choice == 2) fetch_inbox(client_fd);
                else if (choice == 3)
	       	{
                 write(client_fd, "exit", 4);
                 break;
                }
            }
            close(client_fd);
        } 
        else 
        {
            printf("Invalid username or password.\n");
        }
    } 
    else 
    {
        printf("Invalid Option\n");
    }

    return 0;
}

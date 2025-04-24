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

char username[50];

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

void fetch_inbox(int sock) {
    int current_page = 0;
    int total_pages = 0;

    while (1) {
        char fetch_command[20];
        sprintf(fetch_command, "FETCH_INBOX %d", current_page);

        // Send fetch command with page number
        if (write(sock, fetch_command, strlen(fetch_command)) < 0) {
            perror("Failed to send fetch command");
            return;
        }

        // Get count of emails and pagination info
        int count, page, total;
        if (read(sock, &count, sizeof(int)) != sizeof(int) ||
            read(sock, &page, sizeof(int)) != sizeof(int) ||
            read(sock, &total_pages, sizeof(int)) != sizeof(int)) {
            perror("Failed to read email count or pagination info");
            return;
        }

        if (count == 0) {
            if (current_page == 0) {
                printf("\nYour inbox is empty.\n");
            } else {
                printf("\nNo more emails on this page.\n");
            }
        } else {
            printf("\n--- Your Inbox (Page %d of %d, showing %d emails) ---\n",
                   current_page + 1, total_pages, count);

            // Receive and display each email
            for (int i = 0; i < count; i++) {
                Email email;
                ssize_t bytes_read = read(sock, &email, sizeof(Email));

                if (bytes_read != sizeof(Email)) {
                    printf("Error: Incomplete email received\n");
                    break;
                }

                printf("\nEmail %d:\n", (current_page * 4) + i + 1);
                printf("From: %s\n", email.sender);
                printf("Subject: %s\n", email.subject);
                printf("Body: %s\n", email.body);
                printf("----------------------------");
            }
        }

        printf("\n\nOptions: [n]ext page, [p]revious page, [q]uit viewing: ");
        char choice;
        scanf(" %c", &choice);

        if (choice == 'n' && current_page < total_pages - 1) {
            current_page++;
        } else if (choice == 'p' && current_page > 0) {
            current_page--;
        } else if (choice == 'q') {
            break;
        }
    }
}

int main() 
{
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[1024] = {0};
    char password[50];
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

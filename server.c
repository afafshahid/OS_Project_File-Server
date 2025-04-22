#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "email.h"

#define PORT 2727
#define BACKLOG 5
#define MAX_USERNAME_LEN 50
#define MAX_EMAILS 10
#define QUEUE_NAME "/mail_queue"
#define MAX_QUEUE_SIZE 10
#define QUEUE_PERMS 0644

// Global synchronization primitives
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
mqd_t mail_queue;

void initialize_message_queue() {
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_QUEUE_SIZE,
        .mq_msgsize = sizeof(Email),
        .mq_curmsgs = 0
    };

    // Create or open the message queue
    mail_queue = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, QUEUE_PERMS, &attr);
    if (mail_queue == -1) {
        perror("mq_open failed");
        exit(EXIT_FAILURE);
    }
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);
    char buffer[1024] = {0};
    char username[MAX_USERNAME_LEN] = {0};
    Email email;

    // Get username
    if (read(client_socket, username, sizeof(username)) <= 0) {
        perror("Username read failed");
        close(client_socket);
        pthread_exit(NULL);
    }
    username[strcspn(username, "\n")] = '\0';

    // Send welcome message
    char welcome[150];
    snprintf(welcome, sizeof(welcome), "MAIL SERVER: Welcome %s!\n", username);
    if (send(client_socket, welcome, strlen(welcome), 0) < 0) {
        perror("Welcome message send failed");
    }

    printf("Client %s connected\n", username);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            printf("%s disconnected\n", username);
            break;
        }

        if (strncmp(buffer, "SEND_EMAIL", 10) == 0) {
            // Receive email data
            if (read(client_socket, &email, sizeof(Email)) != sizeof(Email)) {
                perror("Email receive failed");
                continue;
            }

            // Thread-safe message queue submission
            pthread_mutex_lock(&client_mutex);
            if (mq_send(mail_queue, (const char *)&email, sizeof(Email), 0) == -1) {
                perror("mq_send failed");
                write(client_socket, "ERROR_SENDING", 13);
            } else {
                write(client_socket, "EMAIL_QUEUED", 12);
            }
            pthread_mutex_unlock(&client_mutex);
        }
        else if (strncmp(buffer, "FETCH_INBOX", 11) == 0) {
            Email emails[MAX_EMAILS];
            int count = fetch_emails("Inbox", emails, username);

            // Filter emails for this user
            Email user_emails[MAX_EMAILS];
            int user_count = 0;
            for (int i = 0; i < count && user_count < MAX_EMAILS; i++) {
                if (strcmp(emails[i].receiver, username) == 0) {
                    user_emails[user_count++] = emails[i];
                }
            }

            // Send response
            if (write(client_socket, &user_count, sizeof(int)) < 0) {
                perror("Count send failed");
                continue;
            }

            for (int i = 0; i < user_count; i++) {
                if (write(client_socket, &user_emails[i], sizeof(Email)) < 0) {
                    perror("Email send failed");
                    break;
                }
            }
        }
        else if (strncmp(buffer, "exit", 4) == 0) {
            printf("%s requested disconnection\n", username);
            break;
        }
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Initialize message queue
    initialize_message_queue();

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind and listen
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Main accept loop
    while (1) {
        int *client_socket = malloc(sizeof(int));
        if (!client_socket) {
            perror("Malloc failed");
            continue;
        }

        *client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (*client_socket < 0) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }

        printf("New connection from %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, client_socket) != 0) {
            perror("Thread creation failed");
            close(*client_socket);
            free(client_socket);
            continue;
        }

        pthread_detach(tid);
    }

    // Cleanup (unreachable in this loop)
    mq_close(mail_queue);
    mq_unlink(QUEUE_NAME);
    close(server_fd);
    pthread_mutex_destroy(&client_mutex);
    return 0;
}

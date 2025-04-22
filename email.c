#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "email.h"

#define FILE_NAME "emails.txt"
#define MAX_EMAILS 10

// Initialize synchronization primitives
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_queue(EmailQueue *queue) {
    queue->front = queue->rear = NULL;
    pthread_mutex_init(&queue->queue_mutex, NULL);
    sem_init(&queue->queue_sem, 0, 0);  // Initialize with 0 items
}

void enqueue(EmailQueue *queue, Email email) {
    EmailNode *new_node = (EmailNode*)malloc(sizeof(EmailNode));
    new_node->email = email;
    new_node->next = NULL;

    pthread_mutex_lock(&queue->queue_mutex);
    
    if (queue->rear == NULL) {
        queue->front = queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }

    sem_post(&queue->queue_sem);  // Increment item count
    pthread_mutex_unlock(&queue->queue_mutex);
}

int dequeue(EmailQueue *queue, Email *email) {
    sem_wait(&queue->queue_sem);  // Wait for available item
    pthread_mutex_lock(&queue->queue_mutex);

    if (queue->front == NULL) {
        pthread_mutex_unlock(&queue->queue_mutex);
        return 0;
    }

    EmailNode *temp = queue->front;
    *email = temp->email;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    free(temp);
    pthread_mutex_unlock(&queue->queue_mutex);
    return 1;
}

int is_empty(EmailQueue *queue) {
    pthread_mutex_lock(&queue->queue_mutex);
    int empty = (queue->front == NULL);
    pthread_mutex_unlock(&queue->queue_mutex);
    return empty;
}

void send_email(Email email) {
    pthread_mutex_lock(&file_mutex);
    
    FILE *file = fopen(FILE_NAME, "a");
    if (file == NULL) {
        pthread_mutex_unlock(&file_mutex);
        printf("Error opening file!\n");
        return;
    }

    fprintf(file, "%s|%s|%s|%s|%s\n", 
           email.sender, email.receiver, 
           email.subject, email.body, email.folder);
    
    fflush(file);  // Ensure data is written
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
    
    printf("Email stored successfully!\n");
}

int fetch_emails(const char *folder, Email *emails, const char *username) {
    pthread_mutex_lock(&file_mutex);
    FILE *file = fopen(FILE_NAME, "r");
    if (!file) {
        pthread_mutex_unlock(&file_mutex);
        return 0;
    }

    EmailQueue queue;
    init_queue(&queue);

    char line[2000];
    while (fgets(line, sizeof(line), file) {
        Email email;
        if (sscanf(line, "%99[^|]|%99[^|]|%199[^|]|%999[^|]|%49[^\n]",
                  email.sender, email.receiver,
                  email.subject, email.body, email.folder) == 5) {
            if (strcmp(email.folder, folder) == 0 && 
                strcmp(email.receiver, username) == 0) {
                enqueue(&queue, email);
            }
        }
    }
    fclose(file);
    pthread_mutex_unlock(&file_mutex);

    int count = 0;
    while (!is_empty(&queue) && count < MAX_EMAILS) {
        dequeue(&queue, &emails[count++]);
    }

    return count;
}

#ifndef EMAIL_H
#define EMAIL_H

#include <pthread.h>
#include <semaphore.h>

// Email structure to hold individual email data
typedef struct {
    char sender[100];
    char receiver[100];
    char subject[200];
    char body[1000];
    char folder[50];
} Email;

typedef struct EmailNode {
    Email email;
    struct EmailNode *next;
} EmailNode;

typedef struct {
    EmailNode *front;
    EmailNode *rear;
    
    // Added synchronization primitives
    pthread_mutex_t queue_mutex;  // Protects queue operations
    sem_t queue_sem;             // Tracks items in queue
} EmailQueue;

// Added file operation synchronization
extern pthread_mutex_t file_mutex;

// Original functions remain unchanged
void send_email(Email email);
int fetch_emails(const char *folder, Email *emails, const char *username);
void init_queue(EmailQueue *queue);
void enqueue(EmailQueue *queue, Email email);
int dequeue(EmailQueue *queue, Email *email);
int is_empty(EmailQueue *queue);

#endif

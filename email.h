#ifndef EMAIL_H
#define EMAIL_H

#include <pthread.h>
#include <semaphore.h>

typedef struct 
{
    char sender[100];
    char receiver[100];
    char subject[200];
    char body[1000];
    char folder[50];
} Email;

typedef struct EmailNode 
{
    Email email;
    struct EmailNode *next;
} EmailNode;

typedef struct 
{
    EmailNode *front;
    EmailNode *rear;
    pthread_mutex_t queue_mutex; 
    sem_t queue_sem;             
} EmailQueue;

extern pthread_mutex_t file_mutex;

void send_email(Email email);
int fetch_emails(const char *folder, Email *emails, const char *username, int page, int emails_per_page);
int get_total_email_count(const char *folder, const char *username);
void init_queue(EmailQueue *queue);
void enqueue(EmailQueue *queue, Email email);
int dequeue(EmailQueue *queue, Email *email);
int is_empty(EmailQueue *queue);

#endif

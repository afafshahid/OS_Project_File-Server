#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "email.h"

#define QUEUE_NAME "/mail_queue"
#define MAX_RETRIES 3
#define RETRY_DELAY 2  // seconds

void process_queue() 
{
    mqd_t mq;
    struct mq_attr attr;
    Email email;
    unsigned int prio;
    
    // Open queue in blocking mode
    mq = mq_open(QUEUE_NAME, O_RDONLY | O_CREAT, 0644, NULL);
    if (mq == -1) {
        perror("mq_open failed in writer");
        exit(EXIT_FAILURE);
    }

    // Get queue attributes
    if (mq_getattr(mq, &attr) == -1) {
        perror("mq_getattr failed");
        mq_close(mq);
        exit(EXIT_FAILURE);
    }

    printf("Writer started. Waiting for messages (PID: %d)...\n", getpid());

    while (1) {
        // BLOCKING receive (no timeout)
        ssize_t bytes = mq_receive(mq, (char*)&email, attr.mq_msgsize, &prio);
        
        if (bytes == -1) {
            perror("mq_receive failed");
            break;  // Exit on serious error
        }

        printf("Processing email from %s to %s...\n", email.sender, email.receiver);

        // Process email with retries
        int retry = 0;
        while (retry < MAX_RETRIES) {
            pthread_mutex_lock(&file_mutex);
            FILE *file = fopen("emails.txt", "a");
            if (!file) {
                pthread_mutex_unlock(&file_mutex);
                perror("Failed to open email file");
                retry++;
                sleep(RETRY_DELAY);
                continue;
            }

            fprintf(file, "%s|%s|%s|%s|%s\n",
                   email.sender, email.receiver,
                   email.subject, email.body, email.folder);
            
            fflush(file);
            fsync(fileno(file));
            fclose(file);
            pthread_mutex_unlock(&file_mutex);
            
            printf("Successfully stored email\n");
            break;
        }

        if (retry == MAX_RETRIES) {
            fprintf(stderr, "FAILED to store email after %d attempts\n", MAX_RETRIES);
        }
    }

    mq_close(mq);
}

int main() 
{
    process_queue();
    return 0;
}

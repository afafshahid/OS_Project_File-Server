#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include "email.h"

#define QUEUE_NAME "/mail_queue"
#define MAX_RETRIES 3
#define RETRY_DELAY 2  // seconds

// Global synchronization
extern pthread_mutex_t file_mutex;

void process_queue() {
    mqd_t mq;
    struct mq_attr attr;
    Email email;
    unsigned int prio;
    
    // Open existing queue
    mq = mq_open(QUEUE_NAME, O_RDONLY);
    if (mq == -1) {
        perror("mq_open failed in writer");
        exit(EXIT_FAILURE);
    }

    // Get queue attributes
    mq_getattr(mq, &attr);
    printf("Writer started. Queue contains %ld messages\n", attr.mq_curmsgs);

    while (1) {
        // Receive with timeout (5 seconds)
        struct timespec timeout = { .tv_sec = 5, .tv_nsec = 0 };
        ssize_t bytes = mq_timedreceive(mq, (char*)&email, attr.mq_msgsize, &prio, &timeout);
        
        if (bytes == -1) {
            if (errno == ETIMEDOUT) {
                // Timeout is normal - just loop again
                continue;
            }
            perror("mq_receive failed");
            break;
        }

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
            
            printf("Stored email from %s to %s\n", email.sender, email.receiver);
            break;  // Success
        }

        if (retry == MAX_RETRIES) {
            fprintf(stderr, "FAILED to store email after %d attempts\n", MAX_RETRIES);
        }
    }

    mq_close(mq);
}

int main() {
    // Initialize sync primitives (defined in email.c)
    init_sync_primitives();
    
    // Daemonize (optional)
    if (fork() != 0) return 0;  // Parent exits
    
    // Process messages forever
    process_queue();
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "email.h"

#define FILE_NAME "emails.txt"
#define MAX_EMAILS 10

void send_email(Email email) 
{
    FILE *file = fopen(FILE_NAME, "a");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    fprintf(file, "%s|%s|%s|%s|%s\n", email.sender, email.receiver, email.subject, email.body, email.folder);
    fclose(file);
    printf("Email stored successfully!\n");
}

int fetch_emails(const char *folder, Email *emails) 
{
    FILE *file = fopen(FILE_NAME, "r");
    if (!file) return 0;

    int count = 0;
    while (count < MAX_EMAILS && 
           fscanf(file, "%99[^|]|%99[^|]|%199[^|]|%499[^|]|%49[^\n]\n",
                  emails[count].sender,
                  emails[count].receiver,
                  emails[count].subject,
                  emails[count].body,
                  emails[count].folder) == 5) {
        if (strcmp(emails[count].folder, folder) == 0) {
            count++;
        }
    }
    fclose(file);
    return count;
}

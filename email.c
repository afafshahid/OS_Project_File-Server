#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "emails.txt"

typedef struct {
    char sender[100];
    char receiver[100];
    char subject[200];
    char body[500];
    char folder[50];  //Inbox, Sent, Trash
}Email;

//function to store an email in a file
void store_email(Email email){
    FILE *file=fopen(FILE_NAME, "a");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    fprintf(file, "%s|%s|%s|%s|%s\n", email.sender, email.receiver, email.subject, email.body, email.folder);
    fclose(file);
    printf("Email stored successfully!\n");
}

//function to fetch emails from a given folder
void fetch_emails(const char *folder) {
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL) {
        printf("No emails found!\n");
        return;
    }

    Email email;
    printf("\nEmails in %s:\n", folder);
    while (fscanf(file, "%99[^|]|%99[^|]|%199[^|]|%499[^|]|%49[^\n]\n",
                  email.sender, email.receiver, email.subject, email.body, email.folder) == 5) {
        if (strcmp(email.folder, folder) == 0) {
            printf("From: %s\nTo: %s\nSubject: %s\nBody: %s\n\n",
                   email.sender, email.receiver, email.subject, email.body);
        }
    }
   
    fclose(file);
}

int main() {
    Email email = {
        "afaf@gmail.com",
        "omaima@gmail.com",
        "Meeting Reminder",
        "Don't forget our meeting at 3 PM.",
        "Inbox"
    };

    //store an email
    store_email(email);

    //fetch emails from Inbox
    fetch_emails("Inbox");

    return 0;
}

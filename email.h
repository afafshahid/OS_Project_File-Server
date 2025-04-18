#ifndef EMAIL_H
#define EMAIL_H

#define FILE_NAME "emails.txt"

typedef struct {
    char sender[100];
    char receiver[100];
    char subject[200];
    char body[500];
    char folder[50];  // Inbox, Sent, Trash
} Email;

// Changed function name here
void send_email(Email email);  // Previously store_email//
int fetch_emails(const char *folder, Email *emails, const char *username);
#endif

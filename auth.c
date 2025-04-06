#include<stdio.h>
#include<openssl/sha.h>
#include<string.h>

void create_user(const char *username, const char *password) 
{
    FILE *file = fopen("users.txt", "a");
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256((unsigned char*)password, strlen(password), hash);

    fprintf(file, "%s ", username);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        fprintf(file, "%02x", hash[i]);
    }

    fprintf(file, "\n");

    fclose(file);
}


int verify_user(const char *username, const char *password) 
{
    FILE *file = fopen("users.txt", "r");
    char stored_username[100], stored_hash[65];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char password_hash[65];

    SHA256((unsigned char*)password, strlen(password), hash);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
        sprintf(&password_hash[i * 2], "%02x", hash[i]);
    }

    while (fscanf(file, "%s %s", stored_username, stored_hash) != EOF) 
    {
        if (strcmp(username, stored_username) == 0 && strcmp(password_hash, stored_hash) == 0)
       	{
            fclose(file);
            return 1; // Authentication successful
        }
    }

    fclose(file);
    return 0; // Authentication failed
}


     

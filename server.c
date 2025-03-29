#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>  //ip address,and func for conversion
#include<sys/socket.h>  //for sockets system call like socket(),bind(),listen()..
#include<sys/types.h>   
#include<netinet/in.h>  //struct sockaddr_in
#include<signal.h>      //handles any kind of delay

#define PORT 2727 //port num >1024 
#define BACKLOG 5  //max client in queue

void handle_client(int client_socket) {
    char buffer[1024] = {0};  //to store client msgs
    char *welcome = "MAIL SERVER AVAILABLE FOR COMMUNICATION\n";
    send(client_socket, welcome, strlen(welcome), 0); //sends welcome msg

    while(1) { //continuously reading and respond to client msg
        memset(buffer, 0, sizeof(buffer));  //clearing buffer before reading new data
        int bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            printf("Client Disconnected\n");
            break;
        }
        printf("Client: %s", buffer);
        if (strncmp(buffer, "AWAZ ARHI HAI", 13) == 0) {
            char *response = "JEE ARHI HAI\n";
            send(client_socket, response, strlen(response), 0);
        } 
        else if(strncmp(buffer,"exit",4)==0){
        printf("Shutdown command receive.Closing server..\n");
        close(client_socket);
        exit(0); //closing server
        }
        
        else {
            char *response = "HELLO AWAZ NHI ARHI\n";
            send(client_socket, response, strlen(response), 0);
           
        }
    }
    close(client_socket);
}

void handle_signal(int sig) {
    printf("ERROR: SERVER NOT FOUND\n");
    exit(0);
}

int main() {
    int server_fd;
    int client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    signal(SIGINT, handle_signal);

    // 1.Create Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    // 2.Initialize and Bind
    server_addr.sin_family = AF_INET;   //internet protocol version4
    server_addr.sin_addr.s_addr = INADDR_ANY;  //accepts connection from any ip(wi-fi,vpn..)
    server_addr.sin_port = htons(PORT); //converts port num(2525) from host byte order to network byte order

    //bind assigns server IP address and port no to socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // 3.Listen for incoming connection
    //marks socket as passive socket, it will wait for connection requests
    //backlog specifies max num of pending connections
    if (listen(server_fd, BACKLOG) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Server listening on port %d..\n", PORT);

    // 4.Accepts and handles Multiple Clients
    //accept waits for a client to connect, then creates new socket(client_socket) for it
    while (1) { 
        client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        printf("New client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Fork creates child process for each connected client
        // Child process handles client, while parent process waits for new clients
        if (fork() == 0) {
            close(server_fd); // Child doesn't need to listen for new connections
            handle_client(client_socket);
            exit(0);
        }
        close(client_socket); // Parent closes socket, child handles the client
    }
    return 0;
}


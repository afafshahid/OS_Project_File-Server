#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>

#define PORT 2727
#define SERVER_IP "127.0.0.1"

int main(){
int client_fd;
struct sockaddr_in server_addr;
char buffer[1024]={0};

//1.Create socket(client-side)
client_fd=socket(AF_INET,SOCK_STREAM,0);
if(client_fd == -1){
perror("Socket creation failed");
exit(1);
}

//2.Intialize server address and connect to server
server_addr.sin_family=AF_INET;
server_addr.sin_port=htons(PORT);
server_addr.sin_addr.s_addr=inet_addr(SERVER_IP);

if(connect(client_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
perror("Connection failed");
exit(1);
}
printf("Connected to Mail Server\n");

//4.Receive Welcome msg
read(client_fd,buffer,sizeof(buffer));
printf("Server:%s",buffer);

while(1){
printf("Enter any message or Type exit to disconnect:");
fgets(buffer,sizeof(buffer),stdin);

send(client_fd,buffer,strlen(buffer),0); //sends user msg to server

if(strncmp(buffer,"exit",4)==0){
printf("Disconnected from server");
break;
}

memset(buffer,0,sizeof(buffer));
read(client_fd,buffer,sizeof(buffer)); //reads server response
printf("Server:%s",buffer);
}
close(client_fd);
return 0;
}


/* Maximilian Guzman, gan022 */
/* Andrew Rodriguez, awa794 */
/* Jerome Daly, qco784*/
/* Server code */
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#define LISTEN_PORT 49000

typedef struct message {
    struct message * prevMessage;
    struct message * nextMessage;
    char thisMsg[125];
} message;

message * lastMessage = NULL;
message * oldMessage = NULL;
int curMessage = 0;

int handleDropped();
void insert(char * msg);
int length();
void removeTail();
void printList();
void *receiveMessage(void *sock);
void sig_chld(int signo);

int main(int argc, char *argv[]){
    int sock, i, nbytes, flags, size, addrlen, binded, ttl;
    int port = 22200;
    char *str_addr = (char *) malloc(11);
    FILE * fh;
    strcpy(str_addr, "239.10.5.");
    char message[125], buffer[100];
    struct sockaddr_in client;
    //struct sockaddr_in client;

    /* Pass in group number, set port and multicast group number/address */
    if (argc > 2){
        port += atoi(argv[1]);
        strcat(str_addr, argv[1]);
    } else {
        printf("Error: Must pass in group number and text file to read.\n");
	      return -1;
    }

    /* File pointer to read in text */
    fh = fopen(argv[2], "r");

    /* create a socket to send on */
    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0) {
        printf("socket error = %d\n", sock);
        return -1;
    }

    ttl = 5;
    int q = setsockopt(sock,IPPROTO_IP,IP_MULTICAST_TTL,&ttl,sizeof(ttl));
    if (q < 0){
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return -1;
    }

    client.sin_addr.s_addr = inet_addr(&str_addr[0]);
    client.sin_port = htons(port);
    client.sin_family = AF_INET;
    /* associate the socket with the address structure - this is called binding */
    binded = bind(sock, (struct sockaddr *) &client, sizeof(client));
    if( binded < 0) {
       printf("bind result: %d\n", binded);
       return -1;
    }

    i = 1;
    //initialize vars for receiveMessage thread
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, (void *)handleDropped, NULL);

    while(1){
        sleep(1);
        fgets(buffer, 100, fh);
        snprintf(message, sizeof(message),
            "%d---%s", i, buffer);
        printf("%s", message);
        nbytes=strlen(message);
        flags = 0;
        size =  sendto(sock, message, nbytes, flags,
            (struct sockaddr *) &client, sizeof(client));
        if (size < 0){
            printf("Error - sendto result: %d\n", size);
            printf("sock %d - message %s\nnbytes %d - flags %d\n",
                    sock, message, nbytes, flags);
            return -1;
        }else{
	         printf("sent message %d\n", i);
	      }
        i++;
        curMessage++;
        insert((char *)&message);
        //printList();
        if(length() > 10) removeTail();
    }
    close(sock);
    return 0;
}

int handleDropped(){
    // Variable declaration.
    int sock, nbytes, i, j, addrlen, size, connection, pid, flags;
    struct sockaddr_in server, from;
    socklen_t fromLen;
    char buffer[150], set_opt[100];
    
    // Establishing the server's settings.
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(LISTEN_PORT);
    

    // Creating our socket.
    sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sock < 0){
        printf("Error declaring socket, sock = %d, errno = %d\n", sock, errno);
        exit(-1);
    } else printf("Socket created!\n");

    // Setting our socket options.
    i = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)set_opt, sizeof(set_opt));
    if (i < 0){
        printf("Error in setting socket option\n");
        exit(-1);
    }

    // Binding the socket.
    i = bind(sock, (struct sockaddr *) &server, sizeof(server));
    if(i < 0){
        printf("Bind result: %d\n", i);
        close(sock);
        exit(-1);
    } else printf("Simple TCP server is ready!\n\n");

    // Listen for up to 3 connections.
    i = listen(sock, 3);
    if (i < 0){
        printf("Listen failed!\n");
        close(sock);
        exit(-1);
    } else printf("Server is listening!\n");

    signal(SIGCHLD,sig_chld);

    fromLen = sizeof(from);
    while(1 == 1){
        // Accept connections from users.
        connection = accept(sock, (struct sockaddr *) &from,(socklen_t *) &fromLen);
        if (connection < 0){
            printf("Accept failed! errno: %d\n", errno);
            close(sock);
            exit(-1);
        } else printf("I accepted a connection!\n");
        message * dropped;
        dropped = lastMessage;
        int listIndex = curMessage;  

        // Fork so the child process can handle the new connection while the parent continues accepting connections.
        pid = fork();
        // Child process block. Gets message, prints it, then sends back the message with indication that message was received.
        if( pid == 0 ){
            nbytes = 99;
            flags = 0;
            j = 0;
            close(sock);
            size = recv(connection, buffer, nbytes, flags);
            listIndex = curMessage - atoi(buffer);
            for (dropped = dropped; j < listIndex; dropped = dropped->prevMessage); 
            if(size<0){
                printf("Error in receiving data.\n");
                return -1;
            }
            printf("Received message: %s\n",buffer);
            flags = 0; 
            size = send(connection, dropped->prevMessage, 125,flags);
            close(connection);
            return 0;

        } else {
            // Parent block. Just handles if the fork fails.
            if (pid == -1){
            printf("Fork failed!\n");
            return -1;
            }
        // Close connection in both since parent does not need it and the child no longer needs it either.
        close(connection);
        }
    }
    // Close socket, exit execution.
    close(sock);

}

void sig_chld(int signo){
    pid_t pid;
    int stat;
    while(( pid=waitpid(-1,&stat,WNOHANG)) > 0);
    return;
}

/* void *receiveMessage(void *sock) {
    //continuously listen for the client to send missing line number
    while (1) {
        char server_reply[5];
        int missing, flags;
        char message[125];
        int addrlen = sizeof(client);
        int nbytes = 99;
        //Receive a reply from the client
        int size = recvfrom(sock, message, nbytes, flags,
              (struct sockaddr*)&client, &addrlen);
    		if (size < 0){
    			fprintf(stderr, "Error: recvfrom failed - %s\n", strerror(errno));
    			return -1;
    		}
        //convert the ascii to integer value
        missing = atoi(server_reply);
        //search LL for missing message and resend
        //
        //resend the missing message and error check
        size =  sendto(sock, message, nbytes, flags,
            (struct sockaddr *) &client, sizeof(client));
        if (size < 0){
          printf("ERROR - sendto failed with value: %d\n", size);
          return -1;
        }else{
	         printf("sent message %d\n", i);
	      }
    }
} */

void insert(char * msg){
    message * newMessage, tmp;
    newMessage = (message *) malloc(sizeof(message));
    strcpy(newMessage->thisMsg, msg);
    newMessage->nextMessage = NULL;
    if (lastMessage == NULL){
        lastMessage = newMessage;
        oldMessage = newMessage;
        newMessage->prevMessage = NULL;
        return;
    }
    lastMessage->nextMessage = newMessage;
    newMessage->prevMessage = lastMessage;
    lastMessage = newMessage;
}

int length() {
    int length = 0;
    struct message *cur;
    for(cur = lastMessage; cur != NULL; cur = cur->prevMessage) length++;
    return length;
}

void removeTail(){
    message * tmp;
    tmp = oldMessage;
    oldMessage = oldMessage->nextMessage;
    oldMessage->prevMessage = NULL;
    //printf("Removed %s\nNew tail %s\n", tmp->thisMsg, oldMessage->thisMsg);
    free(tmp);
}

void printList(){
    struct message *cur;
    printf("CURRENT LINKED LIST:\n");
    for(cur = lastMessage; cur != NULL; cur = cur->prevMessage) printf("%s", cur->thisMsg);
    printf("\n\n");
    return;
}

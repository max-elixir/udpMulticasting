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


typedef struct {
    message * prevMessage;
    message * nextMessage;
    char message[100];
} message;

message * lastMessage = NULL;
message * oldMessage = NULL;
int curMessage = 0;

void handleDropped();
void insert(char * msg);
int length();
void removeTail();

int main(int argc, char *argv[]){
    int sock, i, nbytes, flags, size, addrlen, binded, ttl;    
    int port = 22200;
    char *str_addr = (char *) malloc(11);
    strcpy(str_addr, "239.10.5.");  
    char message[50];
    struct sockaddr_in client;
    //struct sockaddr_in client;

    /* Pass in group number, set port and multicast group number/address */
    if (argc > 1){
        port += atoi(argv[1]);
        strcat(str_addr, argv[1]);
    } else {		
        printf("Error: Must pass in group number\n");
	return -1;
    }
   
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
    while(1){
        sleep(1);
        snprintf(message, sizeof(message), 
            "This is message %d from the Group %s beacon\n", i, argv[1]);
        //printf("%s", message);
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
    }

    return 0;
}

void insert(char * msg){
    message * newMessage, tmp;
    newMessage = (message *) malloc(sizeof(message));
    strcpy(newMessage->message, *msg);
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
    struct node *cur;
    for(cur = lastMessage; cur != NULL; cur = cur->prevMessage) length++;
    return length;
}

void remove(){
    message * tmp;
    tmp = oldMessage;
    oldMessage = oldMessage->nextMessage;
    free(tmp);
}

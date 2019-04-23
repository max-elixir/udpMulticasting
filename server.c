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


typedef struct message {
    struct message * prevMessage;
    struct message * nextMessage;
    char thisMsg[125];
} message;

message * lastMessage = NULL;
message * oldMessage = NULL;
int curMessage = 0;

void handleDropped();
void insert(char * msg);
int length();
void removeTail();
void printList();

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
        insert((char *)&message);
        //printList();
        if(length() > 10) removeTail();
    }

    return 0;
}

/*void handleDropped(){
    return;
    
    while(){

    }
}*/

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

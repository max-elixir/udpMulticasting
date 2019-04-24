/* Maximilian Guzman, gan022 */
/* Andrew Rodriguez, awa794 */
/* Jerome Daly, qco784 */
/* client code*/

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

int nbytes, flags;
struct sockaddr_in beacon;

int requestDropped(void *);

int main(int argc, char *argv[]){
    int sock, i, size;    
    int port = 22200;
    char *str_addr = (char *) malloc(11);
    strcpy(str_addr, "239.10.5.");  
    struct sockaddr_in client;
    char *message = (char *) malloc(99);
    memset(message, '\0', 99*sizeof(char));

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
 
    struct ip_mreq join_group;
    join_group.imr_interface.s_addr = htonl(INADDR_ANY);
    join_group.imr_multiaddr.s_addr = inet_addr(str_addr);
    /* join the specified multicast group */
    if(setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,
        (void *)&join_group, sizeof(join_group))){
        printf("Group joined failed with error %d\n",errno);
        return -1;
    }

    /*  Set socket to timeout after 2 seconds */
    struct timeval timeout={2,0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
		(char*)&timeout,sizeof(struct timeval));

    /* Fill in the address family and port for the client, then bind to socket. */
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    client.sin_family = AF_INET;                 
    client.sin_port = htons(port); 
    i = bind(sock, (struct sockaddr *) &client, sizeof(client));
    if( i < 0) {
        printf("bind result: %d\n", i);                      
        return -1;
    }  

    int count = 1;
    pthread_t reqThread;
    int addrlen, sentNum, sent, size2, nbytes2, prev=0, caught;
    char *token;
    char * copy = (char *) malloc(99);
    char * delimit = "---"; 
    while(1){
        addrlen = sizeof(beacon);
        nbytes = 99;
        size = recvfrom(sock, message, nbytes, flags, 
            (struct sockaddr*)&beacon, &addrlen);
	if(count < 2){
		printf("the beacon is set up for %s\n", inet_ntoa(beacon.sin_addr));
	}
	if (size < 0 && count>1){
		printf("message %d not received\n", prev+1);
		prev++;
                char request[5];
                sprintf(request, "%d", prev);
		pthread_create(&reqThread, NULL, (void *)requestDropped, (void *)request);	
		
	}else{	
		strcpy(copy, message);
        	strtok(message, delimit);
		sentNum = atoi(message);
		printf("message %d: Line %d ** Message %s", count, sentNum, copy);
		memset(message, '\0', 99*sizeof(char));
	}	
	prev = sentNum;
	count++;
    }
   

    return 0;
}

int requestDropped(void *requestMessage){
		int nbytes2, sock2, connected, sent, caught;
		struct sockaddr_in target_pc, me;
		char *request= (char *)malloc(5);
		request = (char *)requestMessage;
		nbytes2= 5;
		char *message2 = (char *) malloc(99);
		/* Establish TCP connection to Beacon*/
		/* create a socket to send on */
    		sock2 = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    		if(sock2 < 0) {
        		printf("socket error = %d errno = %d\n\n", sock2, errno);
        		return -1;
    		}	
			
    		/* we fill in the address-family, port, and ip address of server*/
    		target_pc.sin_family = AF_INET;                 
    		target_pc.sin_port = htons(49000);
    		target_pc.sin_addr.s_addr = beacon.sin_addr.s_addr;
 
    		/* fill in client address, port, and address-family */
    		me.sin_family = AF_INET;
    		me.sin_port = htons(0);
    		me.sin_addr.s_addr = htonl(INADDR_ANY);	

		/* Attempt to connect to the TCP server  */
    		connected = connect(sock2, (struct sockaddr *) &target_pc, 
			sizeof(target_pc));
    		if(connected < 0){
        		printf("Failed to connect: %d\n errno = %s\n", connected, strerror(errno));
        		return -1;
    		}	
		sent = send(sock2, request, nbytes2, flags);
		printf("i want message %s\n", request);
   	 	if (sent <= 0){
        		printf("Error when running \"send\": %d\n errno = %d\n", 
			sent, errno);
        		return -1;
    		}
		
    		caught =  recv(sock2, message2, nbytes, flags);
    		if( caught <= 0){
        		printf("Error when running \"recv\": %d\n errno = %s\n", 
				caught, strerror(errno));
        		return -1;
    		}
		printf("Recovered: %s\n", message2);
		memset(message2, '\0', 99*sizeof(char));
		close(sock2);	
		
		return 0;
}

/* Maximilian Guzman, gan022 */

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]){
    int sock, i, nbytes, flags, size;    
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
    #define Group join_group.imr_multiaddr.s_addr
    #define Interface join_group.imr_interface.s_addr
    /* join the specified multicast group */
    if(setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,
        (void *)&join_group, sizeof(join_group))){
        printf("Group joined failed with error %d\n",errno);
        return -1;
    }

    /* Fill in the address family and port. The inet_addr 
        function converts a string form of IP address to 
        a 32 binary integer in network order*/
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    client.sin_family = AF_INET;                 
    client.sin_port = htons(port);
    /* Comer is incorrect when he says bind is only for servers */
    i = bind(sock, (struct sockaddr *) &client, sizeof(client));
    if( i < 0) {
        printf("bind result: %d\n", i);                      
        return -1;
    }  

    struct sockaddr_in beacon;
    int count = 1;
    int addrlen;
    while(1){
        addrlen = sizeof(beacon);
        nbytes = 99;
        size = recvfrom(sock, message, nbytes, flags, 
            (struct sockaddr*)&beacon, &addrlen);
        printf("message %d: %s", count, message);
        count++;
    }
   

    return 0;
}

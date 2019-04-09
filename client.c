/* Maximilian Guzman, gan022 */

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define Group join_group.imr_multiaddr.s_addr
#define Interface join_group.imr_interface.s_addr

int main(int argc, char *argv[]){
    int sock, i, nbytes, flags, size;    
    int port = 22200;
    char *str_addr = (char *) malloc(11);
    strcpy(str_addr, "239.10.5.");  
    struct sockaddr_in target_pc;
    struct sockaddr_in me;
    
    /* Pass in group number, set port and multicast group number/address */
    if (argc > 1){
        port += atoi(argv[1]);
        printf("%d\n", port);
        strcat(str_addr, argv[1]);
        printf("%s\n", str_addr);
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

    /* Fill in the address family and port. The inet_addr 
        function converts a string form of IP address to 
        a 32 binary integer in network order*/
    target_pc.sin_addr.s_addr = inet_addr(&str_addr[0]);
    target_pc.sin_family = AF_INET;                 
    target_pc.sin_port = htons(port);
    /* fill in my address and port */
    me.sin_family = AF_INET;
    me.sin_port = htons(0);
    me.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Comer is incorrect when he says bind is only for servers */
    i = bind(sock, (struct sockaddr *) &me, sizeof(me));
    if( i < 0) {
        printf("bind result: %d\n", i);                      
        return -1;
    } 
    
    /* join the specified multicast group */
    if(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
        (void *) &join_group, (socklen_t) sizeof(mreq_t)) ){
        printf("Group joined failed with error %d\n",errno);
        return -1;
    }

    return 0;
}

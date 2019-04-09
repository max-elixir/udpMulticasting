/* Maximilian Guzman, gan022 */



/* Server code */
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define Group join_group.imr_multiaddr.s_addr
#define Interface join_group.imr_interface.s_addr

int main(int argc, char *argv[]){
    int sock, i, nbytes, flags, size, addrlen;    
    int port = 22200;
    char *str_addr = (char *) malloc(11);
    strcpy(str_addr, "239.10.5.");  
    char *message = (char *) malloc(30);
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

    return 0;
}

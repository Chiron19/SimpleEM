#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
   
#include "udp_client.h"
   
// Driver code
int main() {
    int sockfd;
    char buffer[MAXLINE];
    const char *hello = "Hello from client!";
    struct sockaddr_in servaddr;
   
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket creation failed...\n");
        exit(0);
    }   
    memset(&servaddr, 0, sizeof(servaddr));
       
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    // servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_addr.s_addr = inet_addr("172.16.0.2");
    
       
    sendto(sockfd, (const char *)hello, strlen(hello),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
        sizeof(servaddr));

    printf("Message sent\n");

    int n;
    socklen_t len;
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                &len);
    buffer[n] = '\0';

    printf("Message received: %s\n", buffer);

    close(sockfd);
    return 0;
}
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "udp_server.h"

void echo(int sockfd) {
    char buffer[MAXLINE], server_suff[] = "[SERVER ECHO]";
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));

    socklen_t len = sizeof(cliaddr);

    int n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                     MSG_WAITALL, (struct sockaddr *) &cliaddr,
                     &len);
    buffer[n] = '\0';
    printf("Received from addr: %s port: %d message: %s\n", 
        inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), buffer);

    strcpy(buffer + n, server_suff);
    buffer[n + strlen(server_suff)] = '\0';
    sendto(sockfd, buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
}

int set_up_server() {
    int sockfd;
    struct sockaddr_in servaddr;
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket creation failed...\n");
        exit(0);
    }
       
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0) {
        printf("Socket binding failed...\n");
        exit(0);
    }
    return sockfd;
}

int main() {
    int sockfd = set_up_server();
    printf("Server echoing client messages.\n");
    while(true) {
        echo(sockfd);
    }
    
    close(sockfd);
    return 0;
}
#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <stdbool.h>
#include <errno.h>

#include "tcp_client.h"


int main() {
    // int pid = getpid();
    struct sockaddr_in servaddr;
    int sockfd = set_up_client(&servaddr);

    if (REPORT_ACTIONS)
        printf("Trying to connect to: %s %u\n", 
            inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        if (REPORT_ERRORS)
            printf("[ERROR] Connection with the server failed.\n");
        exit(0);
    }
    if (REPORT_ACTIONS)
        printf("Connected to server.\n");
    
    mes_rec_server_periodically(sockfd);

    close(sockfd);
    return 0;
}

int set_up_client(struct sockaddr_in* servaddr_ptr) {
    int sockfd;
 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        if (REPORT_ERRORS)
            printf("[ERROR] Socket creation failed.\n");
        exit(0);
    }
    bzero(servaddr_ptr, sizeof(*servaddr_ptr));
 
    servaddr_ptr->sin_family = AF_INET;
    servaddr_ptr->sin_addr.s_addr = inet_addr("172.16.0.2");
    // servaddr_ptr->sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr_ptr->sin_port = htons(PORT);

    return sockfd;
}

void mes_rec_server(int sockfd) {
    char buff[] = "Hello";


    write(sockfd, buff, sizeof(buff));
    if (REPORT_ACTIONS)
        printf("Sent to server message:             %s.\n", buff);

    bzero(buff, sizeof(buff));
    read(sockfd, buff, sizeof(buff));
    if (REPORT_ACTIONS)
        printf("Received from server message:       %s.\n", buff);
}

void mes_rec_server_periodically(int sockfd) {
    char buff[MAX_BUFF];
    int i = 1;
    ssize_t read_res;

    while(true) {
        snprintf(buff, MAX_BUFF, "pid:%d,mes_num:%d", getpid(), i);
        write(sockfd, buff, sizeof(buff));
        if (REPORT_ACTIONS)
            printf("Sent to server message:             %s.\n", buff);

        bzero(buff, sizeof(buff));

        if ((read_res = read(sockfd, buff, sizeof(buff))) == -1) {
            if (errno == ECONNRESET) {
                /* Server reset connection */
                if (REPORT_ACTIONS)
                    printf("Server reset connection.\n");
                return;
            }
            if (REPORT_ERRORS)
                printf("[ERROR] Error while reading, errno: %d\n", errno);
            exit(0);
        }
        if (REPORT_ACTIONS)
            printf("Received from server message:       %s.\n", buff);

        sleep(1); /* Sleepy */
        i++;
    }

}
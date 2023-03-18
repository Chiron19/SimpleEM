#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

#include <pthread.h>

#include "tcp/tcp_server.h"
#include "verbosity/verbosity.h"

#define N 5

int main() {
    int sockfd = set_up_server();
    
    pthread_t thread_ids[N];
    conn_desc_t conn_descs[N];
    unsigned len;

    for (int i = 0; i < N; ++i) {
        len = sizeof(conn_descs[i].addr);
        conn_descs[i].connfd = accept(sockfd, 
            (struct sockaddr*)&conn_descs[i].addr, &len);

        if (conn_descs[i].connfd == -1) {
            if (REPORT_ERROR)
                printf("[ERROR] Accepting failed.\n");
            exit(0);
        }
        if (REPORT_ACTION) {
            printf("[ACTION] Server accepted connection from client: %s %u\n", 
                inet_ntoa(conn_descs[i].addr.sin_addr), 
                ntohs(conn_descs[i].addr.sin_port));
        }
        pthread_create(&thread_ids[i], NULL, thread_routine, &conn_descs[i]);
    }

    for (int i = 0; i < N; ++i) {
        pthread_join(thread_ids[i], NULL);
    }

    close(sockfd);
    if (REPORT_ACTION) {
        printf("[ACTION] Socked closed.\n");
    }
    return 0;
}

void* thread_routine(void* arg) {
    conn_desc_t conn_desc = *(conn_desc_t*)arg;
    echo_client(conn_desc);
    return NULL;
}

int set_up_server() {
    int sockfd;
    struct sockaddr_in servaddr;
   
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        if (REPORT_ERROR)
            printf("[ERROR] Socket creation failed.\n");
        exit(0);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) != 0) {
        if (REPORT_ERROR)
            printf("[ERROR] setsockopt SO_REUSEADDR failed.\n");
        exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        if (REPORT_ERROR)
            printf("[ERROR] Socket binding failed.\n");
        exit(0);
    }
    if (listen(sockfd, 2) != 0) {
        if (REPORT_ERROR)   
            printf("[ERROR] Listening failed.\n");
        exit(0);
    }

    if (REPORT_ACTION)
        printf("[ACTION] Server ready, starts listening.\n");
    return sockfd;
}

void echo_client(conn_desc_t conn_desc) {
    char read_buff[MAX_BUFF], write_buff[MAX_BUFF];
    char echo_pref[] = "[[[SERVER ECHO]]]";
    ssize_t read_res, write_res;

    while(true) {
        if ((read_res = read(conn_desc.connfd, read_buff, sizeof(read_buff))) == -1) {
            if (errno == ECONNRESET) {
                /* Client reset connection */
                if (REPORT_ACTION)
                    printf("Client reset connection.\n");
                return;
            }
            if (REPORT_ERROR)
                printf("[ERROR] Error while reading, errno: %d\n", errno);
            exit(0);
        }
        read_buff[read_res] = '\0';

        if (REPORT_MESSAGE)
            printf("[MESSAGE] Received from addr: %s port: %d message: %s\n", 
                inet_ntoa(conn_desc.addr.sin_addr), 
                ntohs(conn_desc.addr.sin_port), 
                read_buff);

        strcpy(write_buff, echo_pref);
        strcpy(write_buff + strlen(echo_pref), read_buff);
        write_buff[strlen(echo_pref) + strlen(read_buff)] = '\0';

        if ((write_res = write(conn_desc.connfd, write_buff, 
            strlen(write_buff))) != strlen(write_buff)) {
            if (REPORT_ERROR)
                printf("[ERROR] Error while writeing, errno: %d\n", errno);
            exit(0);
        }
    }
}
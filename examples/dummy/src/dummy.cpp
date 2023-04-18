#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <cstdlib>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "dummy.hpp"

#include "utils.hpp"

FILE *logging_fptr;

int main() {
    logging_fptr = open_logging((char*)"dummy", false);
    signal(SIGINT, sigint_handler);

    struct sockaddr_in servaddr;
    int sockfd = setup_socket(&servaddr);
    char buf[MAXLINE];

    log_event_proc_cpu_time("Start of %d", getpid());

    for (int i = 0;; ++i) {
        for (int ii = 0; ii < 1e5; ++ii) {}

        log_event_proc_cpu_time("Sending message");

        size_t offset = 0;
        offset += push_to_buffer_string(buf + offset, "[MES][pid:");
        offset += push_to_buffer_int(buf + offset, getpid());
        offset += push_to_buffer_string(buf + offset, "][mes_nr:");
        offset += push_to_buffer_int(buf + offset, i);
        offset += push_to_buffer_string(buf + offset, "]\n");
        
        sendto(sockfd, buf, offset, MSG_CONFIRM, 
               (const struct sockaddr *) &servaddr, sizeof(servaddr));

    }

    return 0;
}


int setup_socket(struct sockaddr_in* servaddr) {
    int sockfd;
   
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket creation failed...\n");
        exit(0);
    }   
    memset(servaddr, 0, sizeof(servaddr));
       
    servaddr->sin_family = AF_INET;
    servaddr->sin_port = htons(PORT);
    // servaddr->sin_addr.s_addr = INADDR_ANY;
    servaddr->sin_addr.s_addr = inet_addr(SERV_ADDR);

    return sockfd;
}

void sigint_handler(int signum) {
    char buf[BUF_SIZE];
    size_t offset = 0;

    offset += push_to_buffer_string(buf, "[DUMMY][SIGINT]");
    offset += push_to_buffer_time(buf + offset, CLOCK_PROCESS_CPUTIME_ID);
    offset += push_to_buffer_string(buf + offset, "\n");

    /* Write to IO FD */
    write(STDOUT_FILENO, buf, offset);

    fclose(logging_fptr);

    _exit(signum);
}
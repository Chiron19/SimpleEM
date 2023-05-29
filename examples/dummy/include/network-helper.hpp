#pragma once

#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>


#include <vector>
#include <fstream>
#include <iostream>

#include "utils.hpp"
#include "logger.hpp"

#define MAXLINE 10000

typedef std::pair<int, std::string> message_t; // sender em_id + message

class NetworkHelper {
public:
    int em_id, procs;
    int send_fd, recv_fd;
    std::vector<std::pair<std::string, int>> addresses;

    NetworkHelper(int em_id, const std::string& config_file): em_id(em_id) {
        std::ifstream config(config_file);
        std::string address;
        int port;
        while(config >> address >> port) {
            addresses.push_back(std::make_pair(address, port));
        }
        procs = addresses.size();

        setup_recv_socket();
        setup_send_socket();
    }

    void send(int target_em_id, const std::string& message) {
        struct sockaddr_in recvaddr;
        memset(&recvaddr, 0, sizeof(recvaddr));
        recvaddr.sin_family = AF_INET;
        recvaddr.sin_port = htons(addresses[target_em_id].second);
        recvaddr.sin_addr.s_addr = inet_addr(addresses[target_em_id]
            .first.c_str());
        if (sendto(send_fd, message.c_str(), message.size(), 0, 
               (const struct sockaddr *) &recvaddr, sizeof(recvaddr)) == -1) {
            printf("[DUMMY] sendto error: %d\n", errno);
        }
        logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID, 
            "Send packet to proc %d (%s), message: %s", 
            target_em_id, addresses[target_em_id].first.c_str(), message.c_str());
    }

    /*
        Returns received message or empty string if nothing was received
     */
    message_t receive() {
        char buffer[MAXLINE];
        struct sockaddr_in sender_addr;
        int sender_id = -1;
        memset(&sender_addr, 0, sizeof(sender_addr));
        socklen_t len = sizeof(sender_addr);

        ssize_t n = recvfrom(recv_fd, (char *)buffer, MAXLINE, MSG_DONTWAIT, (struct sockaddr*) &sender_addr, &len);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return {-1, ""};
            std::cout << "RECVFROM ERROR" << std::endl;
            exit(1);

        }
        buffer[n] = '\0';

        for (sender_id = 0; sender_id < procs; ++sender_id) {
            if (inet_addr(addresses[sender_id].first.c_str()) == 
                sender_addr.sin_addr.s_addr) 
                break;
        }

        if (sender_id == -1) {
            std::cout << "ERROR - SENDER DOESNT EXIST" << std::endl;
            exit(1);
        }

        logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID,
            "Received packet from proc %d (%s), message: %s",
            sender_id, addresses[sender_id].first.c_str(), buffer);
        return {sender_id, buffer};
    }

private:
    void setup_send_socket() {
        if ((send_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            printf("Socket creation failed...\n");
            exit(0);
        }
    }
    
    void setup_recv_socket() {        
        if ((recv_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            printf("Socket creation failed...\n");
            exit(0);
        }
        
        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET; 
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(addresses[em_id].second);

        if (bind(recv_fd, (const struct sockaddr *)&servaddr, 
                sizeof(servaddr)) < 0) {
            printf("Socket binding failed...\n");
            exit(0);
        }
    }
};

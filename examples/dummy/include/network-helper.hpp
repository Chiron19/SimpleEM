#pragma once

#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>


#include <vector>
#include <stack>
#include <fstream>
#include <iostream>
#include <sys/socket.h>

#include "utils.hpp"
#include "logger.hpp"

#define MAXLINE 10000

typedef std::pair<int, std::string> message_t; // sender em_id + message

class NetworkHelper {
public:
    int em_id, procs;
    int send_fd, recv_fd;
    std::vector<std::pair<std::string, int>> addresses;
    std::vector<std::stack<message_t>> inbox;
    
    NetworkHelper(int em_id, const std::string& config_path): em_id(em_id) {  
        // General Setup: read config
        std::ifstream config(config_path);
        std::string address;
        int port;
        while(config >> address >> port) {
            addresses.push_back(std::make_pair(address, port));
            std::stack<message_t> temp;
            inbox.push_back(temp);
        }
        procs = addresses.size();

        // For em_id: setup socket
        setup_recv_socket();
        setup_send_socket();
    }

    void send_(int target_em_id, const std::string& message) {
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

    int send_tcp(int target_em_id, const std::string& message) {
        struct sockaddr_in recvaddr;
        memset(&recvaddr, 0, sizeof(recvaddr));
        if (procs <= target_em_id) return -1;
        if (inet_pton(AF_INET, addresses[target_em_id].first.c_str(), &(recvaddr.sin_addr)) != 1) {
            std::cerr << "Invalid des IP address: " << addresses[target_em_id].first.c_str() << std::endl;
            return -1;
        }
        recvaddr.sin_family = AF_INET;
        recvaddr.sin_port = htons(addresses[target_em_id].second);
        
        std::cout << "[network-helper] Src: " << em_id << ", Des: " << target_em_id << " " <<  inet_ntoa(recvaddr.sin_addr) << " " << ntohs(recvaddr.sin_port) <<  std::endl;

        // Connect to server
        if (connect(send_fd, (struct sockaddr*)&recvaddr, sizeof(recvaddr)) == -1) {    
            std::cout<< "[network-helper] sender: Fail to connect server socket" << std::endl;
            // sleep(1);
            return -1;
            // // Trying to re-setup
            // close(send_fd);
            // setup_send_socket();
            // continue;
        }
        std::cout<< "[network-helper] sender: socket connected" << std::endl;

        // Send packet to server
        std::cout << "[network-helper] message:" << message.c_str() << std::endl;
        // std::cout << "[network-helper] length :" << message.size() << std::endl;
        if (send(send_fd, message.c_str(), message.size(), 0) == -1) {
            printf("[DUMMY] sendto error: %d\n", errno);
            return -1;
        }

        logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID, 
            "Send packet to proc %d (%s), message: %s", 
            target_em_id, addresses[target_em_id].first.c_str(), message.c_str());

        // close(send_fd);
        return 0;
    }

    /*
        Returns received message or empty string if nothing was received
     */
    message_t receive_() {
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

    message_t receive_tcp() {
        std::cout << "[network-helper] in receive_tcp" << std::endl;
        char buffer[MAXLINE];
        std::cout << "[network-helper] buffer" << std::endl;
        struct sockaddr_in sender_addr;
        int sender_id = -1;
        memset(&sender_addr, 0, sizeof(sender_addr));
        socklen_t len = sizeof(sender_addr);
        std::cout << "[network-helper] sender_sock" << std::endl;

        // Listen for connections
        if (listen(recv_fd, 0) == -1) {
            std::cout << "[network-helper] Error Listening, Retry!" << std::endl;
            // exit(1);
            return {-1, ""};
        }
        std::cout << "[network-helper] Server1, Listening" << std::endl;

        // Accept client connection
        int newSocket_fd;
        if ((newSocket_fd = accept(recv_fd, (struct sockaddr*)&sender_addr, &len)) == -1) {
            std::cout << "[network-helper] Error Accepting, Retry!" << std::endl;
            return {-1, ""};
            // exit(1);
            // close(recv_fd);
            // setup_recv_socket();
            // listen(recv_fd, 0);
        }
        std::cout << "[network-helper] Socket on server1, Accepted" << std::endl;

        // Receive and print received data
        // ssize_t n = recvfrom(newSocket_fd, (char *)buffer, MAXLINE, MSG_DONTWAIT, (struct sockaddr*) &sender_addr, &len);
        ssize_t n = recv(newSocket_fd, buffer, sizeof(buffer), 0);

        if (n == -1) {
            std::cerr << "[network-helper] Server: There was a connection issue." << std::endl;
        }
        if (n == 0) {
            std::cout << "[network-helper] Client Disconnected." << std::endl;
        }
        
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return {-1, ""};
            std::cout << "[network-helper] RECVFROM ERROR" << std::endl;
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

        close(newSocket_fd);

        logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID,
            "Received packet from proc %d (%s), message: %s",
            sender_id, addresses[sender_id].first.c_str(), buffer);
        return {sender_id, buffer};
    }

private:
    void setup_send_socket() {
        // SOCK_STREAM is for TCP socket
        // SOCK_DGRAM  is for UDP socket
        if ((send_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Socket creation failed...\n");
            exit(0);
        }

        int opt = 1, sndbuf = 32768;
        if (setsockopt(send_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cout << "Failed to set SO_REUSEADDR option. " << strerror(errno) << "\n";
            exit(1);
        }
        if (setsockopt(send_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            std::cout << "Failed to set SO_REUSEPORT option. " << strerror(errno) << "\n";
            exit(1);
        }
        if (setsockopt(send_fd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) < 0) {
            std::cout << "Failed to set SO_SNDBUF option. " << strerror(errno) << "\n";
            exit(1);
        }
    }
    
    void setup_recv_socket() { 
        // SOCK_STREAM is for TCP socket
        // SOCK_DGRAM  is for UDP socket       
        if ((recv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Socket creation failed...\n");
            exit(0);
        }

        int opt = 1, rcvbuf = 65536;
        if (setsockopt(recv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cout << "Failed to set SO_REUSEADDR option. " << strerror(errno) << "\n";
            exit(1);
        }
        if (setsockopt(recv_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            std::cout << "Failed to set SO_REUSEPORT option. " << strerror(errno) << "\n";
            exit(1);
        }
        if (setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0) {
            std::cout << "Failed to set SO_RCVBUF option. " << strerror(errno) << "\n";
            exit(1);
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
